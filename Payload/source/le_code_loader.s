#include <ppc-asm.h>

.set region, 'P'

.global get_le_code_loader_hook
.global get_le_code_loader_hook_end

.set r1, sp
.set r2, toc

get_le_code_loader_hook_end:
    mflr r12
    b get_le_code_loader_hook_end_b
get_le_code_loader_hook:
    mflr r12
bl blTrickCommonEnd
le_code_loader_hook:
    #by Leseratte
    #https://mariokartwii.com/showthread.php?tid=1622
.if    (region == 'E' || region == 'e')
        .incbin "C:/Users/Kazuki/Desktop/develop/Wii/ASM/ct_distribution_loader/Payload/source/le_code_loder_e.bin"
.elseif (region == 'P' || region == 'p')
        .incbin "C:/Users/Kazuki/Desktop/develop/Wii/ASM/ct_distribution_loader/Payload/source/le_code_loder_p.bin"
.elseif (region == 'J' || region == 'j')
        .incbin "C:/Users/Kazuki/Desktop/develop/Wii/ASM/ct_distribution_loader/Payload/source/le_code_loder_j.bin"
.else
		.abort
.endif
    .long 0
get_le_code_loader_hook_end_b:
    bl blTrickCommonEnd