#include "kamek_loader.h"

const char* getString7(void);
const char* getString8(void);

void *heapAlloc(void *heap, unsigned int size, int unk){
    #ifdef RMCP
    void *funcPtr = (void*)0x80226c04;
    #endif 
    #ifdef RMCE
    void *funcPtr = (void*)0x80226880;
    #endif
    #ifdef RMCJ
    void *funcPtr = (void*)0x80226b24;
    #endif
    #ifdef RMCK
    void *funcPtr = (void*)0x80226f78;
    #endif
    void* (*heapAllocFunc)(void *heap, unsigned int, int) = funcPtr;
    return heapAllocFunc(heap, size, unk);
}

void *heapFree(void *heap, void *ptr){
    #ifdef RMCP
    void *funcPtr = (void*)0x80226c78;
    #endif 
    #ifdef RMCE
    void *funcPtr = (void*)0x802268f4;
    #endif
    #ifdef RMCJ
    void *funcPtr = (void*)0x80226b98;
    #endif
    #ifdef RMCK
    void *funcPtr = (void*)0x80226fec;
    #endif
    void* (*heapFreeFunc)(void *heap, void *ptr) = funcPtr;
    return heapFreeFunc(heap, ptr);
}

typedef struct {
    u32 magic1; //0x0
    u16 magic2; //0x2
    u16 version; //0x4
    u32 bssSize; //0x8
    u32 codeSize; //0xc
    u32 ctorStart; //0x10
    u32 ctorEnd; //0x14
    u32 length; //0x18
    u32 padding; //0x1c
}KBHeader;

typedef void (*Func)();

#define kAddr32 1
#define kAddr16Lo 4
#define kAddr16Hi 5
#define kAddr16Ha 6
#define kRel24 10
#define kWrite32 32
#define kWrite16 33
#define kWrite8 34
#define kCondWritePointer 35
#define kCondWrite32 36
#define kCondWrite16 37
#define kCondWrite8 38
#define kBranch 64
#define kBranchLink 65


void DisplayError(const LoaderParams* funcs, const char* str) {
    u32 fg = 0xFFFFFFFF, bg = 0;
    funcs->OSFatal(&fg, &bg, str);
}


u32 resolveAddress(u32 text, u32 address) {
    if(address & 0x80000000)
        return address;
    else
        return text + address;
}


#define kCommandHandler(name) \
	const u8 *kHandle##name(const u8 *input, u32 text, u32 address)
#define kDispatchCommand(name) \
	case k##name: input = kHandle##name(input, text, address); break

kCommandHandler(Addr32) {
    u32 target = resolveAddress(text, *(const u32*)input);
    *(u32*)address = target;
    return input + 4;
}
kCommandHandler(Addr16Lo) {
    u32 target = resolveAddress(text, *(const u32*)input);
    *(u16*)address = target & 0xFFFF;
    return input + 4;
}
kCommandHandler(Addr16Hi) {
    u32 target = resolveAddress(text, *(const u32*)input);
    *(u16*)address = target >> 16;
    return input + 4;
}
kCommandHandler(Addr16Ha) {
    u32 target = resolveAddress(text, *(const u32*)input);
    *(u16*)address = target >> 16;
    if(target & 0x8000)
        *(u16*)address += 1;
    return input + 4;
}
kCommandHandler(Rel24) {
    u32 target = resolveAddress(text, *(const u32*)input);
    u32 delta = target - address;
    *(u32*)address &= 0xFC000003;
    *(u32*)address |= (delta & 0x3FFFFFC);
    return input + 4;
}
kCommandHandler(Write32) {
    u32 value = *(const u32*)input;
    *(u32*)address = value;
    return input + 4;
}
kCommandHandler(Write16) {
    u32 value = *(const u32*)input;
    *(u16*)address = value & 0xFFFF;
    return input + 4;
}
kCommandHandler(Write8) {
    u32 value = *(const u32*)input;
    *(u8*)address = value & 0xFF;
    return input + 4;
}
kCommandHandler(CondWritePointer) {
    u32 target = resolveAddress(text, *(const u32*)input);
    u32 original = ((const u32*)input)[1];
    if(*(u32*)address == original)
        *(u32*)address = target;
    return input + 8;
}
kCommandHandler(CondWrite32) {
    u32 value = *(const u32*)input;
    u32 original = ((const u32*)input)[1];
    if(*(u32*)address == original)
        *(u32*)address = value;
    return input + 8;
}
kCommandHandler(CondWrite16) {
    u32 value = *(const u32*)input;
    u32 original = ((const u32*)input)[1];
    if(*(u16*)address == (original & 0xFFFF))
        *(u16*)address = value & 0xFFFF;
    return input + 8;
}
kCommandHandler(CondWrite8) {
    u32 value = *(const u32*)input;
    u32 original = ((const u32*)input)[1];
    if(*(u8*)address == (original & 0xFF))
        *(u8*)address = value & 0xFF;
    return input + 8;
}
kCommandHandler(Branch) {
    *(u32*)address = 0x48000000;
    return kHandleRel24(input, text, address);
}
kCommandHandler(BranchLink) {
    *(u32*)address = 0x48000001;
    return kHandleRel24(input, text, address);
}


void cacheInvalidateAddress(u32 address);
void syncAndIsync(void);

void LoadKamekBinary(LoaderParams* funcs, const void* binary, u32 binaryLength, BOOL isDol) {

    //static u32 text = 0;
    myGlobalVar *g = getMyGlobalVar();
    const KBHeader* header = (const KBHeader*)binary;
    //if(header->magic1 != 'Kame' || header->magic2 != 'k\0')
        //DisplayError(funcs, "FATAL ERROR: Corrupted file, please check your game's Kamek files");
        //return;
    if(header->version != 3) {
        char err[512];
        //funcs->sprintf(err, "FATAL ERROR: Incompatible file (version %d), please upgrade your Kamek Loader", header->version);
        //DisplayError(funcs, err);
        return;
    }

    /*funcs->OSReport("header: bssSize=%u, codeSize=%u, ctors=%u-%u\n",
        header->bssSize, header->codeSize, header->ctorStart, header->ctorEnd);*/

    u32 textSize = header->codeSize + header->bssSize;

    void* heap = funcs->rkSystem->EGGSystem;
    //if(isDol) g->loadKamekBinarytext = (u32)heap->alloc(textSize, 0x20);
    if(isDol) g->loadKamekBinarytext = (u32)heapAlloc(heap, textSize, 0x20);
    if(!g->loadKamekBinarytext){
        //DisplayError(funcs, "FATAL ERROR: Out of code memory");
        return;
    }

    const u8* input = ((const u8*)binary) + sizeof(KBHeader);
    const u8* inputEnd = ((const u8*)binary) + binaryLength;
    u8* output = (u8*)g->loadKamekBinarytext;

    if(isDol) {
        // Create text + bss sections
        for(u32 i = 0; i < header->codeSize; ++i) {
            *output = *(input++);
            cacheInvalidateAddress((u32)(output++));
        }
        for(u32 i = 0; i < header->bssSize; ++i) {
            *output = 0;
            cacheInvalidateAddress((u32)(output++));
        }
    }

    while(input < inputEnd) {
        u32 cmdHeader = *((u32*)input);
        input += 4;

        u8 cmd = cmdHeader >> 24;
        u32 address = cmdHeader & 0xFFFFFF;
        if(address == 0xFFFFFE) {
            // Absolute address
            address = *((u32*)input);
            if(address < 0x80510238 && !isDol) continue;
            else if(address >= 0x80510238 && isDol) continue;
            input += 4;
        }
        else {
            if(!isDol) continue;
            // Relative address
            address += g->loadKamekBinarytext;
        }

        switch(cmd) {
            case kAddr32:
                input = kHandleAddr32(input, g->loadKamekBinarytext, address);
                break;
            case kAddr16Lo:
                input = kHandleAddr16Lo(input, g->loadKamekBinarytext, address);
                break;
            case kAddr16Hi:
                input = kHandleAddr16Hi(input, g->loadKamekBinarytext, address);
                break;
            case kAddr16Ha:
                input = kHandleAddr16Ha(input, g->loadKamekBinarytext, address);
                break;
            case kRel24:
                input = kHandleRel24(input, g->loadKamekBinarytext, address);
                break;
            case kWrite32:
                input = kHandleWrite32(input, g->loadKamekBinarytext, address);
                break;
            case kWrite16:
                input = kHandleWrite16(input, g->loadKamekBinarytext, address);
                break;
            case kWrite8:
                input = kHandleWrite8(input, g->loadKamekBinarytext, address);
                break;
            case kCondWrite32:
                input = kHandleCondWrite32(input, g->loadKamekBinarytext, address);
                break;
            case kCondWrite16:
                input = kHandleCondWrite16(input, g->loadKamekBinarytext, address);
                break;
            case kCondWrite8:
                input = kHandleCondWrite8(input, g->loadKamekBinarytext, address);
                break;
            case kCondWritePointer:
                input = kHandleCondWritePointer(input, g->loadKamekBinarytext, address);
                break;
            case kBranch:
                input = kHandleBranch(input, g->loadKamekBinarytext, address);
                break;
            case kBranchLink:
                input = kHandleBranchLink(input, g->loadKamekBinarytext, address);
                break;
            default:
                //funcs->OSReport("Unknown command: %d\n", cmd);
                break;
        }

        cacheInvalidateAddress(address);
    }
    syncAndIsync();

    if(!isDol) {
        for(Func* f = (Func*)(g->loadKamekBinarytext + header->ctorStart); f < (Func*)(g->loadKamekBinarytext + header->ctorEnd); f++) {
            (*f)();
        }
    }
}

unsigned int RoundUp(unsigned int src){
    if(src & 0x1F){
        src &= (~0x1F);
        src += 0x20;
    }
    return src;
}


void LoadKamekBinaryFromDisc(LoaderParams* params)
{
    myGlobalVar *g = getMyGlobalVar();
    //static void* codePulBuf = nullptr;
    //static u32 fileLength = 0;
    //params->OSReport("{Kamek by Treeki}\nLoading Kamek binary");
    
    BOOL isDol = false;
    void* heap = params->rkSystem->EGGSystem;

    if(g->codePulBuf == nullptr) {
        const char* path = getString7();
        int entrynum = params->DVDConvertPathToEntrynum(path);
        if(entrynum < 0) {
            char err[512];
            //params->sprintf(err, "FATAL ERROR: Failed to locate file on the disc: %s", path);
            //DisplayError(params, err);
            return;
        }

        DVDFileInfo fileInfo;
        if(!params->DVDFastOpen(entrynum, &fileInfo)){
            //DisplayError(params, "FATAL ERROR: Failed to open file!");
            return;
        }
        //params->OSReport("DVD file located: addr=%p, size=%d\n", fileInfo.startAddr, fileInfo.length);

        //alignas(0x20) KBHeader header;
        KBHeader *header = (KBHeader*)my_malloc(sizeof(KBHeader));
        u32 roundedHeaderLength = RoundUp(sizeof(KBHeader));
        params->DVDReadPrio(&fileInfo, header, roundedHeaderLength, 0, 2);

        g->loadKamekBinaryFromDiscFileLength = header->length;
        u32 length = header->length;
        u32 roundedLength = RoundUp(length);

        isDol = true;
        //g->codePulBuf = heap->alloc(roundedLength, -0x20);
        g->codePulBuf = heapAlloc(heap, roundedLength, -0x20);
        if(!g->codePulBuf){
            //DisplayError(params, "FATAL ERROR: Out of file memory");
            DisplayError(params, getString8());
            return;
        }
        params->DVDReadPrio(&fileInfo, g->codePulBuf, roundedLength, length * params->region, 2);
        params->DVDClose(&fileInfo);
    }

    LoadKamekBinary(params, g->codePulBuf, g->loadKamekBinaryFromDiscFileLength, isDol);
    if(!isDol) heapFree(heap, g->codePulBuf);
}

void pulsar1xLoaderEntry(void){
    LoaderParams params;
    params.OSFatal = (void*)OSFATAL;
    params.OSReport = (void*)OSREPORT;
    params.DVDConvertPathToEntrynum = (void*)DVD_CONVERT_PATH_TO_ENTRY_NUM;
    params.DVDFastOpen = (void*)DVD_FAST_OPEN;
    params.DVDReadPrio = (void*)DVD_READ_PRIO;
    params.DVDClose = (void*)DVD_CLOSE;
    #ifdef RMCP
    params.rkSystem = (void*)0x802A4080;
    params.region = PAL;
    #endif
    #ifdef RMCE
    params.rkSystem = (void*)0x8029fd00;
    params.region = NTSC_U;
    #endif
    #ifdef RMCJ
    params.rkSystem = (void*)0x802a3a00;
    params.region = NTSC_J;
    #endif
    LoadKamekBinaryFromDisc(&params);
}