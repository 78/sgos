//map virtual memory to physical memory
//sgos2
//huang guan 090806

#include <sgos.h>
#include <arch.h>
#include <debug.h>


// map virutal space to physical space
void map_pages( uint vir_addr, uint phys_addr, uint size, uint attr )
{
	PAGE_DIR* table_entry;
	uint count;
	table_entry = (PAGE_DIR*)PROC_PAGE_TABLE_MAP + (vir_addr>>PAGE_SIZE_BITS) ;
	count = size >> PAGE_SIZE_BITS;
	kprintf("map vir:0x%X to phy:0x%X  size:0x%X\n", vir_addr, phys_addr, size );
	for( ; count; count--, table_entry++, phys_addr+=PAGE_SIZE ){
		table_entry->v = phys_addr;
		//设置该PDE的属性
		table_entry->a.user = attr&P_USER;
		table_entry->a.write = attr&P_WRITE;
		table_entry->a.present = 1;
	}
	reflush_pages();
}

//
void unmap_pages( uint vir_addr, uint size )
{
	PAGE_DIR* table_entry;
	uint count;
	table_entry = (PAGE_DIR*)PROC_PAGE_TABLE_MAP + (vir_addr>>PAGE_SIZE_BITS) ;
	count = size >> PAGE_SIZE_BITS;
	for( ; count; count--, table_entry++ ){
		if( table_entry->v )
			free_phys_page( table_entry->a.phys_addr<<PAGE_SIZE_BITS );
		table_entry->v = 0;
	}
	reflush_pages();
}
