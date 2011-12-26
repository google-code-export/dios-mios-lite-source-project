#include <asm.h>

# issue read command
#
#	r3	dvdstruct
#	r4	cb

DVDStopStreamAtEndAsync:

	stwu    %sp, -0x10(%sp)
	mflr    %r0
	stw     %r0, 8(%sp)

#send cmd to DM

	li		%r0,	0
	lis		%r8,	0xC000
	addi	%r8,	%r8,	0x2F00
#IPC area
	dcbi	%r0,	%r8

	srwi	%r8,	%r5,	5
	mtctr	%r8
	mr		%r8,	%r4
invalidloop:
	dcbi	%r0,	%r8
	addi	%r8,	%r8,	0x20
	bdnz	invalidloop

	lis     %r8,	0xC000
	stw     %r3,	0x2F24(%r8)
	
	lis     %r0,	0xE100
	stw     %r0,	0x2F08(%r8)
	
	li		%r0,	0
	stw     %r0,	0x2F0C(%r8)
	stw     %r0,	0x2F10(%r8)

	li      %r0,	3
	stw     %r0,	0x2F1C(%r8)
	
#update dvdstruct

	li		%r0,	0
	stw     %r0,	0x00(%r3)
	stw     %r0,	0x04(%r3)
	stw     %r0,	0x08(%r3)

	li		%r0,	2
	stw     %r0,	0x0C(%r3)

	stw     %r4,	0x28(%r3)	

#send dvd cmd to cause irq
	lis		%r5,	0xCC00
	lis		%r0,	0xAB00
#enable IRQs
	li		%r0,	0x2A
	stw		%r0,	0x6000(%r5)
#send cmd
	li		%r0,	1
	stw		%r0,	0x601C(%r5)

	li      %r3,	1

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
