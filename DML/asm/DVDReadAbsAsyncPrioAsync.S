#include <asm.h>

# issue read command
#
#	r3	dvdstruct
#	r4	dst	
#	r5	len
#	r6	off
#	r7	cb?
#	r8	Prio

DVDReadAbsAsyncPrioAsync:

	stwu    %sp, -0x10(%sp)
	mflr    %r0
	stw     %r0, 8(%sp)

#send cmd to DM

	li		%r0,	0
	srwi	%r8,	%r5,	5
	mtctr	%r8
	mr		%r8,	%r4
invalidloop:
	dcbi	%r0,	%r8
	addi	%r8,	%r8,	0x20
	bdnz	invalidloop

	lis     %r8,	0xC000
	
	lis     %r0,	0xA900
	stw     %r0,	0x2F08(%r8)
	
	srwi    %r0,	%r6,	2
	stw     %r0,	0x2F0C(%r8)
	stw     %r4,	0x2F14(%r8)

	stw     %r5,	0x2F10(%r8)
	stw     %r5,	0x2F18(%r8)
	
	stw     %r3,	0x2F24(%r8)

	li      %r0,	3
	stw     %r0,	0x2F1C(%r8)

	li		%r0,	0
	lis		%r8,	0xC000
	addi	%r8,	%r8,	0x2F00
#IPC area
	dcbst	%r0,	%r8
	sc

#update dvdstruct

	li		%r0,	0
	stw     %r0,	0x00(%r3)
	stw     %r0,	0x04(%r3)
	li		%r0,	1
	stw     %r0,	0x08(%r3)

	li		%r0,	2
	stw     %r0,	0x0C(%r3)

#offset
	stw     %r6,	0x10(%r3)
	
#ptr
	stw     %r4,	0x18(%r3)

#TransferSize
	stw     %r5,	0x1C(%r3)

#	lwz		%r0,	0x30(%r3)
#size
	stw     %r5,	0x14(%r3)
	stw     %r5,	0x20(%r3)

	stw     %r7,	0x28(%r3)

	li      %r3,	1

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
