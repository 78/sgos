//����ELF�е���������

typedef unsigned long	Elf32_Addr;
typedef unsigned char	Elf32_Char;
typedef unsigned short	Elf32_Half;
typedef unsigned long	Elf32_Off;
typedef long			Elf32_Sword;
typedef unsigned long	Elf32_Word;

typedef struct
{
	Elf32_Word	ei_mag;		// �ļ���ʶ
	Elf32_Char	ei_class;	// �ļ���
	Elf32_Char	ei_data;	// ���ݱ���
	Elf32_Char	ei_version;	// �ļ��汾
	Elf32_Char	ei_pad;		// �����ֽڿ�ʼ��
	Elf32_Char	ei_res[8];	// ����
}Elf32_Ident;

typedef struct
{
	Elf32_Ident	e_ident;		// Ŀ���ļ���ʶ
	Elf32_Half	e_type;			// Ŀ���ļ�����
	Elf32_Half	e_machine;		// �ļ���Ŀ����ϵ�ṹ����
	Elf32_Word	e_version;		// Ŀ���ļ��汾
	Elf32_Addr	e_entry;		// ������ڵ������ַ
	Elf32_Off	e_phoff;		// ����ͷ������ƫ����
	Elf32_Off	e_shoff;		// ����ͷ������ƫ����
	Elf32_Word	e_flags;		// �������ļ���ص��ض��ڴ������ı�־
	Elf32_Half	e_ehsize;		// ELFͷ���Ĵ�С
	Elf32_Half	e_phentsize;	// ����ͷ�����ı����С
	Elf32_Half	e_phnum;		// ����ͷ�����ı�����Ŀ
	Elf32_Half	e_shentsize;	// ����ͷ�����ı����С
	Elf32_Half	e_shnum;		// ����ͷ�����ı�����Ŀ
	Elf32_Half	e_shstrndx;		// ����ͷ�����������������ַ�������صı��������
}Elf32_Ehdr;	// ELF Header

typedef struct
{
	Elf32_Word	sh_name;		// ������������
	Elf32_Word	sh_type;		// Ϊ���������ݺ�������з���
	Elf32_Word	sh_flags;		// ����֧��1λ��ʽ�ı�־
	Elf32_Addr	sh_addr;		// ��������������ڽ��̵��ڴ�ӳ���У��˳�Ա���������ĵ�һ���ֽ�Ӧ����λ��
	Elf32_Off	sh_offset;		// �˳�Ա��ȡֵ���������ĵ�һ���ֽ����ļ�ͷ֮���ƫ��
	Elf32_Word	sh_size;		// �˳�Ա�����������ֽ���
	Elf32_Word	sh_link;		// �˳�Ա��������ͷ������������
	Elf32_Word	sh_info;		// �˳�Ա����������Ϣ
	Elf32_Word	sh_addralign;	// ĳЩ�������е�ַ����Լ��
	Elf32_Word	sh_entsize;		// �˳�Ա����ÿ���̶���С�ı���ĳ����ֽ���
}Elf32_Shdr;	// Section Header

typedef struct
{
	Elf32_Word st_name;			// ����Ŀ���ļ������ַ����������
	Elf32_Addr st_value;		// �˳�Ա����������ķ��ŵ�ȡֵ
	Elf32_Word st_size;			// �ܶ���ž�����صĳߴ��С
	Elf32_Char st_info;			// �˳�Ա�������ŵ����ͺͰ�����
	Elf32_Char st_other;		// �ó�Ա��ǰ����
	Elf32_Half st_shndx;		// �˳�Ա������صĽ���ͷ��������
}Elf32_Sym;		// ���ű����ֶ�

typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
}Elf32_Rel;		// �ض�λ��

typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_addend;
}Elf32_Rela;	// �ض�λ��(��)

typedef struct
{
	Elf32_Word	p_type;			// ������Ԫ�������Ķε�����
	Elf32_Off	p_offset;		// �˳�Ա�������ļ�ͷ���öε�һ���ֽڵ�ƫ��
	Elf32_Addr	p_vaddr;		// �˳�Ա�����εĵ�һ���ֽڽ����ŵ��ڴ��е������ַ
	Elf32_Addr	p_paddr;		// �˳�Ա�������������ַ��ص�ϵͳ��
	Elf32_Word	p_filesz;		// �˳�Ա���������ļ�ӳ������ռ���ֽ���
	Elf32_Word	p_memsz;		// �˳�Ա���������ڴ�ӳ����ռ�õ��ֽ���
	Elf32_Word	p_flags;		// �˳�Ա���������صı�־
	Elf32_Word	p_align;		// �˳�Ա���������ļ��к��ڴ�����ζ���
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

#define EIM_ELF	0x464C457F		// �ļ���ʾ

enum EI_CLASS
{
	EIC_NONE =	0,		// �Ƿ����
	EIC_32 =	1,		// 32λĿ��
	EIC_64 =	2,		// 64λĿ��
};

enum EI_DATA
{
	EID_NONE =	0,		// �Ƿ����ݱ���
	EID_2LSB =	1,		// ��λ��ǰ
	EID_2MSB =	2,		// ��λ��ǰ
};

enum E_VERSION
{
	EV_NONE =		0,	// �Ƿ��汾
	EV_CURRENT =	1,	// ��ǰ�汾
};

enum E_TYPE
{
	ET_NONE =	0,		// δ֪Ŀ���ļ���ʽ
	ET_REL =	1,		// ���ض�λ�ļ�
	ET_EXEC =	2,		// ��ִ���ļ�
	ET_DYN =	3,		// ����Ŀ���ļ�
	ET_CORE =	4,		// Core�ļ���ת����ʽ��
	ET_LOPROC =	0xFF00,	// �ض�����
	ET_HIPROC =	0xFFFF,	// ���ļ�
};

enum E_MACHINE
{
	EM_NONE =			0,	// δָ��
	EM_M32 =			1,	// AT&T WE 32100
	EM_SPARC =			2,	// SPARC
	EM_386 =			3,	// Intel 80386
	EM_68K =			4,	// Motorola 68000
	EM_88K =			5,	// Motorola 88000
	EM_486 =			6,	// Intel 80486
	EM_860 =			7,	// Intel 80860
	EM_MIPS =			8,	// MIPS RS3000 ���ģʽ
	EM_MIPS_RS3_LE =	10,	// MIPS С��ģʽ
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
	SHT_NULL =		0,	// ��ֵ��־����ͷ���Ƿǻ�ģ�û�ж�Ӧ�Ľ������˽���ͷ���е�������Աȡֵ�����塣
	SHT_PROGBITS =	1,	// �˽����������������Ϣ�����ʽ�ͺ��嶼�ɳ��������͡�
	SHT_SYMTAB =	2,	// �˽�������һ�����ű�
	SHT_STRTAB =	3,	// �˽��������ַ�����Ŀ���ļ����ܰ�������ַ����������
	SHT_RELA =		4,	// �˽��������ض�λ���
	SHT_HASH =		5,	// �˽����������Ź�ϣ��
	SHT_DYNAMIC =	6,	// �˽���������̬���ӵ���Ϣ��Ŀǰһ��Ŀ���ļ���ֻ�ܰ���һ����̬�������������ܻ�ȡ����һ���ơ�
	SHT_NOTE =		7,	// �˽���������ĳ�ַ�ʽ������ļ�����Ϣ��
	SHT_NOBITS =	8,	// �������͵Ľ�����ռ���ļ��еĿռ䡣���������SHT_PROGBITS���ơ�
	SHT_REL =		9,	// �˽��������ض�λ�������û�в��루addends��������32λĿ���ļ��е�Elf32_rel���͡�
	SHT_SHLIB =		10,	// �˽�����������������������δ�涨�ġ����������ͽ����ĳ�����ABI�����ݡ�
	SHT_DYNSYM =	11,	// ��Ϊһ�������ķ��ű������ܰ����ܶ�Զ�̬���Ӷ��Բ���Ҫ�ķ��š�
	SHT_LOOS =		0x60000000,	// �˷�Χ�����ͱ�����
	SHT_HIOS =		0x6FFFFFFF,	// ����ϵͳ������
	SHT_LOPROC =	0x70000000,	// �˷�Χ�����ͱ�����
	SHT_HIPROC =	0x7FFFFFFF,	// ������ר�����塣
	SHT_LOUSER =	0x80000000,	// ��ֵ����������Ӧ�ó���������½硣
	SHT_HIUSER =	0xFFFFFFFF,	// ��ֵ����������Ӧ�ó���������Ͻ硣
};

enum SH_FLAGS
{
	SHF_WRITE =		1,	// ������������ִ�й����н���д�����ݡ�
	SHF_ALLOC =		2,	// �˽����ڽ���ִ�й�����ռ���ڴ档
	SHF_EXECINSTR =	4,	// ����������ִ�еĻ���ָ�
	SHF_MASKPROC =	0xF0000000,	// ���а����ڴ������е���λ�����ڴ�����ר�õ����塣
};

enum P_TYPE
{
	PT_NULL =		0,	// ������Ԫ��δ�á�
	PT_LOAD =		1,	// ������Ԫ�ظ���һ���ɼ��صĶΡ�
	PT_DYNAMIC =	2,	// ����Ԫ�ظ�����̬������Ϣ��
	PT_INTERP =		3,	// ����Ԫ�ظ���һ�� NULL ��β���ַ�����λ�úͳ��ȣ����ַ��������������������á�
	PT_NOTE	 =		4,	// ������Ԫ�ظ���������Ϣ��λ�úʹ�С��
	PT_SHLIB =		5,	// �˶����ͱ���������������δָ����
	PT_PHDR =		6,	// �����͵�����Ԫ��������ڣ�������˳���ͷ��������Ĵ�С��λ�ã��Ȱ������ļ���Ҳ�������ڴ��е���Ϣ��
	PT_LOOS =		0x60000000,	// �˷�Χ�����ͱ�����
	PT_HIOS =		0x6FFFFFFF,	// ����ϵͳ������
	PT_LOPROC =		0x70000000,	// �˷�Χ�����ͱ�����
	PT_HIPROC =		0x7FFFFFFF,	// ������ר�����塣
};

enum P_FLAGS
{
	PF_X =			1,	// ��ִ��
	PF_W =			2,	// ��д
	PF_R =			4,	// �ɶ�
	PF_MASKOS =		0x0FF00000,	// ����ϵͳ����
	PF_MASKPROC =	0xF0000000,	// ������ר������
};

enum ST_TYPE
{
	STT_NOTYPE =	0,	// ���ŵ�����û��ָ��
	STT_OBJECT =	1,	// ������ĳ�����ݶ�����أ�����һ������������ȵ�
	STT_FUNC =		2,	// ������ĳ����������������ִ�д������
	STT_SECTION =	3,	// ������ĳ���������
	STT_FILE =		4,	// ���ŵ����Ƹ�������Ŀ���ļ���ص�Դ�ļ�������
	STT_LOPROC =	13,	// �˷�Χ�����ͱ�����
	STT_HIPROC =	15,	// ������ר�����塣
};

enum ST_BIND
{
	STB_LOCAL =		0,	// �ֲ������ڰ����÷��Ŷ����Ŀ���ļ����ⲻ�ɼ�
	STB_GLOBAL =	1,	// ȫ�ַ��Ŷ����н���ϵ�Ŀ���ļ����ǿɼ���
	STB_WEAK =		2,	// ��������ȫ�ַ������ƣ��������ǵĶ������ȼ��Ƚϵ�
	STB_LOPROC =	13,	// �˷�Χ�����ͱ�����
	STB_HIPROC =	15,	// ������ר�����塣
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
