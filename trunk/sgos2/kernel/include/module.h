#ifndef _MODULE_H_
#define _MODULE_H_

#include <sgos.h>
#include <process.h>

#define MODULE_MAGIC	0xFF0B47FC

#define MODULE_NAME_LEN		32

// 动态链接库可加载在0x10000000 - 0x80000000之间
#define SYMBOL_NAME_LEN		128

//for export and import
typedef struct SYMBOL_ENTRY{
	char		name[SYMBOL_NAME_LEN];	//name
	ushort		ordinal;		//编号
	size_t		address;		//相对模块地址（RVA）
}SYMBOL_ENTRY;

//relocation
typedef struct RELOC_ENTRY{
	size_t		reloc_addr;	//重定位地址
}RELOC_ENTRY;

struct MODULE_LINK;
// 加载的模块描述和数据
typedef struct MODULE{
	char*		name;		//以 '\0' 结尾的字符串
	struct MODULE*	pre, *next;	//pre & next module，链表
	uint		magic;		//模块标识
	uchar		share;		//share??
	ushort		reference;	//引用的进程计数，当引用减为0时，可以卸载该模块，
					//初始化过程中也为0，避免未初始化完毕就被使用。
	size_t		vir_addr;	//模块占用的虚拟地址开始（4KB对齐） 实际可能会改变
	size_t		vir_size;	//模块在内存中占用的虚拟大小（4KB对齐）
	size_t*		page_table;	//页面数据
	ushort		page_num;	//占用页面个数
	size_t		entry_address;	//模块入口
	size_t		stack_size;	//堆栈大小
	ushort		import_num;	//导入模块数
	ushort		export_num;	//导出符号数
	struct MODULE**	import_modules;	//导入模块数组
	SYMBOL_ENTRY*	export_table;	//导出模块数组
	ushort		reloc_num;	//重定位项数
	RELOC_ENTRY*	reloc_table;	//重定位表
	char*		full_name;	//文件路径名称
	int		mid;		//模块ID
}MODULE;

// 进程模块信息
typedef struct MODULE_LINK{
	struct MODULE_LINK	*pre, *next;	//link
	MODULE*			module;
}MODULE_LINK;


struct PROCESS;
//模块函数
//模块初始化
void module_init();
//添加模块
MODULE*	module_add( struct PROCESS* proc, size_t addr, size_t size, uchar type, char* name );
//释放模块
MODULE* module_free( struct PROCESS* proc, MODULE* mod );
//由名称获取模块，会增加引用计数。。。
MODULE* module_get_by_name( struct PROCESS* proc, char* name );
//
MODULE* module_get( struct PROCESS* proc, int id );
//进程映射模块的页面
void module_attach( struct PROCESS* proc, MODULE* mod );
//获取export地址
size_t module_get_export_addr( MODULE* mod, char* name );
//添加到进程链表
void module_link( struct PROCESS* proc, MODULE* mod );
//从进程链表中删除
void module_unlink( struct PROCESS* proc, MODULE* mod );
//从进程链表中查找模块
MODULE* module_search( struct PROCESS* proc, size_t vir_addr );
MODULE* module_search_by_name( struct PROCESS* proc, char* name );
//

#endif	//_MODULE_H_

