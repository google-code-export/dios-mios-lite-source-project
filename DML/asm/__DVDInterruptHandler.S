#include <asm.h>

# issue read command
#
#	r3	?
#	r4	?	

__DVDInterruptHandler:

	stwu    %sp,	-0x10(%sp)
	mflr    %r0
	stw     %r0,	8(%sp)
	
	lis		%r7,	0xC000
	lwz		%r3,	0x2F24(%r7)
	stw		%r3,	0x2F28(%r7)

#clear CoverIRQ
	lis		%r7,	0xCC00
	lwz		%r0,	0x6004(%r7)
	stw		%r0,	0x6004(%r7)

#clear IRQs
	lwz		%r0,	0x6000(%r7)
	andi.	%r6,	%r0,	0x2A
	andi.	%r4,	%r0,	0x54
	slwi	%r0,	%r6,	1
	and		%r0,	%r4,	%r0
	or		%r0,	%r0,	%r6
	stw		%r0,	0x6000(%r7)
	
	cmpwi	%r3,	0
	beq		skip_cb

#set cmd done
	li		%r0,	0
	stw     %r0,	0x0C(%r3)
#load transfer length
	lwz		%r5,	0x1C(%r3)
	
	lwz		%r12,	0x28(%r3)
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
