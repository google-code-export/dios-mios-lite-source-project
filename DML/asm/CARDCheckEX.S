#include <asm.h>

#
#	r3	Channel
#	r4	*xfer bytes used to repair the FS
#	r5	cb

CARDCheckEx:

	mflr	%r0
	stw     %r0, 4(%sp)
	stwu    %sp, -0x10(%sp)	

	cmpwi	%r3, 0
	beq		CARDPresent

	cmpwi	%r5, 0
	beq		NoCardZeroPtr

	li		%r6, 0
	stw		%r6, 0(%r5)

NoCardZeroPtr:
	cmpwi	%r5, 0
	beq		NoCardNoCB
	
	mtctr	%r5
	li		%r4, -3
	bctrl

NoCardNoCB:
	li      %r3, -3
	b		end

CARDPresent:

	cmpwi	%r5, 0
	beq		CardZeroPtr

	li		%r6, 0
	stw		%r6, 0(%r5)

CardZeroPtr:
	cmpwi	%r5, 0
	beq		CardNoCB

	mtctr	%r5
	li		%r4, 0
	bctrl

CardNoCB:
	li      %r3, 0
end:
	lwz     %r0, 0x14(%sp)
	addi    %sp, %sp, 0x10
	mtlr    %r0
	blr
