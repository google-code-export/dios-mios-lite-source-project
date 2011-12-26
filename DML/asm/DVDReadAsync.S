#include <asm.h>

# issue read command
#
#	r3	dvdstruct
#	r4	dst	
#	r5	len
#	r6	off
#	r7	cb

DVDRead:

	stwu    %sp, -0x10(%sp)
	mflr    %r0
	stw     %r0, 8(%sp)

	stw     %r7,	0x28(%r3)
	
#send cmd to DM

	li		%r0,	0
	lis		%r7,	0xC000
	addi	%r7,	%r7,	0x2F00
#IPC area
	dcbi	%r0,	%r7

	srwi	%r7,	%r5,	5
	mtctr	%r7
	mr		%r7,	%r4
invalidloop:
	dcbi	%r0,	%r7
	addi	%r7,	%r7,	0x20
	bdnz	invalidloop

	lis     %r7,	0xC000
	lis     %r0,	0xA900
	
	stw     %r0,	0x2F08(%r7)
	
	lwz     %r0,	0x30(%r3)
	add		%r6,	%r0,	%r6
	srwi    %r0,	%r6,	2
	stw     %r0,	0x2F0C(%r7)
	stw     %r4,	0x2F14(%r7)

	stw     %r5,	0x2F10(%r7)
	stw     %r5,	0x2F18(%r7)
	
	li      %r0,	3
	stw     %r0,	0x2F1C(%r7)
	
ready_loop:
	lwz		%r0,	0x2F1C(%r7)
	cmpwi	%r0,	3
	beq		ready_loop

wait_loop:
	lwz		%r0,	0x2F30(%r7)
	andi.	%r0,	%r0,	0x14
	cmpwi	%r0,	0
	beq		wait_loop

#update dvdstruct

	li		%r0,	0
	stw     %r0,	0x00(%r3)
	stw     %r0,	0x04(%r3)
#cmd status
	stw     %r0,	0x0C(%r3)

#last cmd was read
	li		%r0,	1
	stw     %r0,	0x08(%r3)

#offset
	stw     %r6,	0x10(%r3)
#TransferSize
	stw     %r5,	0x1C(%r3)

	lwz		%r0,	0x34(%r3)
#size
	stw     %r0,	0x14(%r3)
	stw     %r0,	0x20(%r3)

#ptr
	stw     %r4,	0x18(%r3)

#callback
	lwz     %r12,	0x28(%r3)

	cmpwi	%r12,	0
	beq		skip_cb
	mtctr	%r12
	mr		%r4,	%r3
	mr		%r3,	%r5
	bctrl

skip_cb:
	li      %r3,	1

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
