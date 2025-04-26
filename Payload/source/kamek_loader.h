#ifndef _KAMEK_LOADER_H_
#define _KAMEK_LOADER_H_

#include "common.h"

void pulsar1xLoaderEntry(void);

typedef void (*OSReport_t) (const char* str, ...);
typedef void (*OSFatal_t) (u32* fg, u32* bg, const char* str, ...);
typedef int (*DVDConvertPathToEntrynum_t) (const char* path);
typedef BOOL (*DVDFastOpen_t) (int entrynum, DVDFileInfo* fileInfo);
typedef int (*DVDReadPrio_t) (DVDFileInfo* fileInfo, void* buffer, int length, int offset, int unk);
typedef BOOL (*DVDClose_t) (DVDFileInfo* fileInfo);
typedef int (*sprintf_t) (char* str, const char* format, ...);

typedef enum {
    PAL = 0,
    NTSC_U = 1,
    NTSC_J = 2,
    NTSC_K = 3
}Region;

typedef struct {
    OSReport_t OSReport;
    OSFatal_t OSFatal;
    DVDConvertPathToEntrynum_t DVDConvertPathToEntrynum;
    DVDFastOpen_t DVDFastOpen;
    DVDReadPrio_t DVDReadPrio;
    DVDClose_t DVDClose;
    sprintf_t sprintf;
    RKSystem* rkSystem;
    Region region;
}LoaderParams;

#endif//_KAMEK_LOADER_H_