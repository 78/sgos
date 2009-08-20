//map virtual memory to physical memory
//sgos2
//huang guan 090806

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <process.h>
#include <thread.h>

//映射一个临时页，虚拟地址是占用了进程页目录地址的
uint map_temp_page( uint phys_addr )
{
	uint temp_addr = get_page_dir();
	PAGE_DIR *dir, *de, *te;
	dir = (PAGE_DIR*)kernel_page_dir;
	de = dir + (temp_addr>>22);
	if( !de->v )
		KERROR("## impossible. de->v = 0x%X", de->v );
	te = (PAGE_TABLE*)PROC_PAGE_TABLE_MAP + (temp_addr>>12);
	if( te->v )
		PERROR("## impossible. temp_addr: 0x%X  te->v = 0x%X", temp_addr, te->v );
	te->v = phys_addr;
	te->a.present = te->a.write = 1;
	reflush_pages();
	return temp_addr;
}

//释放一个临时页
void unmap_temp_page( uint vir_addr )
{
//	what's the use for to change it to 0?
	PAGE_DIR *dir, *de, *te;
	dir = (PAGE_DIR*)kernel_page_dir;
	de = dir + (vir_addr>>22);
	if( !de->v )
		KERROR("## impossible. de->v = 0x%X", de->v );
	te = (PAGE_TABLE*)PROC_PAGE_TABLE_MAP + (vir_addr>>12);
	if( !te->v )
		PERROR("## impossible. te->v = 0x%X", te->v );
	te->v = 0;
	free_page_dir( vir_addr );
}

// map only one page from virtual address to physical address
void map_one_page( uint dir, uint vir_addr, uint phys_addr, uint attr )
{
	PAGE_DIR* de, *te;
	int newpage=0;
	if( phys_addr%PAGE_SIZE || vir_addr%PAGE_SIZE ){
		PERROR("## wrong vir_addr or phys_addr" );
		return;
	} 
	// get page directory entry
	de = (PAGE_DIR*)dir + (vir_addr>>22);
	if( !de->v ){	//no table
		newpage = 1;
		de->v = get_phys_page();
		if( !de->v )
			return;
		if( attr&P_USER )
			de->a.user = 1;
		if( attr&P_WRITE )
			de->a.write = 1;
		de->a.present = 1;
	}
	// get page table entry
	te = (PAGE_TABLE*)PROC_PAGE_TABLE_MAP + (vir_addr>>12);
	if( newpage ){
		reflush_pages();	//刷新页目录了，否则下面这句就不管用了
		memsetd( (PAGE_TABLE*)PROC_PAGE_TABLE_MAP + ((vir_addr>>22)<<10),
			 0, PAGE_SIZE>>2 );
	}
//	if( te->v )	//remap??
//		free_phys_page( (uint)(te->a.phys_addr<<12) );
	//设置新的值
	te->v = phys_addr;
	if( attr&P_USER )
		te->a.user = 1;
	if( attr&P_WRITE )
		te->a.write = 1;
	te->a.present = 1;
	//更改了分页信息，刷新页目录
	reflush_pages();
}

//取消一个页面的映射
void unmap_one_page( uint dir, uint vir_addr )
{
	PAGE_DIR* de, *te;
	if( vir_addr%PAGE_SIZE ){
		PERROR("## wrong vir_addr" );
		return;
	} 
	// get page directory entry
	de = (PAGE_DIR*)dir + (vir_addr>>22);
	if( !de->v )	//no dir entry
		return;
	// get page table entry
	te = (PAGE_TABLE*)PROC_PAGE_TABLE_MAP + (vir_addr>>12);
	if( te->v )	//there it is!!
		free_phys_page( (uint)(te->a.phys_addr<<12) );
	te->v = 0;
}


// map virutal space to physical space
void map_pages( uint dir, uint vir_addr, uint phys_addr, uint size, uint attr )
{
	uint count;
	count = size >> PAGE_SIZE_BITS;
//	kprintf("map vir:0x%X to phy:0x%X  size:0x%X\n", vir_addr, phys_addr, size );
	for( ; count; count--, vir_addr+=PAGE_SIZE, phys_addr+=PAGE_SIZE ){
		map_one_page( dir, vir_addr, phys_addr, attr );
	}
}

//取消一堆页面的映射。。。
void unmap_pages( uint dir, uint vir_addr, uint size )
{
	uint count;
	count = size >> PAGE_SIZE_BITS;
	for( ; count; count--, vir_addr+=PAGE_SIZE ){
		unmap_one_page( dir, vir_addr );
	}
}
