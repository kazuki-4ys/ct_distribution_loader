#include <ppc-asm.h>
.set region, 'P'

.global getString0
.global getString1
.global getString2
.global getString3
.global getString4
.global getString5

getString0:
    mflr r12
    bl blTrickCommonEnd
    .string "[CT Distribution Loader]Hello World From Payload!!\n"

getString1:
    mflr r12
    bl blTrickCommonEnd
    .long 0x4E800020

getString2:
    mflr r12
    bl blTrickCommonEnd
.if    (region == 'E' || region == 'e')
    .string "/codes/RMCE01.gct\0\0"
.elseif (region == 'P' || region == 'p')
    .string "/codes/RMCP01.gct\0\0"
.elseif (region == 'J' || region == 'j')
    .string "/codes/RMCJ01.gct\0\0"
.elseif (region == 'K' || region == 'k')
    .string "/codes/RMCK01.gct\0\0"
.else
    .abort
.endif

getString3:
    mflr r12
    bl blTrickCommonEnd
    .incbin "C:/Users/Kazuki/Desktop/develop/Wii/ASM/ct_distribution_loader/Payload/source/codehandler.bin"

getString4:
    mflr r12
    bl blTrickCommonEnd
    .long 0x7CE33B78
    .long 0x38870034
    .long 0x38A70038
    .long 0x38C7004C

getString5:
    mflr r12
    bl blTrickCommonEnd
.if    (region == 'E' || region == 'e')
    .string "/rel/lecode-USA.bin"
.elseif (region == 'P' || region == 'p')
    .string "/rel/lecode-PAL.bin"
.elseif (region == 'J' || region == 'j')
    .string "/rel/lecode-JAP.bin"
.elseif (region == 'K' || region == 'k')
    .string "/rel/lecode-KOR.bin"
.else
    .abort
.endif