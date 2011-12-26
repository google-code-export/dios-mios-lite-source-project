
DVDWaitReady:

	li		%r0,	0
	lis		%r7,	0x8000
	addi	%r7,	%r7,	0x2F20
	dcbf	%r0,	%r7
	sc
	
	lis		%r7,	0x8000
	lwz     %r0,	0x2F30(%r7)
	stw     %r0,	0x2F2C(%r7)
	andi.   %r0,	%r0,		0x14
	cmpwi   %r0,	0
	beq		NotReady
	
#send dvd cmd to cause irq
	lis		%r5,	0xCC00

	lis		%r0,	0xAB00
	stw		%r0,	0x6008(%r5)
#enable IRQs
	li		%r0,	0x2A
	stw		%r0,	0x6000(%r5)
#send cmd
	li		%r0,	1
	stw		%r0,	0x601C(%r5)
	
	li		%r0,	0
	stw     %r0,	0x2F30(%r7)

	b		boing

NotReady:
	li		%r0,	23
	stw     %r0,	0x2F28(%r7)

boing:
	lwz       %r0, 0x2C(%sp)
	lwz       %r31, 0x24(%sp)
	lwz       %r30, 0x20(%sp)
	lwz       %r29, 0x1C(%sp)
	addi      %sp, %sp, 0x28
	mtlr      %r0
	blr

	lwz		%r0,	0x24(%sp)
	lwz		%r31,	0x1C(%sp)
	lwz		%r29,	0x14(%sp)
	lwz		%r28,	0x10(%sp)
	addi	%sp,	%sp,		0x20
	mtlr	%r0
	blr
