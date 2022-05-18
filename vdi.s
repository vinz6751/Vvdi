#NO_APP
	.text
	.even
	.globl	_vdi_install
_vdi_install:
	movem.l #12320,-(%sp)
	moveq #34,%d3
	moveq #-1,%d0
#APP
| 139 "vdi.c" 1
	movl	%d0,%sp@-
	movw	%d3,%sp@-
	movw	#5,%sp@-
	trap	#13
	addql	#8,%sp
| 0 "" 2
#NO_APP
	move.l %d0,_old_trap_handler
	move.l #_trap_handler,%d0
#APP
| 140 "vdi.c" 1
	movl	%d0,%sp@-
	movw	%d3,%sp@-
	movw	#5,%sp@-
	trap	#13
	addql	#8,%sp
| 0 "" 2
#NO_APP
	movem.l (%sp)+,#1036
	rts
	.even
	.globl	_vdi_uninstall
_vdi_uninstall:
	movem.l #12320,-(%sp)
	moveq #34,%d3
	moveq #-1,%d0
#APP
| 146 "vdi.c" 1
	movl	%d0,%sp@-
	movw	%d3,%sp@-
	movw	#5,%sp@-
	trap	#13
	addql	#8,%sp
| 0 "" 2
#NO_APP
	move.l %d0,_old_trap_handler
	move.l #_trap_handler,%d0
#APP
| 147 "vdi.c" 1
	movl	%d0,%sp@-
	movw	%d3,%sp@-
	movw	#5,%sp@-
	trap	#13
	addql	#8,%sp
| 0 "" 2
#NO_APP
	movem.l (%sp)+,#1036
	rts
.LC0:
	.ascii "DISPATCHING pb=%p\12\0"
	.even
	.globl	_vdi_dispatcher
_vdi_dispatcher:
	move.l 4(%sp),-(%sp)
	pea .LC0
	jsr _printf
	addq.l #8,%sp
	rts
	.even
	.globl	_workstation_init
_workstation_init:
	lea _workstation+26,%a0
	moveq #0,%d0
.L5:
	move.w %d0,(%a0)
	clr.b -26(%a0)
	move.b #1,-2(%a0)
	addq.l #1,%d0
	lea (58,%a0),%a0
	moveq #8,%d1
	cmp.l %d0,%d1
	jne .L5
	rts
.LC1:
	.ascii "call_vdi2 %p\12\0"
	.even
	.globl	_call_vdi2
_call_vdi2:
	move.l 4(%sp),-(%sp)
	pea .LC1
	jsr _printf
	addq.l #8,%sp
	rts
.LC2:
	.ascii "PB=%p\12\0"
.LC3:
	.ascii "Open workstation\0"
.LC4:
	.ascii "OK\0"
.LC5:
	.ascii "vs_color\0"
.LC6:
	.ascii "Close workstation\0"
	.even
	.globl	_main
_main:
	lea (-44,%sp),%sp
	movem.l #60,-(%sp)
	jsr ___main
	jsr _linea_init
	jsr _vdi_install
	move.l #_pb,40(%sp)
	move.l #_pb+24,44(%sp)
	move.l #_pb+1048,52(%sp)
	move.l #_pb+536,48(%sp)
	move.l #_pb+1560,56(%sp)
	lea (40,%sp),%a3
	move.l %a3,-(%sp)
	pea .LC2
	lea _printf,%a5
	jsr (%a5)
	pea .LC3
	lea _puts,%a4
	jsr (%a4)
	move.w #1,_pb
	clr.w _pb+2
	move.w #11,_pb+6
	move.w #57,_pb+8
	lea (30,%sp),%a2
	lea _pb+24,%a0
	lea (12,%sp),%sp
.L11:
	move.w (%a2)+,(%a0)+
	cmp.l %a2,%a3
	jne .L11
	move.l %a2,-(%sp)
	pea .LC2
	jsr (%a5)
	move.l %a2,-(%sp)
	jsr _call_vdi2
	move.w _pb+12,%a5
	pea .LC4
	jsr (%a4)
	pea .LC5
	jsr (%a4)
	move.w #14,_pb
	clr.w _pb+2
	move.w #4,_pb+6
	clr.w _pb+8
	clr.w _pb+24
	move.w #1000,_pb+26
	move.w #250,_pb+28
	move.w #250,_pb+30
	move.l %a2,-(%sp)
	lea _call_vdi,%a3
	jsr (%a3)
	pea .LC6
	jsr (%a4)
	move.w #2,_pb
	clr.w _pb+2
	clr.w _pb+6
	move.w %a5,_pb+12
	move.l %a2,-(%sp)
	jsr (%a3)
	lea (28,%sp),%sp
	move.l #.LC4,(%sp)
	jsr (%a4)
	jsr _vdi_uninstall
	addq.l #4,%sp
	moveq #0,%d0
	movem.l (%sp)+,#15360
	lea (44,%sp),%sp
	rts
.comm _pb,2072
.comm _workstation,464
	.globl	_work_out
	.even
_work_out:
	.word	0
	.word	0
	.word	0
	.word	372
	.word	372
	.word	3
	.word	7
	.word	0
	.word	6
	.word	0
	.word	1
	.word	24
	.word	12
	.word	0
	.word	1
	.word	1
	.word	2
	.word	3
	.word	4
	.word	5
	.word	6
	.word	7
	.word	8
	.word	9
	.word	10
	.word	3
	.word	0
	.word	3
	.word	3
	.word	3
	.word	0
	.word	3
	.word	0
	.word	3
	.word	1
	.word	0
	.word	0
	.word	0
	.word	0
	.word	2
	.word	1
	.word	1
	.word	1
	.word	0
	.word	5
	.word	4
	.word	7
	.word	13
	.word	1
	.word	0
	.word	40
	.word	0
	.word	15
	.word	11
	.word	16
	.word	16
	.globl	_driver
	.data
	.even
_driver:
	.long	_shifter_driver
