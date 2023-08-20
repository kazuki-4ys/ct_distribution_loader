#include <ppc-asm.h>
.set region, 'E'

.macro defaultInstruction
    li r0, 97
.endm

.macro pushStack
    stwu sp, -0x80 (sp)#124 + パディング
    mflr r0
    stw r0, 0x84 (sp)
    stmw r3, 8 (sp)
.endm

.macro popStack
    lmw r3, 8 (sp)
    lwz r0, 0x84 (sp)
    mtlr r0
    addi sp, sp, 0x80
.endm

.global __entry
.global __end
.global getString0
#.global ICInvalidateRangeAsm
.global getSystemHeap

.section .text.__entry
__entry:
    pushStack
    bl __main
    b __end

#ICInvalidateRangeAsm:
#	cmplwi r4, 0   # zero or negative size?
#	blelr
#	clrlwi. r5, r3, 27  # check for lower bits set in address
#	beq 1f
#	addi r4, r4, 0x20 
#1:
#	addi r4, r4, 0x1f
#	srwi r4, r4, 5
#	mtctr r4
#2:
#	icbi r0, r3
#	addi r3, r3, 0x20
#	bdnz 2b
#	sync
#	isync
#	blr

getString0:
    mflr r12
    bl string_0
.if    (region == 'E' || region == 'e')
    .string "/bin/Payload_E.bin\0"
.elseif (region == 'P' || region == 'p')
    .string "/bin/Payload_P.bin\0"
.elseif (region == 'J' || region == 'j')
    .string "/bin/Payload_J.bin\0"
.else
        .abort
.endif
    string_0:
    mflr r3
    mtlr r12
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

.section .text.__end
#このシンボル__endは必ず最後に配置し、コードハンドラによるbranch命令でゲームのコードに戻れるようにする
__end:
    popStack
    #最後に元の命令を配置(場合によっては最後にならないことも)
    defaultInstruction