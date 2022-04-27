| VDI assembler routines

    .globl  _vdi_trap_handler
    .globl  _old_trap_handler
    .globl _call_vdi

.equ VDI_MAGIC, 0x78

    .dc.b   'X','B','R','A'
_old_trap_handler:
    .dc.l   0
_vdi_trap_handler:
    rte

| How user program calls the VDI, passing a VDI parameter block on the stack
_call_vdi:
    move.l  4(sp),d1
    moveq   #VDI_MAGIC,d0
    trap    #2
    rts
