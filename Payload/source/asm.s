#include <ppc-asm.h>
.set region, 'P'

.global ICInvalidateRangeAsm
.global getSystemHeap
.global blTrickCommonEnd

ICInvalidateRangeAsm:
	cmplwi r4, 0   # zero or negative size?
	blelr
	clrlwi. r5, r3, 27  # check for lower bits set in address
	beq 1f
	addi r4, r4, 0x20 
1:
	addi r4, r4, 0x1f
	srwi r4, r4, 5
	mtctr r4
2:
	icbi r0, r3
	addi r3, r3, 0x20
	bdnz 2b
	sync
	isync
	blr

#by vega
#https://mariokartwii.com/showthread.php?tid=1218
getSystemHeap:
.if    (region == 'E' || region == 'e')
        lwz r3, -0x5CA8(r13)
.elseif (region == 'P' || region == 'p')
        lwz r3, -0x5CA0(r13)
.elseif (region == 'J' || region == 'j')
        lwz r3, -0x5CA0(r13)
.elseif (region == 'K' || region == 'k')
        lwz r3, -0x5C80(r13)
.else
		.abort
.endif
    lwz r3, 0x24(r3)
    blr

blTrickCommonEnd:
    mflr r3
    mtlr r12
    blr