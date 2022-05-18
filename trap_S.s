| VDI trap interface

    | Exports
    .globl  _trap_handler
    .globl  _old_trap_handler
    .globl _call_vdi

    | Imports
    .global _vdi_dispatcher

.equ VDI_MAGIC, 0x78


    .ascii   "XBRA"
_old_trap_handler:
    .dc.l   0
| Trap #2 handler
_trap_handler:
    movem.l d0-d1/a0-a1,-(sp)  | Save GCC scratch registers    
    move.l  d1,-(sp)
    jbsr    _vdi_dispatcher
    addq.l  #4,sp
    movem.l (sp)+,d0-d1/a0-a1
    rte

| How user program calls the VDI, passing a VDI parameter block on the stack
_call_vdi:
    move.l  4(sp),d1
    moveq   #VDI_MAGIC,d0
    trap    #2
    rts
