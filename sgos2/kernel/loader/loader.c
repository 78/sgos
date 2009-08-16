//SGOS2:loader
// BXML Loader
// written by Huang Guan 090810

#include <sgos.h>
#include <arch.h>
#include <string.h>
#include <debug.h>
#include <process.h>
#include <thread.h>
#include <bxml.h>

#define EQU( str_a, str_b ) ( strcmp(bxml_readstr(bxml, str_a), str_b)==0 )

// 从文件中加载可执行文件。
int loader_load( PROCESS* proc, char* filename, MODULE** mod )
{
/*
	//make a message to fs server
	BXML_DATA* bxml;
	size_t size;
	uchar* data;
	bxml = bxml_parse("<kmsg to=\"fs\" command=\"readall\" />");
	bxml_writestr(bxml, ":file", filename );
	bxml = kmsg_send( bxml );	//kmsg_send will help us free the bxml buffer.
	if( !EQU(":result", "ok" ) ){
		bxml_free(bxml);
		return -0x01;	//read file error.
	}
	bxml_read( bxml, ":size", &size, sizeof(size) );
	if(size){
		data = kmalloc( size );
		bxml_read( bxml, ".", data, size );
	}
*/
	die("not implemented.");
}

// 处理加载过程，加载成功返回0。
// char* file必须是完整文件路径。
int loader_process( PROCESS* proc, char* file, uchar* data, uchar share, MODULE** ret_mod )
{
	struct BXML_DATA* bxml;
	MODULE* mod;
	size_t load_addr, load_size;
	uint old_page_dir = 0;
	//parse bxml
	bxml = bxml_parse( data );
	if( !bxml ){
		PERROR("##not an executable bxml file.");
		return -0x11;	//not an executable
	}
	if( strcmp(bxml_readstr(bxml, ":version"), "1.0")!=0 ){
		PERROR("##version is not supported.");
		bxml_free( bxml );
		return -0x12;
	}
	//获取程序基本信息
	if( !bxml_redirect(bxml, "/program", 0) ){
		PERROR("##program not found.");
		bxml_free( bxml );
		return -0x13;
	}
	bxml_read(bxml, ":base", &load_addr, sizeof(load_addr));
	bxml_read(bxml, ":size", &load_size, sizeof(load_size));
	//add a module
	mod = module_add( proc, load_addr, load_size, share, file );
	if(mod){
		//添加到进程链表
		module_link( proc, mod );
		//可执行文件都有入口
		bxml_read(bxml, ":entry_address", &mod->entry_address, sizeof(mod->entry_address) );
		//有的程序指定了堆栈大小
		bxml_read(bxml, ":stack_size", &mod->stack_size, sizeof(mod->stack_size) );
		//开始读入数据
		if( proc->page_dir != current_proc()->page_dir )
			old_page_dir = switch_page_dir( proc->page_dir );
		//section data
		if( bxml_redirect(bxml, "/program/section_table/section", 0) ){
			do{
				char* name;
				size_t addr, siz;
				//read section attributes
				name = bxml_readstr(bxml, ":name");
				bxml_read(bxml, ":virtual_address", &addr, sizeof(addr));
				bxml_read(bxml, ":virtual_size", &siz, sizeof(siz) );
				addr += load_addr;
				//保证不会越界
				if( addr+siz <= load_addr+load_size ){
					if( bxml_read(bxml, ".", (void*)addr, siz ) == 0 ){
						//没有读取任何数据，则做清0操作
						memset( (void*)addr, 0, siz );
					}
				//	PERROR("load section: %s at %x", name, addr );
				}else{
					PERROR("## out of boundary.");
				}
			}while(bxml_movenext(bxml));
		}
		//export table
		if( bxml_redirect(bxml, "/program/export_table/export", 0) ){
			int export_num = 0;
			do export_num ++; while(bxml_movenext(bxml));	//获取符号数目
			//分配存储空间
			mod->export_table = (SYMBOL_ENTRY*)kmalloc( export_num * sizeof(SYMBOL_ENTRY) );
			if( mod->export_table ){
				//重新定位
				bxml_redirect(bxml, "../export", 0);
				do{
					char* name;
					int index;
					size_t addr, ordinal;
					//read export entry attributes
					name = bxml_readstr(bxml, ":name");
					bxml_read(bxml, ":virtual_address", &addr, sizeof(addr));
					bxml_read(bxml, ":id", &ordinal, sizeof(ordinal) );
					addr += load_addr;
					index = mod->export_num;
					mod->export_table[index].ordinal = ordinal;
					mod->export_table[index].address = addr;
					strncpy( mod->export_table[index].name, name, SYMBOL_NAME_LEN );
					mod->export_num ++;
				}while(bxml_movenext(bxml));
			}
		}
		//import table
		if( bxml_redirect(bxml, "/program/import_table/module", 0) ){
			do{
				MODULE* mod_imp;
				char* name;
				char* tmp;
				name = bxml_readstr(bxml, ":name");
				mod_imp = module_get( proc, name ); //看看是否能直接得到
				if( !mod_imp ){
					//否则从文件系统中加载
					if( loader_load(proc, name, &mod_imp) != 0 ){
						PERROR("##failed to load module: %s", name );
						break;
					}
				}
				if( mod_imp && bxml_redirect(bxml, "import", 0) ){
					do{	//修改所有导入地址
						char* name;
						size_t addr, ordinal;
						//read export entry attributes
						name = bxml_readstr(bxml, ":name");
						bxml_read(bxml, ":virtual_address", &addr, sizeof(addr));
						bxml_read(bxml, ":id", &ordinal, sizeof(ordinal) );
						addr += load_addr;
						//保证不会越界
						if( addr+sizeof(size_t) <= load_addr+load_size ){
							//写入地址
							*(size_t*)addr = module_get_export_addr(mod_imp, name);
						}else{
							PERROR("## out of boundary.");
						}
					}while(bxml_movenext(bxml));
				}
				//back
				tmp = (char*)kmalloc(1024);
				sprintf(tmp, "/program/import_table/module?name=%s", name );
				bxml_redirect(bxml, tmp, 0 );
				kfree(tmp);
			}while(bxml_movenext(bxml));
		}
		if( old_page_dir )	//切换过页目录，则恢复
			load_page_dir( old_page_dir );
	}else{ //mod==NULL
		bxml_free(bxml);
		PERROR("##module add failed.");
		return -0x14;
	}
	//free bxml.
	bxml_free(bxml);
	//the module is ok for use now.
	//
	if( mod->share ){
		//若是共享库，则设置页面为只读，以便写时复制
//		page_set_attr( mod->vir_addr, mod->vir_size, P_PRESENT|P_USER );
		module_attach( proc, mod );
	}
	mod->reference = 1;
	//导出参数
	if( ret_mod )
		*ret_mod = mod;
	//加载成功
	kprintf("Module %s is loaded successfully at 0x%X\n", file, mod->vir_addr);
	return 0;
}
