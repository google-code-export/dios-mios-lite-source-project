#include <asm.h>

#
#	r3	Channel

CARDCheck:


	mflr	%r0
	cmpwi	%r3, 0
	beq		CARDPresent

	li      %r3, -3
	b		end

CARDPresent:
	li		%r3, 0
end:

	mtlr    %r0
	blr
