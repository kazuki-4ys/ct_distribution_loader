#ifdef RMCJ

#define OSREPORT 0x801A24F0
#define EGG_HEAP_ALLOC 0x80229734
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015DE6C
#define DVD_FAST_OPEN 0x8015E174
#define DVD_READ_PRIO 0x8015E754
#define DVD_CLOSE 0x8015E488
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000F238
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6C50

#endif
#ifdef RMCE

#define OSREPORT 0x801a2530
#define EGG_HEAP_ALLOC 0x80229490
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015deac
#define DVD_FAST_OPEN 0x8015e1b4
#define DVD_READ_PRIO 0x8015e794
#define DVD_CLOSE 0x8015e4c8
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000e7b4
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6C90

#endif
#ifdef RMCP

#define OSREPORT 0x801a25d0
#define EGG_HEAP_ALLOC 0x80229814
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015df4c
#define DVD_FAST_OPEN 0x8015e254
#define DVD_READ_PRIO 0x8015e834
#define DVD_CLOSE 0x8015e568
#define MEMCPY 0x80005F34
#define MEMCMP 0x8000f314
#define LE_CODE_LOADER_INJECT_ADDR 0x801A6D30

#endif

typedef struct{
    unsigned char unk0[0x34];
    unsigned int length;
    //0x38
    unsigned char unk1[4];
    //全部で0x3Cバイト
}DVDFileInfo;

void *getSystemHeap(void);
void ICInvalidateRangeAsm(void*, unsigned int);
const char *getString0(void);
unsigned char *getString1(void);
const char *getString2(void);
void *getString3(void);
void *getString4(void);
void *getString5(void);
void *get_le_code_loader_hook(void);
void *get_le_code_loader_hook_end(void);

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

void injectC2Patch(void *targetAddr, void *codeStart, void *codeEnd){
    //inject code like C2 code type
    u32ToBytes((unsigned char*)codeEnd - 4, makeBranchInstructionByAddrDelta((unsigned int)targetAddr + 4 - ((unsigned int)codeEnd - 4)));
    u32ToBytes(targetAddr, makeBranchInstructionByAddrDelta(codeStart - targetAddr));
    ICInvalidateRange((void*)((unsigned int)codeEnd - 4), 4);
    ICInvalidateRange(targetAddr, 4);
}

void installLeCodeLoader(void){
    int (*DVDConvertPathToEntryNum)(const char*) = (void*)DVD_CONVERT_PATH_TO_ENTRY_NUM;
    if(DVDConvertPathToEntryNum(getString5()) < 0)return;
    injectC2Patch((void*)LE_CODE_LOADER_INJECT_ADDR, get_le_code_loader_hook(), get_le_code_loader_hook_end());
}

void __main(void){
    void (*OSReport)(const char*, ...) = (void*)OSREPORT;
    int (*DVDConvertPathToEntryNum)(const char*) = (void*)DVD_CONVERT_PATH_TO_ENTRY_NUM;
    int (*DVDFastOpen)(int, DVDFileInfo*) = (void*)DVD_FAST_OPEN;
    int (*DVDReadPrio)(DVDFileInfo*, void*, unsigned int, unsigned int, unsigned int) = (void*)DVD_READ_PRIO;
    void (*DVDClose)(DVDFileInfo*) = (void*)DVD_CLOSE;
    void (*memcpy)(void*, void*, unsigned int) = (void*)MEMCPY;
    OSReport(getString0());
    installLeCodeLoader();
    DVDFileInfo fi;
	//int result = DVDFastOpen(DVDConvertPathToEntryNum("/codes/RMCJ01.gct"), &fi);
    int result = DVDFastOpen(DVDConvertPathToEntryNum(getString2()), &fi);
    if(!result)return;
    
    void *gctFile = my_malloc(fi.length);
    unsigned int *gctFileAddr = (void*)0x800041F0;//改変したcodehandler.binの機能により、0x800041F0にgctへのポインタを書き込むとgctコードが適用される
    *gctFileAddr = (unsigned int)gctFile;
    DVDReadPrio(&fi, gctFile, fi.length, 0, 2);
    ICInvalidateRange(gctFile, fi.length);
    DVDClose(&fi);

    memcpy((void*)0x80001800, getString3(), 0xB40);//cppy modified codehandler.bin to 0x80001800
    ICInvalidateRange((void*)0x80001800, 0xB40);
    //unsigned char viHookValue[16] = {0x7C, 0xE3, 0x3B, 0x78, 0x38, 0x87, 0x00, 0x34, 0x38, 0xA7, 0x00, 0x38, 0x38, 0xC7, 0x00, 0x4C};
    ocarinaPatch((void*)0x800018A8, getString4(), 16);//vi hook 
}