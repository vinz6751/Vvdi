#include "debug.h"
#include "features.h"
#include "linea.h"
#include "trap.h"

extern void vdi_install(void);
extern void vdi_uninstall(void);

static void tests(void);
uint16_t call_vdi(vdi_pb_t *pb);

int main(void)
{
    // So we can get access to fonts
    linea_init();

    // Install trap handler
    vdi_install();

    tests();
    
    vdi_uninstall();

    return 0;
}


#define PTSIN_SIZE 256
#define PTSOUT_SIZE 256
#define INTIN_SIZE 256
#define INTOUT_SIZE 256
struct {
    vdi_pb_contrl_t contrl;
    uint16_t intin[INTIN_SIZE];
    union {
        uint16_t words[PTSIN_SIZE];
        vdi_point_t pts[(PTSIN_SIZE*sizeof(uint16_t))/sizeof(vdi_point_t)];
    } ptsin;
    uint16_t intout[INTOUT_SIZE];
    union {
        uint16_t words[PTSOUT_SIZE];
        vdi_point_t pts[(PTSOUT_SIZE*sizeof(uint16_t))/sizeof(vdi_point_t)];
    } ptsout;
} pb;

// void call_vdi2(vdi_pb_t *b) {
//     _debug("call_vdi2 %p",b);
// }

static void tests(void) {
    vdi_pb_t vdipb;

    vdipb.contrl = &pb.contrl;
    vdipb.intin = pb.intin;
    vdipb.intout = pb.intout;
    vdipb.ptsin.words = pb.ptsin.words;
    vdipb.ptsout.words = pb.ptsout.words;

    int i;
    uint16_t work_in[11],work_out[57];
    uint16_t handle;

    // Open workstation
    _debug("Open workstation\n");
    pb.contrl.opcode = 1;
    pb.contrl.ptsin_count = 0;
    pb.contrl.intin_count = 11;
    pb.contrl.intout_count = 57;
    for(i = 0; i < 11; i++)
        pb.intin[i] = work_in[i];
    _debug("PB=%p",&vdipb);
    call_vdi(&vdipb);
    handle = pb.contrl.wkid;
    for(i = 0; i < 45; i++)
        work_out[i] = pb.intout[i];
    for(i = 0; i < 13; i++)
        work_out[45+i] = pb.ptsout.words[i];
    _debug("OK");

    // vs_color: Set color 0
    _debug("vs_color\n");
    pb.contrl.opcode = 14;
    pb.contrl.ptsin_count = 0;
    pb.contrl.intin_count = 4;
    pb.intin[0] = 0;
    pb.intin[1] = 0;
    pb.intin[2] = 142;
    pb.intin[3] = 0;
    call_vdi(&vdipb);

    // vsf_color
    _debug("vsf_color\n");
    pb.contrl.opcode = 25;
    pb.intin[0] = 0;
    call_vdi(&vdipb);

    // vsf_interior
    pb.contrl.opcode = 23;
    pb.intin[0] = FIS_SOLID;
    call_vdi(&vdipb);

    // vsf_style
    _debug("vsf_style\n");
      pb.contrl.opcode = 25;
    pb.intin[0] = 5;
    call_vdi(&vdipb);  

    // v_bar
    _debug("v_bar\n");
    pb.contrl.opcode = 11;
    pb.contrl.subopcode = 1;
    pb.contrl.ptsin_count = 2;
    pb.contrl.intin_count = 0;
    pb.ptsin.pts[0].x = 40;
    pb.ptsin.pts[0].y = 50;
    pb.ptsin.pts[1].x = 400;
    pb.ptsin.pts[1].y = 110;
    call_vdi(&vdipb);


    _debug("Close workstation\n");
    pb.contrl.opcode = 2;
    pb.contrl.ptsin_count = 0;
    pb.contrl.intin_count = 0;
    pb.contrl.wkid = handle;
    call_vdi(&vdipb);
    _debug("OK\n");
}


