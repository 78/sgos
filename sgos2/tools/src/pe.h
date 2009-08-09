#ifndef _PE_H_
#define _PE_H_

typedef unsigned short t_16;
typedef unsigned int t_32;

/* Dos Header */
#define IMAGE_DOS_SIGNATURE 0x5A4D
typedef struct _IMAGE_DOS_HEADER {
	t_16 e_magic;
	t_16 e_cblp;
	t_16 e_cp;
	t_16 e_crlc;
	t_16 e_cparhdr;
	t_16 e_minalloc;
	t_16 e_maxalloc;
	t_16 e_ss;
	t_16 e_sp;
	t_16 e_csum;
	t_16 e_ip;
	t_16 e_cs;
	t_16 e_lfarlc;
	t_16 e_ovno;
	t_16 e_res[4];
	t_16 e_oemid;
	t_16 e_oeminfo;
	t_16 e_res2[10];
	t_32 e_lfanew;
} IMAGE_DOS_HEADER;

/* Coff Header */
typedef struct _FILEHDR{
	t_16 usMagic;  
	t_16 usNumSec; 
	t_32 ulTime; 
	t_32 ulSymbolOffset;  
	t_32 ulNumSymbol;
	t_16 usOptHdrSZ; 
	t_16 usFlags; 
} FILEHDR;


#define F_RELFLG 0x0001   // 重定位
#define F_EXEC 0x0002   // 可执行标记
#define F_LNNO 0x0004   // 文件中所有行号已经被去掉
#define F_LSYMS 0x0008   // 文件中的符号信息已经被去掉

#define IS_OBJ( flag ) (!(flag&(F_EXEC|F_LSYMS)))
#define IS_EXE( flag ) ( flag&F_EXEC )

//Data Directory
#define NUM_OF_IMAGE_DATA_DIRECTORY	10
typedef struct _IMAGE_DATA_DIRECTORY {
	t_32 RVA;
	t_32 Size;
} IMAGE_DATA_DIRECTORY;

//可选头部
typedef struct _IMAGE_OPTIONAL_HEADER {
	// 标准域
	t_16 Magic;
	unsigned char MajorLinkerVersion;
	unsigned char MinorLinkerVersion;
	t_32 SizeOfCode;
	t_32 SizeOfInitializedData;
	t_32 SizeOfUninitializedData;
	t_32 AddressOfEntryPoint;
	t_32 BaseOfCode;
	t_32 BaseOfData;
	// NT附加域
	t_32 ImageBase;
	t_32 SectionAlignment;
	t_32 FileAlignment;
	t_16 MajorOperatingSystemVersion;
	t_16 MinorOperatingSystemVersion;
	t_16 MajorImageVersion;
	t_16 MinorImageVersion;
	t_16 MajorSubsystemVersion;
	t_16 MinorSubsystemVersion;
	t_32 Reserved1;
	t_32 SizeOfImage;
	t_32 SizeOfHeaders;
	t_32 CheckSum;
	t_16 Subsystem;
	t_16 DllCharacteristics;
	t_32 SizeOfStackReserve;
	t_32 SizeOfStackCommit;
	t_32 SizeOfHeapReserve;
	t_32 SizeOfHeapCommit;
	t_32 LoaderFlags;
	t_32 NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY	DataDirectory[NUM_OF_IMAGE_DATA_DIRECTORY];
} IMAGE_OPTIONAL_HEADER;

#define IMAGE_DIRECTORY_ENTRY_EXPORT	0
#define IMAGE_DIRECTORY_ENTRY_IMPORT	1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE	2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION	3
#define IMAGE_DIRECTORY_ENTRY_SECURITY	4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC	5
#define IMAGE_DIRECTORY_ENTRY_DEBUG	6
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT	7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR	8
#define IMAGE_DIRECTORY_ENTRY_TLS	9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG	10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT	11
#define IMAGE_DIRECTORY_ENTRY_IAT	12

typedef struct __attribute__((packed)) HINT_NAME_TABLE{
	t_16	Hint;
	char	Name[];
}HINT_NAME_TABLE;

//Import dir entry
typedef struct IMPORT_DIR_TABLE{
	t_32 ImportLookupTable;
	t_32 TimeStamp;
	t_32 FowarderChain;
	t_32 Name;
	t_32 ImportAddressTable;
}IMPORT_DIR_TABLE;
//Export dir entry
typedef struct EXPORT_DIR_TABLE{
	t_32 ExportFlags;
	t_32 TimeStamp;
	t_16 MajorVersion;
	t_16 MinorVersion;
	t_32 Name;	
	t_32 OrdinalBase;
	t_32 AddressTableEntries;
	t_32 NumberOfNamePointers;	
	t_32 ExportAddressTable;
	t_32 NamePointerTable;	
	t_32 OrdinalTable;
}EXPORT_DIR_TABLE;

typedef struct EXPORT_ADDR_TABLE{
	t_32 Address;	//RVA
	t_32 Forwarder;	//RVA
}EXPORT_ADDR_TABLE;

typedef struct {
	char  cName[8];  // 段名
	t_32  ulVSize;  // 虚拟大小
	t_32  ulVAddr;  // 虚拟地址
	t_32  ulSize;  // 段长度
	t_32  ulSecOffset;  // 段数据偏移
	t_32  ulRelOffset;  // 段重定位表偏移
	t_32  ulLNOffset;  // 行号表偏移
	t_16  ulNumRel;  // 重定位表长度
	t_16  ulNumLN;  // 行号表长度
	t_32  ulFlags;  // 段标识
} SECHDR;

#define STYP_TEXT 0x0020   //正文段标识，说明该段是代码。
#define STYP_DATA 0x0040   //数据段标识，有些标识的段将用来保存已初始化数据。
#define STYP_BSS 0x0080   //有这个标识段也是用来保存数据，不过这里的数据是未初始化数据。

typedef struct __attribute__((packed)){
  unsigned long  VirtualAddress;  //
  unsigned long  SizeOfBlock;  //
  unsigned short TypeOffset;  // 
} IMAGE_BASE_RELOCATION;
#define IMAGE_REL_BASED_ABSOLUTE 0   //
#define IMAGE_REL_BASED_HIGHLOW 3   //

#endif //PE
