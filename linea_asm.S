    .globl linea_init
    .globl linea_vars
    .globl linea_fonts
    .globl linea_routs

linea_init:
    movem.l   d0-d2/a0-a2,-(sp)  // Save registers
    .dc.w     0xA000             // Line-A opcode
    move.l    a0,linea_vars      // Pointer parameter block LINEA
    move.l    a1,linea_fonts     // Pointer system fonts
    move.l    a2,linea_routs     // Pointer start addr. Line-A routines
    movem.l   (a7)+,d0-d2/a0-a2  // Restore registers
    rts

linea_vars:  .ds.l 1
linea_fonts: .ds.l 1
linea_routs: .ds.l 1
