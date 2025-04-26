#include <ppc-asm.h>
.set region, REGION_ID

.global cacheInvalidateAddress
.global syncAndIsync
.global pulsar1xLoaderEntryAsm
.global getPulsar1xLoaderEntryAsmPtr

cacheInvalidateAddress:
dcbst 0, r3
sync
icbi 0, r3
blr

syncAndIsync:
sync
isync
blr

getPulsar1xLoaderEntryAsmPtr:
mflr r12
bl blTrickCommonEnd
pulsar1xLoaderEntryAsm:
b pulsar1xLoaderEntry