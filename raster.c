/*
 * vdi_raster.c - Blitting routines
 *
 * Copyright 2002 Joachim Hoenig (blitter)
 * Copyright 2003-2020 The EmuTOS development team
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#include <stdbool.h>
#include <stdint.h>

#include "vdi.h"

/* flag:1 SOURCE and PATTERN   flag:0 SOURCE only */
#define PAT_FLAG        16

/* bitblt modes */
#define BM_ALL_WHITE   0
#define BM_S_AND_D     1
#define BM_S_AND_NOTD  2
#define BM_S_ONLY      3
#define BM_NOTS_AND_D  4
#define BM_D_ONLY      5
#define BM_S_XOR_D     6
#define BM_S_OR_D      7
#define BM_NOT_SORD    8
#define BM_NOT_SXORD   9
#define BM_NOT_D      10
#define BM_S_OR_NOTD  11
#define BM_NOT_S      12
#define BM_NOTS_OR_D  13
#define BM_NOT_SANDD  14
#define BM_ALL_BLACK  15

/*
 * values for skew
 */
#define FXSR    0x80
#define NFSR    0x40
#define SKEW    0x0f

/*
 * values for hop
 */
#define HOP_ALL_ONES            0
#define HOP_HALFTONE_ONLY       1
#define HOP_SOURCE_ONLY         2
#define HOP_SOURCE_AND_HALFTONE 3

/* setting of skew flags */

/* ---QUALIFIERS--- -ACTIONS-
 * dirn equal Sx&F>
 * L->R spans Dx&F  FXSR NFSR
 *  0     0     0     0    1  |..ssssssssssssss|ssssssssssssss..|
 *                            |......dddddddddd|dddddddddddddddd|dd..............|
 *
 *  0     0     1     1    0  |......ssssssssss|ssssssssssssssss|ss..............|
 *                            |..dddddddddddddd|dddddddddddddd..|
 *
 *  0     1     0     1    1  |..ssssssssssssss|ssssssssssssss..|
 *                            |...ddddddddddddd|ddddddddddddddd.|
 *
 *  0     1     1     0    0  |...sssssssssssss|sssssssssssssss.|
 *                            |..dddddddddddddd|dddddddddddddd..|
 *
 *  1     0     0     0    1  |..ssssssssssssss|ssssssssssssss..|
 *                            |......dddddddddd|dddddddddddddddd|dd..............|
 *
 *  1     0     1     1    0  |......ssssssssss|ssssssssssssssss|ss..............|
 *                            |..dddddddddddddd|dddddddddddddd..|
 *
 *  1     1     0     0    0  |..ssssssssssssss|ssssssssssssss..|
 *                            |...ddddddddddddd|ddddddddddddddd.|
 *
 *  1     1     1     1    1  |...sssssssssssss|sssssssssssssss.|
 *                            |..dddddddddddddd|dddddddddddddd..|
 */

static const uint8_t skew_flags[8] = {
                        /* for blit direction Right->Left */
    NFSR,                   /* Source span < Destination span */
    FXSR,                   /* Source span > Destination span */
    NFSR+FXSR,              /* Spans equal, Shift Source right */
    0,                      /* Spans equal, Shift Source left */
                        /* for blit direction Left->Right */
    NFSR,                   /* Source span < Destination span */
    FXSR,                   /* Source span > Destination span */
    0,                      /* Spans equal, Shift Source right */
    NFSR+FXSR               /* Spans equal, Shift Source left */
};

/* structure passed to raster blit functions */
typedef struct {
    uint16_t          halftone[16];
    int16_t           src_x_inc, src_y_inc;
    uint32_t          src_addr;
    int16_t           end_1, end_2, end_3;
    int16_t           dst_x_inc, dst_y_inc;
    uint32_t          dst_addr;
    uint16_t          x_cnt, y_cnt;
    uint8_t          hop, op, status, skew;
} BLITVARS;

/* common settings needed both by VDI and line-A raster
 * operations, but being given through different means.
 */
struct raster_t {
    int16_t line_length; /* Length of a line in video RAM */
    int n_planes;
    vdi_clipping_rect_t *clipper;
    int clip;
    int multifill;
    int transparent;
};

/* 76-byte line-A BITBLT struct passing parameters to bitblt */
struct blit_frame {
    int16_t b_wd;          /* +00 width of block in pixels */
    int16_t b_ht;          /* +02 height of block in pixels */
    int16_t plane_ct;      /* +04 number of consecutive planes to blt */
    uint16_t fg_col;       /* +06 foreground color (logic op table index:hi bit) */
    uint16_t bg_col;       /* +08 background color (logic op table index:lo bit) */
    uint8_t op_tab[4];    /* +10 logic ops for all fore and background combos */
    int16_t s_xmin;        /* +14 minimum X: source */
    int16_t s_ymin;        /* +16 minimum Y: source */
    uint16_t * s_form;     /* +18 source form base address */
    int16_t s_nxwd;        /* +22 offset to next word in line  (in bytes) */
    int16_t s_nxln;        /* +24 offset to next line in plane (in bytes) */
    int16_t s_nxpl;        /* +26 offset to next plane from start of current plane */
    int16_t d_xmin;        /* +28 minimum X: destination */
    int16_t d_ymin;        /* +30 minimum Y: destination */
    uint16_t * d_form;     /* +32 destination form base address */
    int16_t d_nxwd;        /* +36 offset to next word in line  (in bytes) */
    int16_t d_nxln;        /* +38 offset to next line in plane (in bytes) */
    int16_t d_nxpl;        /* +40 offset to next plane from start of current plane */
    uint16_t * p_addr;     /* +42 address of pattern buffer   (0:no pattern) */
    int16_t p_nxln;        /* +46 offset to next line in pattern  (in bytes) */
    int16_t p_nxpl;        /* +48 offset to next plane in pattern (in bytes) */
    int16_t p_mask;        /* +50 pattern index mask */

    /* these frame parameters are internally set */
    int16_t p_indx;        /* +52 initial pattern index */
    uint16_t * s_addr;     /* +54 initial source address */
    int16_t s_xmax;        /* +58 maximum X: source */
    int16_t s_ymax;        /* +60 maximum Y: source */
    uint16_t * d_addr;     /* +62 initial destination address */
    int16_t d_xmax;        /* +66 maximum X: destination */
    int16_t d_ymax;        /* +68 maximum Y: destination */
    int16_t inner_ct;      /* +70 blt inner loop initial count */
    int16_t dst_wr;        /* +72 destination form wrap (in bytes) */
    int16_t src_wr;        /* +74 source form wrap (in bytes) */
};

// Helpers
static void setup_pattern(const struct raster_t *raster, struct blit_frame *info);
static bool setup_info(struct raster_t *raster, struct blit_frame * info, MFDB *src, MFDB *dst);
static void cpy_raster(
    struct raster_t *raster, struct blit_frame *info,
    vdi_rectangle_t *src, vdi_rectangle_t *dst,
    MFDB *psrcMFDB, MFDB *pdesMFDB,
    int16_t mode, int16_t *colors);
static void bit_blt(struct blit_frame *blit_info);
static void do_blit(BLITVARS * blt);

#define GetMemW(addr) ((uint32_t)*(uint16_t*)(addr))
#define SetMemW(addr, val) *(uint16_t*)(addr) = val

/*
 * vr_trnfm - transform screen bitmaps
 *
 * Convert device-independent bitmaps to device-dependent and vice versa
 *
 * The major difference between the two formats is that, in the device-
 * independent ("standard") form, the planes are consecutive, while on
 * the Atari screen they are interleaved.
 */
void vr_trnfm(const MFDB *src_mfdb, MFDB *dst_mfdb)
{
    int16_t *src, *dst, *work;
    int16_t planes;
    bool inplace;
    int32_t size, inner, outer, i, j;

    src = src_mfdb->fd_addr;
    dst = dst_mfdb->fd_addr;
    planes = src_mfdb->fd_nplanes;
    size = (int32_t)src_mfdb->fd_h * src_mfdb->fd_wdwidth; /* size of plane in words */
    inplace = (src == dst);

    if (src_mfdb->fd_stand)     /* source is standard format */
    {
        dst_mfdb->fd_stand = 0;     /* force dest to device-dependent */
        outer = planes;             /* set outer & inner loop counts */
        inner = size;
    }
    else                        /* source is device-dependent format */
    {
        dst_mfdb->fd_stand = 1;     /* force dest to standard */
        outer = size;               /* set loop counts */
        inner = planes;
    }

    if (!inplace)               /* the simple option */
    {
        for (i = 0; i < outer; i++, dst++)
        {
            for (j = 0, work = dst; j < inner; j++)
            {
                *work = *src++;
                work += outer;
            }
        }
        return;
    }

    /* handle in-place transform - can be slow (on Atari TOS too) */
    if (planes == 1)            /* for mono, there is no difference    */
        return;                 /* between standard & device-dependent */

    if (--outer <= 0)
        return;

    for (--inner; inner >= 0; inner--)
    {
        int32_t count;
        for (i = 0, count = 0L; i < outer; i++)
        {
            int16_t temp;
            src += inner + 1;
            temp = *src;
            dst = src;
            work = src;
            count += inner;
            for (j = 0; j < count; j++)
            {
                work = dst--;
                *work = *dst;
            }
            *dst = temp;
        }
        src = work;
    }
}

#if 0
/*
 * vdi_vrt_cpyfm - copy raster transparent
 *
 * This function copies a monochrome raster area from source form to a
 * color area. A writing mode and color indices for both 0's and 1's
 * are specified in the INTIN array.
 */
void vdi_vrt_cpyfm(int16_t handle, int16_t vr_mode, vdi_rectangle_t *src_dst, MFDB *psrcMFDB, MFDB *pdesMFDB, int16_t *colors);
{
    workstation_t *wk = &workstation[handle];

    struct raster_t raster;

    vdi_info.p_addr = vwk->patptr;

    raster.n_planes = wk->screen_info.bitplanes;
    raster.line_length = wk->screen_info.line_length;
    raster.clipper = &(wk->settings.clip_rect);
    raster.clip = vwk->settings.clip;
    raster.multifill = wk->settings.multifill;
    raster.transparent = 1;

    cpy_raster(&raster, &vdi_info, &vdi_rectangle_t[0], &vdi_rectangle_t[1], psrcMFDB, pdesMFDB, vr_mode, colors);
}
#endif

/* common functionality for vdi_vro_cpyfm, vdi_vrt_cpyfm, linea_raster */
static void cpy_raster(
    struct raster_t *raster, struct blit_frame *info,
     vdi_rectangle_t *src, vdi_rectangle_t *dst,
     MFDB *psrcMFDB, MFDB *pdesMFDB,
     int16_t mode, int16_t *colors)
{
    int16_t fg_col, bg_col;

    // src = PTSIN
    // dst = PTSIN+4
    //mode = INTIN[0];

    sort_corners(src);
    sort_corners(dst);

    /* if mode is made up of more than the first 5 bits */
    if (mode & ~0x001f)
        return;                 /* mode is invalid */

    /* check the pattern flag (bit 5) and revert to log op # */
    info->p_addr = NULL;        /* get pattern pointer */
    if (mode & PAT_FLAG) {
        mode &= ~PAT_FLAG;      /* set bit to 0! */
        setup_pattern(raster, info);   /* fill in pattern related stuff */
    }

    /* if true, the plane count is invalid or clipping took all! */
    if (setup_info(raster, info, psrcMFDB, pdesMFDB))
        return;

    if (!raster->transparent) {
        /* COPY RASTER OPAQUE */

        /* planes of source and destination equal in number? */
        if (info->s_nxwd != info->d_nxwd)
            return;

        info->op_tab[0] = mode; /* fg:0 bg:0 */
        info->bg_col = 0;       /* bg:0 & fg:0 => only first OP_TAB */
        info->fg_col = 0;       /* entry will be referenced */

    } else {

        /*
         * COPY RASTER TRANSPARENT - copies a monochrome raster area
         * from source form to a color area. A writing mode and color
         * indices for both 0's and 1's are specified in the INTIN array.
         */

        /* is source area one plane? */
        if (info->s_nxwd != 2)
            return;             /* source must be mono plane */

        info->s_nxpl = 0;       /* use only one plane of source */

        /* d6 <- background color */
        fg_col = colors[0];
        //fg_col = validate_color_index(INTIN[1]);
        //fg_col = MAP_COL[fg_col];

        /* d7 <- foreground color */
        bg_col = colors[1];
        //bg_col = validate_color_index(INTIN[2]);
        //bg_col = MAP_COL[bg_col];

        switch(mode) {
        case MD_TRANS:
            info->op_tab[0] = 04;    /* fg:0 bg:0  D' <- [not S] and D */
            info->op_tab[2] = 07;    /* fg:1 bg:0  D' <- S or D */
            info->fg_col = fg_col;   /* were only interested in one color */
            info->bg_col = 0;        /* save the color of interest */
            break;

        case MD_REPLACE:
            /* CHECK: bug, that colors are reversed? */
            info->op_tab[0] = 00;    /* fg:0 bg:0  D' <- 0 */
            info->op_tab[1] = 12;    /* fg:0 bg:1  D' <- not S */
            info->op_tab[2] = 03;    /* fg:1 bg:0  D' <- S */
            info->op_tab[3] = 15;    /* fg:1 bg:1  D' <- 1 */
            info->bg_col = bg_col;   /* save fore and background colors */
            info->fg_col = fg_col;
            break;

        case MD_XOR:
            info->op_tab[0] = 06;    /* fg:0 bg:0  D' <- S xor D */
            info->bg_col = 0;
            info->fg_col = 0;
            break;

        case MD_ERASE:
            info->op_tab[0] = 01;    /* fg:0 bg:0  D' <- S and D */
            info->op_tab[1] = 13;    /* fg:0 bg:1  D' <- [not S] or D */
            info->fg_col = 0;        /* were only interested in one color */
            info->bg_col = bg_col;   /* save the color of interest */
            break;

        default:
            return;                     /* unsupported mode */
        }
    }

    /*
     * call assembler blit routine or C-implementation.  we call the
     * assembler version if we're not on ColdFire and either
     * (a) the blitter isn't configured, or
     * (b) it's configured but not available.
     */
#if 0 && ASM_BLIT_IS_AVAILABLE
#if CONF_WITH_BLITTER
    if (blitter_is_enabled)
    {
        bit_blt(info);
    }
    else
#endif
    {
        fast_bit_blt(info);
    }
#else
    bit_blt(info);
#endif
}


/*
 * setup_pattern - if bit 5 of mode is set, use pattern with blit
 */
static void setup_pattern(const struct raster_t *raster, struct blit_frame *info)
{
    /* multi-plane pattern? */
    info->p_nxpl = 0;           /* next plane pattern offset default. */
    if (raster->multifill) {
        info->p_nxpl = 32;      /* yes, next plane pat offset = 32. */
    }
    info->p_nxln = 2;        /* offset to next line in pattern */
    info->p_mask = 0xf;      /* pattern index mask */
}


/*
 * setup_info - fill the info structure with MFDB values
 */
static bool setup_info(struct raster_t *raster, struct blit_frame * info, MFDB *src, MFDB *dst)
{
    bool use_clip = false;

    /* Get the pointers to the MFDBs */

    /* setup plane info for source MFDB */
    if (src->fd_addr) {
        /* for a positive source address */
        info->s_form = src->fd_addr;
        info->s_nxwd = src->fd_nplanes * 2;
        info->s_nxln = src->fd_wdwidth * info->s_nxwd;
    }
    else {
        /* source form is screen */
        info->s_form = (uint16_t*)v_bas_ad;
        info->s_nxwd = raster->n_planes * 2;
        info->s_nxln = raster->line_length;
    }

    /* setup plane info for destination MFDB */
    if (dst->fd_addr) {
        /* for a positive address */
        info->d_form = dst->fd_addr;
        info->plane_ct = dst->fd_nplanes;
        info->d_nxwd = dst->fd_nplanes * 2;
        info->d_nxln = dst->fd_wdwidth * info->d_nxwd;
    }
    else {
        /* destination form is screen */
        info->d_form = (uint16_t*)v_bas_ad;
        info->plane_ct = raster->n_planes;
        info->d_nxwd = raster->n_planes * 2;
        info->d_nxln = raster->line_length;

        /* check if clipping is enabled, when destination is screen */
        if (raster->clip)
            use_clip = true;
    }

#if 0
    if (use_clip) {
        if (do_clip(raster->clipper, info))
            return true;        /* clipping took away everything */
    }
    else
        dont_clip(info);
#endif
    info->s_nxpl = 2;           /* next plane offset (source) */
    info->d_nxpl = 2;           /* next plane offset (destination) */

    /* only 8, 4, 2 and 1 planes are valid (destination) */
    return info->plane_ct & ~0x000f;
}


/*
 * bit_blt()
 *
 * Purpose:
 * Transfer a rectangular block of pixels located at an arbitrary X,Y
 * position in the source memory form to another arbitrary X,Y position
 * in the destination memory form, using replace mode (boolean operator 3).
 * This is used on ColdFire (where fast_bit_blt() is not available) or if
 * configuring with the blitter on a 68K system, since fast_bit_blt() does
 * not provide an interface to the hardware.
 *
 * In:
 *  blit_info   pointer to 76 byte input parameter block
 *
 * Note: This is a translation of the original assembler code in the Atari
 * blitter document, with the addition that source and destination are
 * allowed to overlap.  Original source code comments are mostly preserved.
 */
static void bit_blt(struct blit_frame *blit_info)
{
    int16_t plane;
    uint16_t s_xmin, s_xmax;
    uint16_t d_xmin, d_xmax;
    uint16_t lendmask, rendmask;
    int16_t skew, skew_idx;
    int16_t s_span, s_xmin_off, s_xmax_off;
    int16_t d_span, d_xmin_off, d_xmax_off;
    uint32_t s_addr, d_addr;
    BLITVARS blitter;

    /* a5-> BLiTTER register block */
    BLITVARS *blt = &blitter;

    /* Calculate Xmax coordinates from Xmin coordinates and width */
    s_xmin = blit_info->s_xmin;               /* d0<- src Xmin */
    s_xmax = s_xmin + blit_info->b_wd - 1;    /* d1<- src Xmax=src Xmin+width-1 */
    d_xmin = blit_info->d_xmin;               /* d2<- dst Xmin */
    d_xmax = d_xmin + blit_info->b_wd - 1;    /* d3<- dst Xmax=dstXmin+width-1 */

    /*
     * Skew value is (destination Xmin mod 16 - source Xmin mod 16) && 0x000F.
     * Three main discriminators are used to determine the states of the skew
     * flags (FXSR and NFSR):
     *
     * bit 0     0: Source Xmin mod 16 =< Destination Xmin mod 16
     *           1: Source Xmin mod 16 >  Destination Xmin mod 16
     *
     * bit 1     0: SrcXmax/16-SrcXmin/16 <> DstXmax/16-DstXmin/16
     *                       Source span      Destination span
     *           1: SrcXmax/16-SrcXmin/16 == DstXmax/16-DstXmin/16
     *
     * bit 2     0: Blit direction is from Right to Left
     *           1: Blit direction is from Left to Right
     *
     * These form an offset into a skew flag table yielding FXSR and NFSR flag
     * states for the given source and destination alignments.
     *
     * NOTE: this table lookup is overridden for the special case when both
     * the source & destination widths are one, and the skew is 0.  For this
     * case, the FXSR flag alone is always set.
     */

    skew_idx = 0x0000;                  /* default */

    s_xmin_off = s_xmin >> 4;           /* d0<- word offset to src Xmin */
    s_xmax_off = s_xmax >> 4;           /* d1<- word offset to src Xmax */
    s_span = s_xmax_off - s_xmin_off;   /* d1<- Src span - 1 */

    d_xmin_off = d_xmin >> 4;           /* d2<- word offset to dst Xmin */
    d_xmax_off = d_xmax >> 4;           /* d3<- word offset to dst Xmax */
    d_span = d_xmax_off - d_xmin_off;   /* d3<- dst span - 1 */

                                        /* the last discriminator is the */
    if ( d_span == s_span ) {           /* equality of src and dst spans */
        skew_idx |= 0x0002;             /* d6[bit1]:1 => equal spans */
    }

    /* d4<- number of words in dst line */
    blt->x_cnt = d_span + 1;            /* set value in BLiTTER */

    /* Endmasks derived from dst Xmin mod 16 and dst Xmax mod 16 */
    lendmask=0xffff>>(d_xmin%16);
    rendmask=~(0x7fff>>(d_xmax%16));

    /* d7<- Dst Xmin mod16 - Src Xmin mod16 */
    skew = (d_xmin & 0x0f) - (s_xmin & 0x0f);
    if (skew < 0)
        skew_idx |= 0x0001;             /* d6[bit0]<- alignment flag */

    /* Calculate starting addresses */
    s_addr = (uint32_t)blit_info->s_form
        + (uint32_t)blit_info->s_ymin * (uint32_t)blit_info->s_nxln
        + (uint32_t)s_xmin_off * (uint32_t)blit_info->s_nxwd;
    d_addr = (uint32_t)blit_info->d_form
        + (uint32_t)blit_info->d_ymin * (uint32_t)blit_info->d_nxln
        + (uint32_t)d_xmin_off * (uint32_t)blit_info->d_nxwd;

    /* if (just_screen && (s_addr < d_addr)) { */
    if ((s_addr < d_addr)
     || ((s_addr == d_addr) && (skew >= 0))) {
        /* start from lower right corner, so add width+length */
        s_addr = (uint32_t)blit_info->s_form
            + (uint32_t)blit_info->s_ymax * (uint32_t)blit_info->s_nxln
            + (uint32_t)s_xmax_off * (uint32_t)blit_info->s_nxwd;
        d_addr = (uint32_t)blit_info->d_form
            + (uint32_t)blit_info->d_ymax * (uint32_t)blit_info->d_nxln
            + (uint32_t)d_xmax_off * (uint32_t)blit_info->d_nxwd;

        /* offset between consecutive words in planes */
        blt->src_x_inc = -blit_info->s_nxwd;
        blt->dst_x_inc = -blit_info->d_nxwd;

        /* offset from last word of a line to first word of next one */
        blt->src_y_inc = -(blit_info->s_nxln - blit_info->s_nxwd * s_span);
        blt->dst_y_inc = -(blit_info->d_nxln - blit_info->d_nxwd * d_span);

        blt->end_1 = rendmask;          /* first write mask */
        blt->end_2 = 0xFFFF;            /* center mask */
        blt->end_3 = lendmask;          /* last write mask */
    }
    else {
        /* offset between consecutive words in planes */
        blt->src_x_inc = blit_info->s_nxwd;
        blt->dst_x_inc = blit_info->d_nxwd;

        /* offset from last word of a line to first word of next one */
        blt->src_y_inc = blit_info->s_nxln - blit_info->s_nxwd * s_span;
        blt->dst_y_inc = blit_info->d_nxln - blit_info->d_nxwd * d_span;

        blt->end_1 = lendmask;          /* first write mask */
        blt->end_2 = 0xFFFF;            /* center mask */
        blt->end_3 = rendmask;          /* last write mask */

        skew_idx |= 0x0004;             /* blitting left->right */
    }

    /* does destination just span a single word? */
    if (!d_span) {
        /* merge both end masks into Endmask1. */
        blt->end_1 &= blt->end_3;       /* single word end mask */
        /* The other end masks will be ignored by the BLiTTER */
    }

    /*
     * Set up the skew byte, which contains the FXSR/NFSR flags and the
     * skew value.  The skew value is the low nybble of the difference
     * in Source and Destination alignment.
     *
     * The main complication is setting the FXSR/NFSR flags.  Normally
     * we use the calculated skew_idx to obtain them from the skew_flags[]
     * array.  However, when the source and destination widths are both 1,
     * we do not set either flag unless the skew value is zero, in which
     * case we set the FXSR flag only.  Additionally, we must set the skew
     * direction in source x incr.
     *
     * Thank you blitter hardware designers ...
     */
    if (!s_span && !d_span) {
        blt->src_x_inc = skew;          /* sets skew direction */
        blt->skew = skew ? (skew & 0x0f) : FXSR;
    } else {
        blt->skew = (skew & 0x0f) | skew_flags[skew_idx];
    }

    blt->hop = HOP_SOURCE_ONLY;         /* set HOP to source only */

    for (plane = 0; plane < blit_info->plane_ct; plane++) {
        int op_tabidx;

        blt->src_addr = s_addr;         /* load Source pointer to this plane */
        blt->dst_addr = d_addr;         /* load Dest ptr to this plane   */
        blt->y_cnt = blit_info->b_ht;   /* load the line count   */

        /* calculate operation for actual plane */
        op_tabidx = ((blit_info->fg_col>>plane) & 0x0001 ) <<1;
        op_tabidx |= (blit_info->bg_col>>plane) & 0x0001;
        blt->op = blit_info->op_tab[op_tabidx] & 0x000f;

        /*
         * We can only be here if either:
         * (a) we are on ColdFire (ASM_BLIT_IS_AVAILABLE is 0): the
         *     hardware blitter may or may not be enabled, or
         * (b) we are on 68K (ASM_BLIT_IS_AVAILABLE is 1): the
         *     hardware blitter must be enabled to get here.
         */
#if !ASM_BLIT_IS_AVAILABLE
#if CONF_WITH_BLITTER
        if (blitter_is_enabled)
        {
            hwblit_raster(blt);
        }
        else
#endif
        {
            do_blit(blt);
        }
#else
        hwblit_raster(blt);
#endif

        s_addr += blit_info->s_nxpl;          /* a0-> start of next src plane   */
        d_addr += blit_info->d_nxpl;          /* a1-> start of next dst plane   */
    }
}


/*
 * the following is a modified version of a blitter emulator, with the HOP
 * processing removed since it is always called with a HOP value of 2 (source)
 */
static void do_blit(BLITVARS * blt)
{
    uint32_t   blt_src_in;
    uint16_t   blt_src_out, blt_dst_in, blt_dst_out, mask_out;
    int     last, first;
    uint16_t   xc;

#if 0
    KDEBUG(("do_blit(): Start\n"));
    /*
     * note: because HOP is always set to source, the halftone RAM
     * and the starting halftone line number (status&0x0f) are not
     * used and so are not dumped at the moment ...
     */
    KDEBUG(("X COUNT %u\n",blt->x_cnt));
    KDEBUG(("Y COUNT %u\n",blt->y_cnt));
    KDEBUG(("X S INC %d\n",blt->src_x_inc));
    KDEBUG(("Y S INC %d\n",blt->src_y_inc));
    KDEBUG(("X D INC %d\n",blt->dst_x_inc));
    KDEBUG(("Y D INC %d\n",blt->dst_y_inc));
    KDEBUG(("ENDMASK 0x%04x-%04x-%04x\n",(UWORD)blt->end_1,(UWORD)blt->end_2,(UWORD)blt->end_3));
    KDEBUG(("S_ADDR  %p\n",(UWORD *)blt->src_addr));
    KDEBUG(("D_ADDR  %p\n",(UWORD *)blt->dst_addr));
    KDEBUG(("HOP %d, OP %d\n",blt->hop&0x03,blt->op&0x0f));
    KDEBUG(("NFSR=%d,FXSR=%d,SKEW=%d\n",
            (blt->skew&NFSR)!=0,(blt->skew&FXSR)!=0,(blt->skew & SKEW)));
#endif
    do {
        xc = blt->x_cnt;
        first = 1;
        blt_src_in = 0;
        do {
            last = (xc == 1);
            /* read source into blt_src_in */
            if (blt->src_x_inc >= 0) {
                if (first && (blt->skew & FXSR)) {
                    blt_src_in = GetMemW (blt->src_addr);
                    blt->src_addr += blt->src_x_inc;
                }
                blt_src_in <<= 16;

                if (last && (blt->skew & NFSR)) {
                    blt->src_addr -= blt->src_x_inc;
                } else {
                    blt_src_in |= GetMemW (blt->src_addr);
                    if (!last) {
                        blt->src_addr += blt->src_x_inc;
                    }
                }
            } else {
                if (first &&  (blt->skew & FXSR)) {
                    blt_src_in = GetMemW (blt->src_addr);
                    blt->src_addr +=blt->src_x_inc;
                } else {
                    blt_src_in >>= 16;
                }
                if (last && (blt->skew & NFSR)) {
                    blt->src_addr -= blt->src_x_inc;
                } else {
                    blt_src_in |= (GetMemW (blt->src_addr) << 16);
                    if (!last) {
                        blt->src_addr += blt->src_x_inc;
                    }
                }
            }
            /* shift blt->skew times into blt_src_out */
            blt_src_out = blt_src_in >> (blt->skew & SKEW);

            /* read destination into blt_dst_in */
            blt_dst_in = GetMemW (blt->dst_addr);
            /* op into blt_dst_out */
            switch (blt->op & 0xf) {
            case BM_ALL_WHITE:
                blt_dst_out = 0;
                break;
            case BM_S_AND_D:
                blt_dst_out = blt_src_out & blt_dst_in;
                break;
            case BM_S_AND_NOTD:
                blt_dst_out = blt_src_out & ~blt_dst_in;
                break;
            case BM_S_ONLY:
                blt_dst_out = blt_src_out;
                break;
            case BM_NOTS_AND_D:
                blt_dst_out = ~blt_src_out & blt_dst_in;
                break;
            case BM_D_ONLY:
                blt_dst_out = blt_dst_in;
                break;
            case BM_S_XOR_D:
                blt_dst_out = blt_src_out ^ blt_dst_in;
                break;
            case BM_S_OR_D:
                blt_dst_out = blt_src_out | blt_dst_in;
                break;
            case BM_NOT_SORD:
                blt_dst_out = ~blt_src_out & ~blt_dst_in;
                break;
            case BM_NOT_SXORD:
                blt_dst_out = ~blt_src_out ^ blt_dst_in;
                break;
            case BM_NOT_D:
                blt_dst_out = ~blt_dst_in;
                break;
            case BM_S_OR_NOTD:
                blt_dst_out = blt_src_out | ~blt_dst_in;
                break;
            case BM_NOT_S:
                blt_dst_out = ~blt_src_out;
                break;
            case BM_NOTS_OR_D:
                blt_dst_out = ~blt_src_out | blt_dst_in;
                break;
            case BM_NOT_SANDD:
                blt_dst_out = ~blt_src_out | ~blt_dst_in;
                break;
            case BM_ALL_BLACK:
                blt_dst_out = 0xffff;
                break;
            }

            /* and endmask */
            if (first) {
                mask_out = (blt_dst_out & blt->end_1) | (blt_dst_in & ~blt->end_1);
            } else if (last) {
                mask_out = (blt_dst_out & blt->end_3) | (blt_dst_in & ~blt->end_3);
            } else {
                mask_out = (blt_dst_out & blt->end_2) | (blt_dst_in & ~blt->end_2);
            }
            SetMemW (blt->dst_addr, mask_out);
            if (!last) {
                blt->dst_addr += blt->dst_x_inc;
            }
            first = 0;
        } while(--xc != 0);
        blt->status = (blt->status + ((blt->dst_y_inc >= 0) ? 1 : 15)) & 0xef;
        blt->src_addr += blt->src_y_inc;
        blt->dst_addr += blt->dst_y_inc;
    } while(--blt->y_cnt != 0);
    /* blt->status &= ~BUSY; */
}
