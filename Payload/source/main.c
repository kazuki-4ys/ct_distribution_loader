#include "common.h"
#include "kamek_loader.h"

void *getSystemHeap(void);
void ICInvalidateRangeAsm(void*, unsigned int);
const char *getString0(void);
unsigned char *getString1(void);
const char *getString2(void);
void *getString3(void);
void *getString4(void);
void *getString5(void);

void *getPulsar1xLoaderEntryAsmPtr(void);

void u32ToBytes(unsigned char *mem, unsigned int val){
    *mem = (val >> 24);
    *(mem + 1) = ((val >> 16) & 0xFF);
    *(mem + 2) = ((val >> 8) & 0xFF);
    *(mem + 3) = (val & 0xFF);
}

void ICInvalidateRange(void *_start, unsigned int length){
    //CPUのキャッシュメモリを更新し、過去にキャッシュされたコードの実行を防ぐ？
    //_start とlengthを0x20でアラインメント alignment for 0x20
    unsigned int start = (unsigned int)_start;
    unsigned int end = start + length;
    if(end & 0x1F){
        end = ((end >> 5) + 1) << 5;
    }
    if(start & 0x1F){
        start = (start >> 5) << 5;
    }
    ICInvalidateRangeAsm((void*)start, end - start);
}

void *my_malloc(unsigned int length){
    void* (*Egg__Heap__Alloc)(unsigned int, unsigned int, void*) = (void*)EGG_HEAP_ALLOC;
    unsigned int requsetLength = length;
    if(requsetLength & 0x1F){//0x20でアラインメント alignment for 0x20
        requsetLength = ((requsetLength >> 5) + 1) << 5;
    }
    return Egg__Heap__Alloc(requsetLength, 0x20, getSystemHeap());
}

BOOL isDvdFileExsist(const char *path){
    int (*DVDFastOpen)(int, DVDFileInfo*) = (void*)DVD_FAST_OPEN;
    int (*DVDConvertPathToEntryNum)(const char*) = (void*)DVD_CONVERT_PATH_TO_ENTRY_NUM;
    void (*DVDClose)(DVDFileInfo*) = (void*)DVD_CLOSE;
    DVDFileInfo fi;
    int result = DVDFastOpen(DVDConvertPathToEntryNum(path), &fi);
    if(!result)return false;
    unsigned int fileSize = fi.length;
    DVDClose(&fi);
    if(fileSize)return true;
    return false;
}

void allocMyGlobalVar(void){
    void *p = my_malloc(sizeof(myGlobalVar));
    void **tmp = (void**)((void*)0x80005930);
    *tmp = p;
}

myGlobalVar *getMyGlobalVar(void){
    myGlobalVar **ptr = (myGlobalVar**)((void*)0x80005930);
    return *ptr;
}

unsigned int makeBranchInstructionByAddrDelta(int addrDelta){//アドレス差分からbranch命令作成
    unsigned int instruction = 0;
    if(addrDelta < 0){
        instruction = addrDelta + 0x4000000;
    }else{
        instruction = addrDelta;
    }
    instruction |= 0x48000000;
    return instruction;
}

void injectBranch(void *target, void *src){
    //srcからtargetへジャンプ
    //branch to src from target
    unsigned int instruction = makeBranchInstructionByAddrDelta((int)target - (int)src);
    u32ToBytes((void*)src, instruction);
    ICInvalidateRange((void*)src, 4);
}

void* searchForOcarinaPatch(void *start, void *value, unsigned int valueLength){
    int (*memcmp)(void*, void*, unsigned int) = (void*)MEMCMP;
    unsigned int curAddr = (unsigned int)start;
    while((valueLength + curAddr) < 0x81800001){
        if(!memcmp((void*)curAddr, value, valueLength))return (void*)curAddr;
        curAddr += 4;
    }
    return (void*)0xFFFFFFFF;//not found
}

void ocarinaPatch(void *offset, void *value, unsigned int valueLength){
    //unsigned char blrInstruction[4] = {0x4E, 0x80, 0, 0x20};
    unsigned char *blrInstruction = getString1();
    void *searchResult = searchForOcarinaPatch((void*)0x80000000, value, valueLength);
    if(searchResult == ((void*)0xFFFFFFFF))return;
    searchResult = searchForOcarinaPatch(searchResult, blrInstruction, 4);
    if(searchResult == ((void*)0xFFFFFFFF))return;
    injectBranch(offset, searchResult);
}

void installGeckoCodeHandler(void){
    int (*DVDFastOpen)(int, DVDFileInfo*) = (void*)DVD_FAST_OPEN;
    myGlobalVar *g = getMyGlobalVar();
    DVDFileInfo fi;
	//int result = DVDFastOpen(DVDConvertPathToEntryNum("/codes/RMCJ01.gct"), &fi);
    int result = DVDFastOpen(DVDConvertPathToEntryNum(getString2()), &fi);
    if(!result)return;
    g->gctFile = my_malloc(fi.length);
    DVDReadPrio(&fi, g->gctFile, fi.length, 0, 2);
    ICInvalidateRange(g->gctFile, fi.length);
    DVDClose(&fi);

    memcpy((void*)0x80001800, getString3(), 0xB38);//cppy modified codehandler.bin to 0x80001800
    ICInvalidateRange((void*)0x80001800, 0xB38);
    //unsigned char viHookValue[16] = {0x7C, 0xE3, 0x3B, 0x78, 0x38, 0x87, 0x00, 0x34, 0x38, 0xA7, 0x00, 0x38, 0x38, 0xC7, 0x00, 0x4C};
    ocarinaPatch((void*)0x800018A8, getString4(), 16);//vi hook 
}

void installPulsar1xLoader(void){
    injectBranch(getPulsar1xLoaderEntryAsmPtr(), (void*)PULSAR_LOADER_REL_INJECT);//RMCJ ONLY!!!!!
    pulsar1xLoaderEntry();
    return;
}

void __main(void){
    void (*OSReport)(const char*, ...) = (void*)OSREPORT;
    int (*DVDConvertPathToEntryNum)(const char*) = (void*)DVD_CONVERT_PATH_TO_ENTRY_NUM;
    int (*DVDFastOpen)(int, DVDFileInfo*) = (void*)DVD_FAST_OPEN;
    int (*DVDReadPrio)(DVDFileInfo*, void*, unsigned int, unsigned int, unsigned int) = (void*)DVD_READ_PRIO;
    void (*DVDClose)(DVDFileInfo*) = (void*)DVD_CLOSE;
    void (*memcpy)(void*, void*, unsigned int) = (void*)MEMCPY;
    OSReport(getString0());
    allocMyGlobalVar();
    myGlobalVar *g = getMyGlobalVar();
    g->codePulBuf = nullptr;
    g->loadKamekBinaryFromDiscFileLength = 0;
    g->loadKamekBinarytext = 0;
    if(isDvdFileExsist(getString7()))installPulsar1xLoader();
    if(isDvdFileExsist(getString2()))installPulsar1xLoader();
}