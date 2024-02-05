// VDI Trap interface

#include "debug.h"
#include "font.h"
#include "fonthdr.h"
#include "vdi_funcs.h"
#include "raster.h"
#include "text.h"
#include "trap.h"

#include <osbind.h>

extern void trap_handler(void);
#define TRAP2_VECNR 34
extern void (*old_trap_handler)();

void trap_install(void) {
    old_trap_handler = Setexc(TRAP2_VECNR, -1L);
    Setexc(TRAP2_VECNR,trap_handler);
}

void trap_uninstall(void) {
    Setexc(TRAP2_VECNR,old_trap_handler);
}

// Trap handler, function dispatcher -------------------------------------------
// TODO set ptsout, intout
static void vdi_v_opnwk(vdi_pb_t *pb) { v_opnwk(pb->intin, &pb->contrl->wkid, pb->intout); }
static void vdi_v_clswk(vdi_pb_t *pb) { v_clswk(pb->contrl->wkid); }
static void vdi_v_clrwk(vdi_pb_t *pb) { v_clrwk(pb->contrl->wkid); }
static void vdi_vs_clip(vdi_pb_t *pb) { vs_clip(pb->contrl->wkid, pb->intin[0], pb->ptsin.rect); }
static void vdi_vs_color(vdi_pb_t *pb) { vs_color(pb->contrl->wkid, pb->intin[0], &pb->intin[1]); }
static void vdi_vq_color(vdi_pb_t *pb) { pb->intout[0] = vq_color(pb->contrl->wkid, pb->intin[0], pb->intin[1], &(pb->intout[1])); pb->contrl->intout_count = 4; }
static void vdi_vsl_color(vdi_pb_t *pb) { pb->intout[0] = vsl_color(pb->contrl->wkid, pb->intin[0]); }
static void vdi_vsl_type(vdi_pb_t *pb) { pb->intout[0] = vsl_type(pb->contrl->wkid, pb->intin[0]); }
static void vdi_vsl_udsty(vdi_pb_t *pb) { vsl_udsty(pb->contrl->wkid, pb->intin[0]); }
static void vdi_vsf_color(vdi_pb_t *pb) { pb->intout[0] = vsf_color(pb->contrl->wkid, pb->intin[0]); }
static void vdi_vsf_perimeter(vdi_pb_t *pb) { pb->intout[0] = vsf_perimeter(pb->contrl->wkid, pb->intin[0]); }
static void vdi_vsf_style(vdi_pb_t *pb) { pb->intout[0] = vsf_style(pb->contrl->wkid, pb->intin[0]); pb->contrl->intout_count = 1; }
static void vdi_vsf_interior(vdi_pb_t *pb) { pb->intout[0] = vsf_interior(pb->contrl->wkid, pb->intin[0]); pb->contrl->intout_count = 1; }
static void vdi_vqf_attributes(vdi_pb_t *pb) { vqf_attributes(pb->contrl->wkid, pb->intout); pb->contrl->intout_count = 5; }
static void vdi_vswr_mode(vdi_pb_t *pb) { pb->intout[0] = vswr_mode(pb->contrl->wkid, pb->intin[0]); pb->contrl->intout_count = 1; }
static void vdi_vr_recfl(vdi_pb_t *pb) { vr_recfl(pb->contrl->wkid, pb->ptsin.rect); }
static void vdi_v_gdp(vdi_pb_t *pb); // See below
static void vdi_v_pline(vdi_pb_t *pb) { v_pline(pb->contrl->wkid, pb->contrl->ptsin_count, pb->ptsin.pts); }
static void vdi_v_bar(vdi_pb_t *pb) { v_bar(pb->contrl->wkid, pb->ptsin.rect); }
static void vdi_v_fontinit(vdi_pb_t *pb) { v_fontinit(pb->contrl->wkid, pb->intin[0], pb->intin[1]); }
static void vdi_v_gtext(vdi_pb_t *pb) { v_gtext(pb->contrl->wkid, pb->ptsin.pts[0], pb->intin); }
static void vdi_vst_color(vdi_pb_t *pb) { pb->intout[0] = vst_color(pb->contrl->wkid, pb->intin[0]); pb->contrl->intout_count = 1; }
static void void_vst_height(vdi_pb_t *pb) { vst_height(pb->contrl->wkid, pb->ptsin.words[1], &(pb->ptsout.words[0]), &(pb->ptsout.words[1]), &(pb->ptsout.words[2]), &(pb->ptsout.words[3])); }
static void vdi_vqt_name(vdi_pb_t *pb) { pb->intout[0] = vqt_name(pb->contrl->wkid, pb->intin[0], &(pb->intout[1])); pb->contrl->intout_count = 1; }
static void vdi_vr_trnfm(vdi_pb_t *pb) { vr_trnfm(*(MFDB **)&(pb->contrl->word[7]), *(MFDB **)&(pb->contrl->word[9])); }
static void vdi_v_get_pixel(vdi_pb_t *pb) { v_get_pixel(pb->contrl->wkid, pb->ptsin.words[0], pb->ptsin.words[1], &(pb->intout[0]), &(pb->intout[1]) ); }

static void (*const vdi_calls[])(vdi_pb_t *) = {
    0L,
    vdi_v_opnwk, // 1
    vdi_v_clswk, // 2
    vdi_v_clrwk, // 3
};

void vdi_dispatcher(vdi_pb_t *pb) {

    //_debug("DISPATCHING pb=%p, opcode: %d\n", pb, pb->contrl->opcode);

    switch (pb->contrl->opcode) {
        case 5: vdi_v_fontinit(pb); break;
        case 6: vdi_v_pline(pb); break;
        case 8: vdi_v_gtext(pb); break;
        case 11: vdi_v_gdp(pb); break;
        case 14: vdi_vs_color(pb); break;
        case 15: vdi_vsl_type(pb); break;
        case 17: vdi_vsl_color(pb); break;
        case 22: vdi_vst_color(pb); break;
        case 23: vdi_vsf_interior(pb); break;
        case 24: vdi_vsf_style(pb); break;
        case 25: vdi_vsf_color(pb); break;
        case 26: vdi_vq_color(pb); break;
        case 32: vdi_vswr_mode(pb); break;
        case 37: vdi_vqf_attributes(pb); break;
        case 104: vdi_vsf_perimeter(pb); break;
        case 105: vdi_v_get_pixel(pb); break;
        case 110: vdi_vr_trnfm(pb); break;
        case 113: vdi_vsl_udsty(pb); break;
        case 129: vdi_vs_clip(pb); break;
        case 130: vdi_vqt_name(pb); break;
        default:
            (*vdi_calls[pb->contrl->opcode])(pb);
            return;
    }    
}


static void vdi_v_gdp(vdi_pb_t *pb)
{
    //_debug("vdi_v_gdp: subopcode = %u", pb->contrl->subopcode);
    switch (pb->contrl->subopcode) {
    case 1:         /* GDP BAR - converted to alpha 2 RJG 12-1-84 */
        vdi_v_bar(pb);
        break;

    case 2:         /* GDP Arc */
    case 3:         /* GDP Pieslice */
    case 4:         /* GDP Circle */
    case 5:         /* GDP Ellipse */
    case 6:         /* GDP Elliptical Arc */
    case 7:         /* GDP Elliptical Pieslice */
    case 8:         /* GDP Rounded Box */
    case 9:         /* GDP Rounded Filled Box */
    case 10:        /* GDP Justified Text */
    default:
        break;
    }
}
