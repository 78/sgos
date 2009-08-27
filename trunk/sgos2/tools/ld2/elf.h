//定义ELF中的数据类型

typedef unsigned long	Elf32_Addr;
typedef unsigned char	Elf32_Char;
typedef unsigned short	Elf32_Half;
typedef unsigned long	Elf32_Off;
typedef long			Elf32_Sword;
typedef unsigned long	Elf32_Word;

typedef struct
{
	Elf32_Word	ei_mag;		// 文件标识
	Elf32_Char	ei_class;	// 文件类
	Elf32_Char	ei_data;	// 数据编码
	Elf32_Char	ei_version;	// 文件版本
	Elf32_Char	ei_pad;		// 补齐字节开始处
	Elf32_Char	ei_res[8];	// 保留
}Elf32_Ident;

typedef struct
{
	Elf32_Ident	e_ident;		// 目标文件标识
	Elf32_Half	e_type;			// 目标文件类型
	Elf32_Half	e_machine;		// 文件的目标体系结构类型
	Elf32_Word	e_version;		// 目标文件版本
	Elf32_Addr	e_entry;		// 程序入口的虚拟地址
	Elf32_Off	e_phoff;		// 程序头部表格的偏移量
	Elf32_Off	e_shoff;		// 节区头部表格的偏移量
	Elf32_Word	e_flags;		// 保存与文件相关的特定于处理器的标志
	Elf32_Half	e_ehsize;		// ELF头部的大小
	Elf32_Half	e_phentsize;	// 程序头部表格的表项大小
	Elf32_Half	e_phnum;		// 程序头部表格的表项数目
	Elf32_Half	e_shentsize;	// 节区头部表格的表项大小
	Elf32_Half	e_shnum;		// 节区头部表格的表项数目
	Elf32_Half	e_shstrndx;		// 节区头部表格中与节区名称字符串表相关的表项的索引
}Elf32_Ehdr;	// ELF Header

typedef struct
{
	Elf32_Word	sh_name;		// 给出节区名称
	Elf32_Word	sh_type;		// 为节区的内容和语义进行分类
	Elf32_Word	sh_flags;		// 节区支持1位形式的标志
	Elf32_Addr	sh_addr;		// 如果节区将出现在进程的内存映像中，此成员给出节区的第一个字节应处的位置
	Elf32_Off	sh_offset;		// 此成员的取值给出节区的第一个字节与文件头之间的偏移
	Elf32_Word	sh_size;		// 此成员给出节区的字节数
	Elf32_Word	sh_link;		// 此成员给出节区头部表索引链接
	Elf32_Word	sh_info;		// 此成员给出附加信息
	Elf32_Word	sh_addralign;	// 某些节区带有地址对齐约束
	Elf32_Word	sh_entsize;		// 此成员给出每个固定大小的表项的长度字节数
}Elf32_Shdr;	// Section Header

typedef struct
{
	Elf32_Word st_name;			// 包含目标文件符号字符串表的索引
	Elf32_Addr st_value;		// 此成员给出相关联的符号的取值
	Elf32_Word st_size;			// 很多符号具有相关的尺寸大小
	Elf32_Char st_info;			// 此成员给出符号的类型和绑定属性
	Elf32_Char st_other;		// 该成员当前包含
	Elf32_Half st_shndx;		// 此成员给出相关的节区头部表索引
}Elf32_Sym;		// 符号表项字段

typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
}Elf32_Rel;		// 重定位项

typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_addend;
}Elf32_Rela;	// 重定位项(补)

typedef struct
{
	Elf32_Word	p_type;			// 此数组元素描述的段的类型
	Elf32_Off	p_offset;		// 此成员给出从文件头到该段第一个字节的偏移
	Elf32_Addr	p_vaddr;		// 此成员给出段的第一个字节将被放到内存中的虚拟地址
	Elf32_Addr	p_paddr;		// 此成员仅用于与物理地址相关的系统中
	Elf32_Word	p_filesz;		// 此成员给出段在文件映像中所占的字节数
	Elf32_Word	p_memsz;		// 此成员给出段在内存映像中占用的字节数
	Elf32_Word	p_flags;		// 此成员给出与段相关的标志
	Elf32_Word	p_align;		// 此成员给出段在文件中和内存中如何对齐
}Elf32_Phdr;	// Program Header

typedef struct
{
	Elf32_Word	n_namesz;		// name length
	Elf32_Word	n_descsz;		// descriptor length
	Elf32_Word	n_type;			// type
}Elf32_Nhdr;

typedef struct
{
	Elf32_Sword	d_tag;
	union
	{
		Elf32_Word	d_val;
		Elf32_Addr	d_ptr;
	}d_un;
}Elf32_Dyn;

typedef union
{
	struct 
	{
		Elf32_Half	ver;
		Elf32_Half	cnt;
		Elf32_Word	name;
		Elf32_Word	res1;
		Elf32_Word	res2;
	}file;
	struct 
	{
		Elf32_Word	res1;
		Elf32_Half	flags;
		Elf32_Half	ver;
		Elf32_Word	name;
		Elf32_Word	res2;
	}sym;
}Elf32_Verr;

#define EIM_ELF	0x464C457F		// 文件标示

enum EI_CLASS
{
	EIC_NONE =	0,		// 非法类别
	EIC_32 =	1,		// 32位目标
	EIC_64 =	2,		// 64位目标
};

enum EI_DATA
{
	EID_NONE =	0,		// 非法数据编码
	EID_2LSB =	1,		// 高位在前
	EID_2MSB =	2,		// 低位在前
};

enum E_VERSION
{
	EV_NONE =		0,	// 非法版本
	EV_CURRENT =	1,	// 当前版本
};

enum E_TYPE
{
	ET_NONE =	0,		// 未知目标文件格式
	ET_REL =	1,		// 可重定位文件
	ET_EXEC =	2,		// 可执行文件
	ET_DYN =	3,		// 共享目标文件
	ET_CORE =	4,		// Core文件（转储格式）
	ET_LOPROC =	0xFF00,	// 特定处理
	ET_HIPROC =	0xFFFF,	// 器文件
};

enum E_MACHINE
{
	EM_NONE =			0,	// 未指定
	EM_M32 =			1,	// AT&T WE 32100
	EM_SPARC =			2,	// SPARC
	EM_386 =			3,	// Intel 80386
	EM_68K =			4,	// Motorola 68000
	EM_88K =			5,	// Motorola 88000
	EM_486 =			6,	// Intel 80486
	EM_860 =			7,	// Intel 80860
	EM_MIPS =			8,	// MIPS RS3000 大端模式
	EM_MIPS_RS3_LE =	10,	// MIPS 小端模式
	EM_RS6000 =			11,	// RS6000
	EM_PARISC =			15,	// HP PA-RISC
	EM_nCUBE =			16,	// nCUBE
	EM_VPP500 =			17,	// Fujitsu VPP500
	EM_SPARC32PLUS =	18,	// SPARC32+
	EM_PPC =			20,	// PowerPC
	EM_SPARCV9 =		43,	// SPARC V9
};

enum SH_TYPE
{
	SHT_NULL =		0,	// 此值标志节区头部是非活动的，没有对应的节区。此节区头部中的其他成员取值无意义。
	SHT_PROGBITS =	1,	// 此节区包含程序定义的信息，其格式和含义都由程序来解释。
	SHT_SYMTAB =	2,	// 此节区包含一个符号表。
	SHT_STRTAB =	3,	// 此节区包含字符串表。目标文件可能包含多个字符串表节区。
	SHT_RELA =		4,	// 此节区包含重定位表项。
	SHT_HASH =		5,	// 此节区包含符号哈希表。
	SHT_DYNAMIC =	6,	// 此节区包含动态链接的信息。目前一个目标文件中只能包含一个动态节区，将来可能会取消这一限制。
	SHT_NOTE =		7,	// 此节区包含以某种方式来标记文件的信息。
	SHT_NOBITS =	8,	// 这种类型的节区不占用文件中的空间。其他方面和SHT_PROGBITS相似。
	SHT_REL =		9,	// 此节区包含重定位表项，其中没有补齐（addends），例如32位目标文件中的Elf32_rel类型。
	SHT_SHLIB =		10,	// 此节区被保留，不过其语义是未规定的。包含此类型节区的程序与ABI不兼容。
	SHT_DYNSYM =	11,	// 作为一个完整的符号表，它可能包含很多对动态链接而言不必要的符号。
	SHT_LOOS =		0x60000000,	// 此范围的类型保留给
	SHT_HIOS =		0x6FFFFFFF,	// 操作系统保留。
	SHT_LOPROC =	0x70000000,	// 此范围的类型保留给
	SHT_HIPROC =	0x7FFFFFFF,	// 处理器专用语义。
	SHT_LOUSER =	0x80000000,	// 此值给出保留给应用程序的索引下界。
	SHT_HIUSER =	0xFFFFFFFF,	// 此值给出保留给应用程序的索引上界。
};

enum SH_FLAGS
{
	SHF_WRITE =		1,	// 节区包含进程执行过程中将可写的数据。
	SHF_ALLOC =		2,	// 此节区在进程执行过程中占用内存。
	SHF_EXECINSTR =	4,	// 节区包含可执行的机器指令。
	SHF_MASKPROC =	0xF0000000,	// 所有包含于此掩码中的四位都用于处理器专用的语义。
};

enum P_TYPE
{
	PT_NULL =		0,	// 此数组元素未用。
	PT_LOAD =		1,	// 此数组元素给出一个可加载的段。
	PT_DYNAMIC =	2,	// 数组元素给出动态链接信息。
	PT_INTERP =		3,	// 数组元素给出一个 NULL 结尾的字符串的位置和长度，该字符串将被当作解释器调用。
	PT_NOTE	 =		4,	// 此数组元素给出附加信息的位置和大小。
	PT_SHLIB =		5,	// 此段类型被保留，不过语义未指定。
	PT_PHDR =		6,	// 此类型的数组元素如果存在，则给出了程序头部表自身的大小和位置，既包括在文件中也包括在内存中的信息。
	PT_LOOS =		0x60000000,	// 此范围的类型保留给
	PT_HIOS =		0x6FFFFFFF,	// 操作系统保留。
	PT_LOPROC =		0x70000000,	// 此范围的类型保留给
	PT_HIPROC =		0x7FFFFFFF,	// 处理器专用语义。
};

enum P_FLAGS
{
	PF_X =			1,	// 可执行
	PF_W =			2,	// 可写
	PF_R =			4,	// 可读
	PF_MASKOS =		0x0FF00000,	// 操作系统保留
	PF_MASKPROC =	0xF0000000,	// 处理器专用语义
};

enum ST_TYPE
{
	STT_NOTYPE =	0,	// 符号的类型没有指定
	STT_OBJECT =	1,	// 符号与某个数据对象相关，比如一个变量、数组等等
	STT_FUNC =		2,	// 符号与某个函数或者其他可执行代码相关
	STT_SECTION =	3,	// 符号与某个节区相关
	STT_FILE =		4,	// 符号的名称给出了与目标文件相关的源文件的名称
	STT_LOPROC =	13,	// 此范围的类型保留给
	STT_HIPROC =	15,	// 处理器专用语义。
};

enum ST_BIND
{
	STB_LOCAL =		0,	// 局部符号在包含该符号定义的目标文件以外不可见
	STB_GLOBAL =	1,	// 全局符号对所有将组合的目标文件都是可见的
	STB_WEAK =		2,	// 弱符号与全局符号类似，不过他们的定义优先级比较低
	STB_LOPROC =	13,	// 此范围的类型保留给
	STB_HIPROC =	15,	// 处理器专用语义。
};

enum D_TYPE
{
	DT_NULL =		0,
	DT_NEEDED =		1,
	DT_PLTRELSZ =	2,
	DT_PLTGOT =		3,
	DT_HASH =		4,
	DT_STRTAB =		5,
	DT_SYMTAB =		6,
	DT_RELA =		7,
	DT_RELASZ =		8,
	DT_RELAENT =	9,
	DT_STRSZ =		10,
	DT_SYMENT =		11,
	DT_INIT =		12,
	DT_FINI =		13,
	DT_SONAME =		14,
	DT_RPATH =		15,
	DT_SYMBOLIC =	16,
	DT_REL =		17,
	DT_RELSZ =		18,
	DT_RELENT =		19,
	DT_PLTREL =		20,
	DT_DEBUG =		21,
	DT_TEXTREL =	22,
	DT_JMPREL =		23,
	DT_VERSYM =		0x6FFFFFF0,
	DT_VERNEED =	0x6FFFFFFE,
	DT_VERNEEDNUM =	0x6FFFFFFF,
	DT_LOPROC =		0x70000000,
	DT_HIPROC =		0x7FFFFFFF,
};
