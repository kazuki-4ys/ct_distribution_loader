#include <ppc-asm.h>
.set region, REGION_ID

.global getSystemHeap
.global blTrickCommonEnd

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