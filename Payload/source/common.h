#ifndef _COMMON_H_
#define _COMMON_H_

#define nullptr 0
#define NULL 0

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef unsigned int BOOL;

#define true 1
#define false 0

#ifdef RMCJ

#define OSREPORT 0x801A24F0
#define OSFATAL 0x801A4DE4
#define EGG_HEAP_ALLOC 0x80229734
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015DE6C
#define DVD_FAST_OPEN 0x8015E174
#define DVD_READ_PRIO 0x8015E754
#define DVD_CLOSE 0x8015E488
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000F238
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6C50
#define CT_CODE_LOADER_INJECT_ADDR 0x8004bfc8
#define CT_CODE_TEXTURE_PATH 0x80244EA8
#define PULSAR_LOADER_REL_INJECT 0x8000A350

#endif
#ifdef RMCE

#define OSREPORT 0x801a2530
#define OSFATAL 0x801a4e24
#define EGG_HEAP_ALLOC 0x80229490
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015deac
#define DVD_FAST_OPEN 0x8015e1b4
#define DVD_READ_PRIO 0x8015e794
#define DVD_CLOSE 0x8015e4c8
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000e7b4
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6C90
#define CT_CODE_LOADER_INJECT_ADDR 0x8004c008
#define CT_CODE_TEXTURE_PATH 0x80244F08
#define PULSAR_LOADER_REL_INJECT 0x8000a3b4

#endif
#ifdef RMCP

#define OSREPORT 0x801a4ec4
#define OSFATAL 0x801a4e24
#define EGG_HEAP_ALLOC 0x80229814
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015df4c
#define DVD_FAST_OPEN 0x8015e254
#define DVD_READ_PRIO 0x8015e834
#define DVD_CLOSE 0x8015e568
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000f314
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6D30
#define CT_CODE_LOADER_INJECT_ADDR 0x8004c0a8
#define CT_CODE_TEXTURE_PATH 0x80244F88
#define PULSAR_LOADER_REL_INJECT 0x8000a3f4

#endif

typedef struct{
    unsigned char unk0[0x34];
    unsigned int length;
    //0x38
    unsigned char unk1[4];
    //全部で0x3Cバイト
}DVDFileInfo;

typedef struct{
    void *vtable;
    void* MEM1ArenaLo;
    void* MEM1ArenaHi;
    void* MEM2ArenaLo;
    void* MEM2ArenaHi;
    u32 memorySize; //0x14
    void* EGGRootMEM1; //0x18
    void* EGGRootMEM2;  //0x1C
    void* EGGRootDebug; //0x20
    void* EGGSystem;  //0x24
    void* heapSystem; //thread
    u32 unknown_0x2C; //just the start of mem1?
    u32 unknown_0x30; //idk
    u32 sysHeapSize;
    u32 gxFifoBufSize;
    void* mode;
    void* audioManager; //0x40
    void* video; //0x44
    void* xfbManager; //0x48
    void* asyncDisplay; //0x4c
    void* processMeter; //0x50
    void* sceneManager; //0x54 //actually a RKSceneManager
    void* kpadWorkHeap; //0x58
    void* wpadAllocator; //0x5c
    void* relLinkHeap; //0x60
    u8 unknown_0x64[0x74 - 0x64];
}RKSystem;

typedef struct{
    void *gctFile;
    void *codePulBuf;
    u32 loadKamekBinaryFromDiscFileLength;
    u32 loadKamekBinarytext;
}myGlobalVar;

myGlobalVar *getMyGlobalVar(void);

void *my_malloc(unsigned int length);

#endif//_COMMON_H_