#include <asm.h>

# issue read command
#
#	r3	dvdstruct
#	r4	dst	
#	r5	cb	

DVDReadDiscIDAsync:

	stwu    %sp, -0x10(%sp)
	mflr    %r0
	stw     %r0, 8(%sp)

	stw     %r5,	0x28(%r3)

	li      %r5,	0x20

#send cmd to DM

	li		%r0,	0
	lis		%r8,	0xC000
	addi	%r8,	%r8,	0x2F00
#IPC area
	dcbi	%r0,	%r8

	dcbi	%r0,	%r4

	lis     %r8,	0xC000

#dvdstruct for IRQ	
	stw     %r3,	0x2F24(%r8)
	
	lis     %r0,	0xA900
	stw     %r0,	0x2F08(%r8)
	
	li		%r0,	0
	stw     %r0,	0x2F0C(%r8)
	stw     %r4,	0x2F14(%r8)

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

	li		%r0,	0
	stw     %r0,	0x00(%r3)
	stw     %r0,	0x04(%r3)
	stw     %r0,	0x08(%r3)
	stw     %r0,	0x10(%r3)

	li		%r0,	2
	stw     %r0,	0x0C(%r3)

#TransferSize
	stw     %r5,	0x1C(%r3)

	lwz		%r0,	0x30(%r3)
#size
	stw     %r0,	0x14(%r3)
	stw     %r0,	0x20(%r3)

#ptr
	stw     %r4,	0x18(%r3)
	
#send dvd cmd to cause irq
	lis		%r5,	0xCC00
	lis		%r0,	0xE000
#enable IRQs
	li		%r0,	0x2A
	stw		%r0,	0x6000(%r5)
#send cmd
	li		%r0,	1
	stw		%r0,	0x601C(%r5)

#	li      %r3,	1

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
