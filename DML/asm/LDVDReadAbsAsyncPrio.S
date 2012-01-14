#include <asm.h>

# issue read command
#
#	r3	?
#	r4	dvdstruct

DVDReadAbsAsyncPrio:

	stwu    %sp, -0x10(%sp)
	mflr    %r0
	stw     %r0, 8(%sp)
	
	lwz		%r3,	0x10(%r4)
	lwz		%r5,	0x14(%r4)
	lwz		%r6,	0x18(%r4)

#send cmd to DM

	li		%r0,	0
	lis		%r8,	0xC000
	addi	%r8,	%r8,	0x2F00
#IPC area
	dcbi	%r0,	%r8

	srwi	%r8,	%r5,	5
	mtctr	%r8
	mr		%r8,	%r6
invalidloop:
	dcbi	%r0,	%r8
	addi	%r8,	%r8,	0x20
	bdnz	invalidloop

	lis     %r8,	0xC000
	
	lis     %r0,	0xA900
	stw     %r0,	0x2F08(%r8)
	
	srwi    %r0,	%r3,	2
	stw     %r0,	0x2F0C(%r8)
	stw     %r6,	0x2F14(%r8)

	stw     %r5,	0x2F10(%r8)
	stw     %r5,	0x2F18(%r8)
	
	li      %r0,	3
	stw     %r0,	0x2F1C(%r8)
	
ready_loop:
	lwz		%r0,	0x2F1C(%r8)
	cmpwi	%r0,	3
	beq		ready_loop

wait_loop:
	lwz		%r0,	0x2F30(%r8)
	andi.	%r0,	%r0,	0x14
	cmpwi	%r0,	0
	beq		wait_loop

#update dvdstruct

#TransferSize
	stw     %r5,	0x1C(%r4)

#size
	stw     %r5,	0x14(%r4)
	stw     %r5,	0x20(%r4)

	li		%r3,	0

	stw     %r3,	0x0C(%r4)
	
	li		%r3,	1

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
