//pagefault
//sgos2
//huang guan 090806

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <mm.h>


//pagefault
//页面缺页、权限等异常处理
int pagefault_handler( int err_code, I386_REGISTERS* r )
{
	uint addr;	
	//被访问或修改的内存地址
	__asm__("movl %%cr2, %0" : "=r"( addr ) );
	PROCESS* proc = current_proc();
	if( !(err_code&P_PRESENT ) ){	//not present this page
		if( IS_KERNEL_MEMORY( addr ) ){	//如果是在内核空间
			//判断是否用户态违规访问。
			if( err_code&P_USER )
				KERROR("##user trying to write the kernel memory!!");
			// !proc表示进程尚未初始化，此时无需检查边界
			if( !proc || kcheck_allocated( addr ) ||
				(addr>=PROC_PAGE_DIR_BASE && addr<PROC_PAGE_DIR_END) ){//是否允许？
				uint phys_addr = get_phys_page();
				addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
				if( !phys_addr )
					KERROR("##no page for allocation.");
				//映射到分配的物理地址去。
				map_one_page( kernel_page_dir, addr, phys_addr, P_WRITE );
				return 0;
			}else{
				PERROR("##trying to write the unallocated KERNEL memory!!");
			}
		}else if( IS_USER_MEMORY( addr ) ){ //用户态
			if( ucheck_allocated( proc, addr ) ){	//
				MODULE* mod;
				uint phys_addr = get_phys_page();
				addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
				if( !phys_addr )
					KERROR("##no page for allocation.");
				//映射到分配的物理地址去。
				map_one_page( proc->page_dir, addr, phys_addr, P_WRITE | P_USER );
				//判断是否属于某个模块
				mod = module_search( proc, addr );
				if( mod && mod->share && mod->page_table ){
					//设置模块页面
					mod->page_table[(addr-mod->vir_addr)>>PAGE_SIZE_BITS] = phys_addr;
				}
				return 0;
			}else{
				PERROR("##trying to write the unallocated user memory!!");
			}
		}
		PERROR("##not present this page at 0x%X.", addr);
	}else if(!(err_code&P_WRITE)){//写了只读页面
		if(err_code&P_USER){
			PERROR("##user try to write a read only page at 0x%X.", addr );
		}else{
			PERROR("##kernel try to access read only page at 0x%X.", addr );
		}
	}
	//未能处理，则BSOD
	isr_dumpcpu( r );
	KERROR("##unhandled pagefault at 0x%X, err_code=0x%X", addr, err_code );
	return 0;
}

