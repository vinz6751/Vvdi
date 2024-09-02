| VDI trap interface

    | Exports
    .globl  trap_handler
    .globl  old_trap_handler
    .globl  call_vdi

    | Imports
    .global _vdi_dia7atcher

.equ VDI_MAGIC, 0x78


    .ascii   "XBRA"
old_trap_handler:
    .dc.l   0
| Trap #2 handler
trap_handler:
    movem.l %d0-%d1/%a0-%a1,-(%a7)  | Save GCC scratch registers    
    move.l  %d1,-(%a7)
    jbsr    vdi_dispatcher
    addq.l  #4,a7
    movem.l (%a7)+,%d0-%d1/%a0-%a1
    rte

| How user program calls the VDI, passing a VDI parameter block on the stack
call_vdi:
    move.l  4(%a7),%d1
    moveq   #VDI_MAGIC,%d0
    trap    #2
    rts
