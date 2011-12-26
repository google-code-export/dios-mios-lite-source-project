#include <asm.h>

# issue read command
#
#	r3	dst
#	r4	len	
#	r5	off

DVDLowReadShift:

	stwu    %sp,	-0x10(%sp)
	mflr    %r0
	stw     %r0,	8(%sp)


	add		%r5,	%r4,	%r5
	li		%r4,	0
	
	mr		%r29,	%r4
	mr		%r30,	%r5

	nop

	lwz     %r0, 8(%sp)
	mtlr    %r0
	addi    %sp, %sp, 0x10
	blr
