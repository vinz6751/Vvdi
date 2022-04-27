    .globl _linea_init
    .globl _linea_vars
    .globl _linea_fonts
    .globl _linea_routs

_linea_init:
    movem.l   d0-d2/a0-a2,-(sp)  // Save registers
    .dc.w     0xA000             // Line-A opcode
    move.l    a0,_linea_vars     // Pointer parameter block LINEA
    move.l    a1,_linea_fonts    // Pointer system fonts
    move.l    a2,_linea_routs    // Pointer start addr. Line-A routines
    movem.l   (a7)+,d0-d2/a0-a2  // Restore registers
    rts

_linea_vars:  .ds.l 1
_linea_fonts: .ds.l 1    
_linea_routs: .ds.l 1
