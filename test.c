#include "debug.h"
#include "features.h"
#include "linea.h"
#include "trap.h"
#include "vdi_funcs.h"

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
    uint16_t work_in[11];
    workstation_features_t features;
    uint16_t handle;

    // Open workstation
    //_debug("Open workstation\n");
    pb.contrl.opcode = 1;
    pb.contrl.ptsin_count = 0;
    pb.contrl.intin_count = 11;
    pb.contrl.intout_count = sizeof(features)/sizeof(int16_t);
    for(i = 0; i < 11; i++)
        pb.intin[i] = work_in[i];
    
    call_vdi(&vdipb);
    handle = pb.contrl.wkid;
    for(i = 0; i < 45; i++)
        features.words[i] = pb.intout[i];
    for(i = 0; i < 13; i++)
        features.words[45+i] = pb.ptsout.words[i];
    
    // vs_color: Set color 0
    //_debug("vs_color\n");
    pb.contrl.opcode = 14;
    pb.contrl.ptsin_count = 0;
    pb.contrl.intin_count = 4;
    pb.intin[0] = 0;
    pb.intin[1] = 0;
    pb.intin[2] = 142;
    pb.intin[3] = 0;
    call_vdi(&vdipb);

    // vsf_color
    //_debug("vsf_color\n");
    pb.contrl.opcode = 25;
    pb.intin[0] = 0;
    call_vdi(&vdipb);

    // vsf_interior
    pb.contrl.opcode = 23;
    pb.intin[0] = FIS_PATTERN;
    call_vdi(&vdipb);

    // vsf_style
    //_debug("vsf_style\n");
      pb.contrl.opcode = 25;
    pb.intin[0] = 5;
    call_vdi(&vdipb);  

    // v_bar
    //_debug("\r\nv_bar\n");
    pb.contrl.opcode = 11;
    pb.contrl.subopcode = 1;
    pb.contrl.ptsin_count = 2;
    pb.contrl.intin_count = 0;
    pb.ptsin.pts[0].x = 40;
    pb.ptsin.pts[0].y = 50;
    pb.ptsin.pts[1].x = 400;
    pb.ptsin.pts[1].y = 110;
    call_vdi(&vdipb);


    //_debug("\nvswr_mode\n");
    pb.contrl.opcode = 32;
    pb.intin[0] = MD_REPLACE;
    call_vdi(&vdipb);    

    //_debug("\vsl_color\n");
    pb.contrl.opcode = 17;
    pb.intin[0] = 2;
    call_vdi(&vdipb);

    // v_pline
    pb.ptsin.pts[0].y = 20;
    pb.ptsin.pts[1].y = 10;
    for (i = LS_SOLID; i <= LS_USER_DEFINED; i++)
    {
        vsl_type(handle, i);
        vsl_color(handle, i == 0 ? 1 : (i & (features.n_simultaneous_colors-1)));
        // v_pline        
        pb.contrl.opcode = 6;
        pb.contrl.ptsin_count = 2;
        pb.ptsin.pts[0].x = 20;        
        pb.ptsin.pts[1].x = 350;        
        call_vdi(&vdipb);
        pb.ptsin.pts[0].y += 3;
        pb.ptsin.pts[1].y += 3;
    }

    //_debug("Close workstation\n");
    pb.contrl.opcode = 2;
    pb.contrl.ptsin_count = 0;
    pb.contrl.intin_count = 0;
    pb.contrl.wkid = handle;
    call_vdi(&vdipb);
    _debug("OK\r\n");
}


