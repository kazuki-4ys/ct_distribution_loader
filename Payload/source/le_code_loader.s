#include <ppc-asm.h>

#Original code was written by Leseratte
#https://mariokartwii.com/showthread.php?tid=1622

#Modified by kazuki to use custom REL with LE-CODE Distribution.

.set already_ran_flag, 0x80000CF0

.set region, 'P'

.if (region == 'P')
    .set DVDOpen, 0x8015e2bc
    .set DVDReadPrio, 0x8015e834
    .set DVDClose, 0x8015e568
    .set OSFatal, 0x801a4ec4
.elseif (region == 'E')
    .set DVDOpen, 0x8015e21c
    .set DVDReadPrio, 0x8015e794
    .set DVDClose, 0x8015e4c8
    .set OSFatal, 0x801a4e24
.elseif (region == 'J')
    .set DVDOpen, 0x8015e1dc
    .set DVDReadPrio, 0x8015e754
    .set DVDClose, 0x8015e488
    .set OSFatal, 0x801a4de4
.elseif (region == 'K')
    .set DVDOpen, 0x8015e334
    .set DVDReadPrio, 0x8015e8ac
    .set DVDClose, 0x8015e5e0
    .set OSFatal, 0x801a5220
.else
    .err
.endif

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

    lis r12, already_ran_flag@ha
    lbz r11, already_ran_flag@l(r12)
    cmpwi r11, 0x1
    beq- already_ran
    li r11, 0x1
    stb r11, already_ran_flag@l(r12)
    stwu r1, -0x128(r1)
    mflr r0
    stw r0, 0x12C(r1)
    stmw r23, 0x104(r1)
    bl le_code_bin_path
.if (region == 'P')
    .string "/rel/lecode-PAL.bin"
.elseif (region == 'E')
    .string "/rel/lecode-USA.bin"
.elseif (region == 'J')
    .string "/rel/lecode-JAP.bin"
.elseif (region == 'K')
    .string "/rel/lecode-KOR.bin"
.else
    .err
.endif
    .balign 4
    le_code_bin_path:
    addi r3, r1, 0x3C
    lis r4, 0xFFFF
    ori r4, r4, 0xFFE0
    and r27, r3, r4
    addi r28, r27, 0x60
    mflr r3 #r3 = pointer to /rel/lecode-XXX.bin
    mr r4, r27 #pointer to DVDFileInfo struct
    lis r5, DVDOpen@ha
    addi r5, r5, DVDOpen@l
    mtctr r5
    bctrl
    cmpwi r3, 0
    beq- lecode_load_fail
    #read first 0x20 byte to get file size and entry point
    mr r3, r27 #pointer to DVDFileInfo struct
    mr r4, r28
    li r5, 0x20
    li r6, 0
    li r7, 0x2
    lis r8, DVDReadPrio@ha
    addi r8, r8, DVDReadPrio@l
    mtctr r8
    bctrl
    lwz r26, 0x10(r28) #LE-CODE entry point
    mr r3, r27 #pointer to DVDFileInfo struct
    lwz r4, 0xC(r28) #LE-CODE base addr
    lwz r5, 0x14(r28)
    li r6, 0
    li r7, 0x2
    lis r8, DVDReadPrio@ha
    addi r8, r8, DVDReadPrio@l
    mtctr r8
    bctrl
    mr r3, r27
    lis r8, DVDClose@ha
    addi r8, r8, DVDClose@l
    mtctr r8
    bctrl 
    mr r3, r26
    cmpwi r3, 0 #if LE-CODE entry point is NULL, skip.
    beq- skip_le_code_entry_point
    mtctr r3
    bctrl #run LE-CODE entry point
    skip_le_code_entry_point:
    lmw r23, 0x104(r1)
    lwz r0, 0x12C(r1)
    mtlr r0
    addi r1, r1, 0x128
    blr
    lecode_load_fail:
    bl get_error_screen_string
    .string "Failed to load LE-CODE extension for Mario Kart.\n\nThat usually means the image is corrupted and/or is\nmissing required files. Try rebuilding the distribution.\nIf that doesn't help, please contact Wiimm or Leseratte.\n"
    .balign 4
    get_error_screen_string:
    mflr r5
    bl get_error_screen_color
    .long 0xE0E0E0FF #string color
    .long 0x000030FF #bg color
    get_error_screen_color:
    mflr r3
    addi r4, r3, 0x4
    lis r8, OSFatal@ha
    addi r8, r8, OSFatal@l
    mtlr r8
    blr
    already_ran:
get_le_code_loader_hook_end_b:
    bl blTrickCommonEnd