#ifndef _PE_H
#define _PE_H

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

#define F_EXEC 0x0002   // 可执行标记
/* Coff Header */
typedef struct _FILEHDR{
	unsigned short usMagic;  		// 魔法数字
	unsigned short usNumSec;  		// 段落（Section）数
	unsigned long  ulTime;  		// 时间戳
	unsigned long  ulSymbolOffset;  	// 符号表偏移
	unsigned long  ulNumSymbol; 	 	// 符号数
	unsigned short usOptHdrSZ; 	 	// 可选头长度
	unsigned short usFlags;  		// 文件标记
} FILEHDR;

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
	t_8 MajorLinkerVersion;
	t_8 MinorLinkerVersion;
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

typedef struct __attribute__((packed)) HINT_NAME_TABLE{
	t_16	Hint;
	t_8	Name[];
}HINT_NAME_TABLE;

typedef struct IMPORT_DIRECTORY_TABLE{
	t_32 ImportLookupTable;
	t_32 TimeStamp;	
	t_32 FowarderChain;
	t_32 Name;
	t_32 ImportAddressTable;
}IMPORT_DIRECTORY_TABLE;

typedef struct EXPORT_DIRECTORY_TABLE{
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
}EXPORT_DIRECTORY_TABLE;

typedef struct EXPORT_ADDRESS_TABLE{
	t_32 Address;	//RVA
	t_32 Forwarder;	//RVA
}EXPORT_ADDRESS_TABLE;

typedef struct {
  char           cName[8];  	// 段名
  unsigned long  ulVSize;  	// 虚拟大小
  unsigned long  ulVAddr;  	// 虚拟地址
  unsigned long  ulSize;  	// 段长度
  unsigned long  ulSecOffset;  	// 段数据偏移
  unsigned long  ulRelOffset;  	// 段重定位表偏移
  unsigned long  ulLNOffset;  	// 行号表偏移
  unsigned short ulNumRel;  	// 重定位表长度
  unsigned short ulNumLN;  	// 行号表长度
  unsigned long  ulFlags;  	// 段标识
} SECHDR;

#define STYP_TEXT 0x0020   	//正文段标识，说明该段是代码。
#define STYP_DATA 0x0040   	//数据段标识，有些标识的段将用来保存已初始化数据。
#define STYP_BSS 0x0080   	//有这个标识段也是用来保存数据，不过这里的数据是未初始化数据。

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




#endif

