//SGOS2: module
//Huang Guan (gdxxhg@gmail.com) 

#include <sgos.h>
#include <arch.h>
#include <mm.h>
#include <string.h>
#include <debug.h>
#include <process.h>
#include <module.h>
#include <mutex.h>

struct MODULE_INFO{
	uint	mod_num;
	MODULE*	module;
	mutex_t	mutex;
}g_mods;

static int module_id = 1;

static int generate_mid()
{
	int id;
	id = module_id;
	module_id ++;
	return id;
}


//模块初始化
void module_init()
{
	memset( &g_mods, 0, sizeof(g_mods) );
	mutex_init( &g_mods.mutex );
}

//映射进程地址到模块处
void module_attach( struct PROCESS* proc, MODULE* mod )
{
	int i;
	uint attr;
	attr = P_PRESENT | P_USER;
	if( mod->share==0 )
		attr |= P_WRITE;
	for( i=0; i<mod->page_num; i++ )
		if( mod->page_table[i] )
			map_one_page( proc->page_dir, mod->vir_addr + (i<<PAGE_SIZE_BITS), 
				mod->page_table[i], attr );
}

//取消映射
void module_detach( struct PROCESS* proc, MODULE* mod )
{
	die("not implemented.");
}

//添加模块
MODULE*	module_add( struct PROCESS* proc, size_t addr, size_t size, uchar share, char* name )
{
	MODULE* mod;
	size_t len;
	//申请内存
	mod = (MODULE*)kmalloc( sizeof(MODULE) );
	if( !mod )
		return NULL;
	memset( mod, 0, sizeof(MODULE) );
	mod->vir_addr = addr;
	mod->vir_size = size;
	mod->share = share;
	mod->mid = generate_mid();
	//模块名称，例如 /sgos2/api.bxm
	len = strlen(name);
	mod->full_name = (char*)kmalloc(len+1);	//预留'\0'的空间
	strcpy( mod->full_name, name );	//strcpy应该会补'\0'
	//短名，例如 api.bxm
	if( (mod->name = strrchr(name, '/')+1 )==(char*)1 )
		mod->name = mod->full_name;
	//获取锁
	mutex_lock( &g_mods.mutex );
	//插入到链头 
	if( g_mods.module ){//有模块
		mod->next = g_mods.module;
		g_mods.module->pre = mod;
	}
	g_mods.module = mod;
	mutex_unlock( &g_mods.mutex );
	//尝试申请虚拟内存地址
	if( umalloc_ex( proc, addr, size ) == NULL ){
		//需要重定位
		PERROR("need relocation.");
		PERROR("##not implemented.");
		mod->share = 0;	//重定位后不要共享，数据已被修改。
		//释放资源
		kfree( mod->full_name );
		kfree( mod );
		return NULL;
	}
	//申请成功了
	if( mod->share ){
		//是动态链接库，则创建共享页表
		mod->page_num = size%PAGE_SIZE ? (size>>PAGE_SIZE_BITS)+1 : (size>>PAGE_SIZE_BITS);
		mod->page_table = (void*)kmalloc( sizeof(PAGE_TABLE) * mod->page_num );
		//注意，没有assert
		if(mod->page_table)
			memset( mod->page_table, 0, sizeof(PAGE_TABLE) * mod->page_num );
	}
	return mod;
}

//释放模块
MODULE* module_free( struct PROCESS* proc, MODULE* mod )
{
	die("not implemented.");
	return NULL;
}

//添加到进程链表
void module_link( struct PROCESS* proc, MODULE* mod )
{
	MODULE_LINK* ml;
	ml = kmalloc( sizeof(MODULE_LINK) );
	if( ml ){
		ml->module = mod;
		ml->next = proc->module_link;
		if(proc->module_link)
			proc->module_link->pre = ml;
		proc->module_link = ml;
	}else{
		PERROR("failed to allocate module_link");
	}
}

//从进程链表中删除
void module_unlink( struct PROCESS* proc, MODULE* mod )
{
	die("not implemented.");
}

//由名称获取模块，会增加引用计数。。。
MODULE* module_get_by_name( struct PROCESS* proc, char* name )
{
	MODULE* mod;
	int i;
	//获取锁
	mutex_lock( &g_mods.mutex );
	mod = module_search_by_name( proc, name );
	if( !mod ){
		//查找
		for( mod=g_mods.module; mod; mod=mod->next ){
			if( mod->share && strcmp(name, mod->name)==0 &&
				mod->reference>0 ){
				//尝试申请虚拟内存地址
				if( umalloc_ex( proc, mod->vir_addr, mod->vir_size ) == NULL ){
					//需要重定位
					PERROR("need relocation.");
					PERROR("##not implemented.");
					mutex_unlock( &g_mods.mutex );
					return NULL;
				}
				mod->reference ++;
				mutex_unlock( &g_mods.mutex );
				//连接该模块到进程
				module_link( proc, mod );
				//映射模块页面到进程
				module_attach( proc, mod );
				//加载导入模块
				for(i=0; i<mod->import_num; i++ )
					if(mod->import_modules[i]){
						module_get_by_name( proc, mod->import_modules[i]->name );
					}
				return mod;
			}
		}
	}else{
		mutex_unlock( &g_mods.mutex );
		return mod;
	}
	//not found
	mutex_unlock( &g_mods.mutex );
	PERROR("##module %s is not loaded.", name );
	return NULL;
}

MODULE* module_get( struct PROCESS* proc, int mid )
{
	MODULE_LINK* ml;
	for( ml=proc->module_link; ml; ml=ml->next )
		if( ml->module->mid == mid ){
			//we found it
			return ml->module;
		}
	//not found.
	return NULL;
}

//获取export地址
size_t module_get_export_addr( MODULE* mod, char* name )
{
	//下面的代码可以用指针优化 
	int i;
	for( i=0; i<mod->export_num; i++ ){
		if( strcmp( name, mod->export_table[i].name )==0 )
			return mod->export_table[i].address;
	}
	PERROR("export %s is not found in %s.", name, mod->name );
	return 0;
}

//从进程链表中查找模块
MODULE* module_search( struct PROCESS* proc, size_t vir_addr )
{
	MODULE_LINK* ml;
	for( ml=proc->module_link; ml; ml=ml->next )
		if( ml->module->vir_addr <= vir_addr && 
			ml->module->vir_addr+ml->module->vir_size > vir_addr ){
			//we found it
			return ml->module;
		}
	//not found.
	return NULL;
}

//从进程链表中查找模块
MODULE* module_search_by_name( struct PROCESS* proc, char* name )
{
	MODULE_LINK* ml;
	for( ml=proc->module_link; ml; ml=ml->next )
		if( strcmp(ml->module->name, name)==0 ){
			//we found it
			return ml->module;
		}
	//not found.
	return NULL;
}
