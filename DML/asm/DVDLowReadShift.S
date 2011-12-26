#include <asm.h>

# issue read command
#
#	%r3	dst
#	%r4	len	
#	%r5	off
#	%r6	cb

DVDLowReadShift:

	stwu    %sp,	-16(%sp)
	mflr    %r0
	stw     %r0,	20(%sp)
	stw     %r30,	8(%sp)
	stw     %r31,	12(%sp)

	li      %r0,	0
	stw     %r6,	-32208(%r13)
	stw     %r0,	-32248(%r13)
	
	li		%r0,	0
	lis		%r7,	0xC000
	addi	%r7,	%r7,	0x2F00
#IPC area
	dcbi	%r0,	%r7

	lis     %r7,	0xC000
	lis     %r0,	0xA800
	stw     %r0,	0x2F08(%r7)
	
	srwi	%r0,	%r5,	2
	stw     %r0,	0x2F0C(%r7)
	
	stw     %r3,	0x2F14(%r7)

	stw     %r4,	0x2F10(%r7)
	stw     %r4,	0x2F18(%r7)

	li      %r6,	3
	stw     %r6,	0x2F1C(%r7)
	
ready_loop:
	lwz		%r6,	0x2F1C(%r7)
	cmpwi	%r6,	3
	beq		ready_loop
		
wait_loop:
	lwz		%r6,	0x2F30(%r7)
	andi.	%r6,	%r6,	0x14
	cmpwi	%r6,	0
	beq		wait_loop

#send dvd cmd to cause irq
	lis		%r7,	0xCC00
	
	lis		%r0,	0xAB00
	stw		%r0,	0x6008(%r7)

	add     %r3,	%r3,		%r4
	stw     %r3,	0x6014(%r7)
	stw     %r4,	0x6010(%r7)
	
	li		%r0,	0
	stw		%r0,	0x6018(%r7)

	li		%r0,	1
	stw		%r0,	0x601C(%r7)

dvd_loop:
	lwz		%r0,	0x6000(%r7)
	andi.	%r0,	%r0,	0x14
	cmpwi	%r0,	0
	beq		dvd_loop

	li      %r3,	1

	lwz		%r31,	12(%sp)
	lwz     %r30,	8(%sp)
	lwz     %r0,	20(%sp)
	mtlr    %r0
	addi    %sp,	%sp,		16
	blr
