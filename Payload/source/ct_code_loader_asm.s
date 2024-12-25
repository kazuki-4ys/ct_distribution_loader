#include <ppc-asm.h>

.set already_ran_flag, 0x800001B9

.global get_ct_code_loader_hook
.global get_ct_code_loader_hook_end

.macro pushStack
    stwu sp, -0x80 (sp)#124 + パディング
    mflr r0
    stw r0, 0x84 (sp)
    stmw r3, 8 (sp)
.endm

.macro popStack
    lmw r3, 8 (sp)
    lwz r12, 0x84 (sp)
    mtlr r12
    addi sp, sp, 0x80
.endm

get_ct_code_loader_hook_end:
    mflr r12
    b get_ct_code_loader_hook_end_b
get_ct_code_loader_hook:
    mflr r12
bl blTrickCommonEnd
ct_code_loader_hook:

add r3, r4, 0
lis r12, already_ran_flag@ha
lbz r12, already_ran_flag@l (r12)
cmpwi r12, 1
beq end

pushStack
lwz r12, 76 (r3)
add r12, r3, r12
mtlr r12
blrl
popStack
li r11, 1
lis r12, already_ran_flag@ha
stb r11, already_ran_flag@l (r12)

end:
get_ct_code_loader_hook_end_b:
bl blTrickCommonEnd