#ifndef LINEA_H
#define LINEA_H

#include <stdint.h>

typedef struct font_hdr
{
   int16_t     font_id;        /* Font number                        */
   int16_t     point;          /* Size in points                     */
   int8_t      name[32];       /* Name of the font                   */
   uint16_t    first_ade;      /* First character in font            */
   uint16_t    last_ade;       /* Last character in font             */
   uint16_t    top;            /* Distance: Top line    <-> Baseline */
   uint16_t    ascent;         /* Distance: Ascent line <-> Baseline */
   uint16_t    half;           /* Distance: Half line   <-> Baseline */
   uint16_t    descent;        /* Distance: Descent line<-> Baseline */
   uint16_t    bottom;         /* Distance: Bottom line <-> Baseline */
   uint16_t    max_char_width; /* Largest character width            */
   uint16_t    max_cell_width; /* Largest character cell width       */
   uint16_t    left_offset;    /* Left offset for italic (skewed)    */
   uint16_t    right_offset;   /* Right offset for italic (skewed)   */
   uint16_t    thicken;        /* Thickening factor for bold         */
   uint16_t    ul_size;        /* Width of underline                 */
   uint16_t    lighten;        /* Mask for light (0x5555)            */
   uint16_t    skew;           /* Mask for italic (0x5555)           */
   uint16_t    flags;          /* Various flags:
                                  Set for system font
                                   Bit 1: Set if horizontal offset
                                          table is in use
                                   Bit 2: Set if Motorola format
                                   Bit 3: Set if non-proportional    */
   uint8_t     *hor_table;     /* Pointer to horizontal offset table */
   uint16_t    *off_table;     /* Pointer to character offset table  */
   uint16_t    *dat_table;     /* Pointer to font image              */
   uint16_t    form_width;     /* Width of the font image            */
   uint16_t    form_height;    /* Height of the font image           */
   struct font_hdr *next_font; /* Pointer to next font header        */
} linea_font_header_t;

typedef struct
{
  int16_t  v_planes,               /*   0: # Bit planes (1, 2 or 4)     */
           v_lin_wr,               /*   2: # bytes/scanline             */
           *contrl,
           *intin,
           *ptsin,                 /*  12: Coordinates input            */
           *intout,
           *ptsout,                /*  20: Coordinates output           */
           fg_bp_1,                /*  24: Plane 0                      */
           fg_bp_2,                /*  26: Plane 1                      */
           fg_bp_3,                /*  28: Plane 2                      */
           fg_bp_4,                /*  30: Plane 3                      */
           lstlin;                 /*  32: Draw last pixel of a line    */
                                   /*      (1) or don't draw it (0)     */
  uint16_t ln_mask;                /*  34: Line pattern                 */
  int16_t  wrt_mode,               /*  36: Writing modes                */
           x1, y1, x2, y2;         /*  38: Coordinate                   */
  void     *patptr;                /*  46: Fill pattern                 */
  uint16_t patmsk;                 /*  50: Fill pattern "mask"          */
  int16_t  multifill,              /*  52: Fill pattern for planes      */
           clip,                   /*  54: Flag for clipping            */
           xmn_clip, ymn_clip,
           xmx_clip, ymx_clip,     /*  60: Clipping rectangle           */
                                   /*      Rest for text_blt:           */
           xacc_dda,               /*  64: Set to 0x8000 before text    */
                                   /*      output                       */
           dda_inc,                /*  66: Scaling increment            */
           t_sclsts,               /*  68: Scaling direction            */
           mono_status,            /*  70: Proportional font            */
           sourcex, sourcey,       /*  72: Coordinates in font          */
           destx, desty,           /*  76: Screen coordinates           */
           delx, dely;             /*  80: Width and height of character*/
  linea_font_header_t *fbase;      /*  84: Pointer to font data         */
  int16_t  fwidth,                 /*  88: Width of font form           */
           style;                  /*  90: Font style effect            */
  uint16_t litemask,               /*  92: Mask for light               */
           skewmask;               /*  94: Mask for italic              */
  int16_t  weight,                 /*  96: Width for bold               */
           r_off,                  /*  98: Italic offset right          */
           l_off,                  /* 100: Italic offset left           */
           scale,                  /* 102: Scaling flag yes/no          */
           chup,                   /* 104: Character rotation angle *10 */
           text_fg;                /* 106: Text foreground colour       */
  void     *scrtchp;               /* 108: Pointer to 2 contiguous      */
                                   /*      scratch buffers              */
  int16_t  scrpt2,                 /* 112: Index in buffer              */
           text_bg,                /* 114: Unused                       */
           copy_tran,              /* 116: --                           */
           (*fill_abort)( void );  /* 118: Tests seedfill               */
} linea_vars_t;


extern linea_vars_t *linea_vars;
extern linea_font_header_t *linea_fonts;

void linea_init(void);

#endif