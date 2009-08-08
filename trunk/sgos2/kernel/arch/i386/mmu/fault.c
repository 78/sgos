//pagefault
//sgos2
//huang guan 090806

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <mm.h>

#define IS_KERNEL_MEMORY(addr) ( addr>=KERNEL_BASE && addr<PROC_PAGE_DIR_BASE )

//pagefault
int pagefault_handler( int err_code, I386_REGISTERS* r )
{
	uint addr;	//被访问或修改的内存地址
	__asm__("movl %%cr2, %0" : "=r"( addr ) );
//	kprintf("page fault at 0x%X\n", addr );
	if( !(err_code&P_PRESENT ) ){	//not present this page
		if( IS_KERNEL_MEMORY( addr ) ){
			uint phys_addr = get_phys_page();
			addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
			if( !phys_addr )
				KERROR("no page for allocation.");
			map_one_page( kernel_page_dir, addr, phys_addr, P_WRITE );
			return 0;
		}
		PERROR("not present this page at 0x%X.", addr);
	}else if(err_code&P_USER){
		if(err_code&P_WRITE){
			PERROR("user try to write a read only page at 0x%X.", addr );
		}else{
			PERROR("user try to access kernel page at 0x%X.", addr );
		}
	}
	isr_dumpcpu( r );
	KERROR("unhandled pagefault at 0x%X, err_code=0x%X", addr, err_code );
	return 0;
}

