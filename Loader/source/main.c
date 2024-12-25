#ifdef RMCJ

#define ALREADY_LAUNCHED_FLAG 0x800001B8
#define EGG_HEAP_ALLOC 0x80229734
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015DE6C
#define DVD_FAST_OPEN 0x8015E174
#define DVD_READ_PRIO 0x8015E754
#define DVD_CLOSE 0x8015E488

#endif
#ifdef RMCE

#define ALREADY_LAUNCHED_FLAG 0x800001B8
#define EGG_HEAP_ALLOC 0x80229490
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015deac
#define DVD_FAST_OPEN 0x8015e1b4
#define DVD_READ_PRIO 0x8015e794
#define DVD_CLOSE 0x8015e4c8

#endif
#ifdef RMCP

#define ALREADY_LAUNCHED_FLAG 0x800001B8
#define EGG_HEAP_ALLOC 0x80229814
#define DVD_CONVERT_PATH_TO_ENTRY_NUM 0x8015df4c
#define DVD_FAST_OPEN 0x8015e254
#define DVD_READ_PRIO 0x8015e834
#define DVD_CLOSE 0x8015e568

#endif

typedef struct{
    unsigned char unk0[0x34];
    unsigned int length;
    //0x38
    unsigned char unk1[4];
    //全部で0x3Cバイト
} DVDFileInfo;

const char *getString0(void);
void *getSystemHeap(void);
//void ICInvalidateRangeAsm(void*, unsigned int);

/*void ICInvalidateRange(void *_start, unsigned int length){
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
}*/

void *my_malloc(unsigned int length){
    void* (*Egg__Heap__Alloc)(unsigned int, unsigned int, void*) = (void*)EGG_HEAP_ALLOC;
    unsigned int requsetLength = length;
    if(requsetLength & 0x1F){//0x20でアラインメント alignment for 0x20
        requsetLength = ((requsetLength >> 5) + 1) << 5;
    }
    return Egg__Heap__Alloc(requsetLength, 0x20, getSystemHeap());
}

void __main(void){
    void (*systemBinEntry)(void);
    int (*DVDConvertPathToEntryNum)(const char*) = (void*)DVD_CONVERT_PATH_TO_ENTRY_NUM;
    int (*DVDFastOpen)(int, DVDFileInfo*) = (void*)DVD_FAST_OPEN;
    int (*DVDReadPrio)(DVDFileInfo*, void*, unsigned int, unsigned int, unsigned int) = (void*)DVD_READ_PRIO;
    void (*DVDClose)(DVDFileInfo*) = (void*)DVD_CLOSE;
	unsigned char *alreadyLaunchedFlag = (void*)ALREADY_LAUNCHED_FLAG;
	if(*alreadyLaunchedFlag)return;
	*alreadyLaunchedFlag = 1;
	DVDFileInfo fi;
	int result = DVDFastOpen(DVDConvertPathToEntryNum(getString0()), &fi);
    if(!result)return;
    void *systemBin = my_malloc(fi.length);
    DVDReadPrio(&fi, systemBin, fi.length, 0, 2);
    DVDClose(&fi);
    systemBinEntry = systemBin;
    systemBinEntry();
}