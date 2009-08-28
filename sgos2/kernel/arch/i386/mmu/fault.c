//pagefault
//sgos2
//huang guan 090806

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <mm.h>
#include <thread.h>

//if in kmalloc, we can not allow irqs
extern int in_allocator; 
//pagefault
//页面缺页、权限等异常处理
int pagefault_handler( int err_code, I386_REGISTERS* r )
{
	uint addr;
	uint state;
	PROCESS* proc;
	//被访问或修改的内存地址
	__asm__ __volatile__("movl %%cr2, %0" : "=r"( addr ) );
	proc = current_proc();
	//stop scheduling
	sched_state_save(state);
	//Enable IRQ
	/*
		If current_thread()==NULL, it means the kernel is sitll 
		initializing. If in kmalloc, we should disable IRQ.
	*/
	if( current_thread() && !in_allocator )
		local_irq_enable();
	if( !(err_code&P_PRESENT ) ){	
		//not present this page
		if( IS_KERNEL_MEMORY( addr ) ){	//如果是在内核空间
			//判断是否用户态违规访问。
			if( err_code&P_USER ){
				sched_state_restore(state);
				PERROR("##user trying to write the kernel memory!!");
				return 0;
			}
			// !proc表示进程尚未初始化，此时无需检查边界
			if( !proc || kcheck_allocated( addr ) ||
				(addr>=PROC_PAGE_DIR_BASE && addr<PROC_PAGE_DIR_END) ){//是否允许？
				// 分配物理页面
				uint phys_addr = get_phys_page();
				if( !phys_addr ){
					sched_state_restore(state);
					PERROR("##no page for allocation.");
					return 0;
				}
				//可以用&优化
				addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
				//映射到分配的物理地址去。
				map_one_page( kernel_page_dir, addr, phys_addr, P_WRITE );
				sched_state_restore(state);
				return 1;
			}else{
				PERROR("trying to write the unallocated KERNEL memory!!");
			}
		}else if( IS_USER_MEMORY( addr ) ){ //用户态
			if( ucheck_allocated( proc, addr ) ){	//
				MODULE* mod;
				// 分配物理页面
				uint phys_addr = get_phys_page();
				addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
				if( !phys_addr ){
					sched_state_restore(state);
					KERROR("##no page for allocation.");
				}
				//映射到分配的物理地址去。
				map_one_page( proc->page_dir, addr, phys_addr, P_WRITE | P_USER );
				//判断是否属于该进程的某个模块
				mod = module_search( proc, addr );
				/* 注意，原来这里没有添加mod->reference==0会出现问题。
				 * 首先reference==0表示这个模块正在初始化，此时需要设置页面。
				 * 当reference>0时，初始化已完成，但还可能有缺页现象，例如BSS未
				 * 初始化等（说明Loader未完善），但模块要保持干净的，因此不再修改。
				 */
				if( mod && mod->share && mod->page_table && mod->reference==0 ){
					//设置模块页面
					mod->page_table[(addr-mod->vir_addr)>>PAGE_SIZE_BITS] = phys_addr;
				}
				sched_state_restore(state);
				return 1;
			}else{
				PERROR("trying to write the unallocated user memory at 0x%X!!", addr);
			}
		}else{
			PERROR("##write critical memory at 0x%X.", addr);
		}
	}else if(err_code&P_WRITE){//写了只读页面
		if(err_code&P_USER){
			//Copy On Write
			// 分配物理页面
			uint phys_addr, temp_addr;
			phys_addr= get_phys_page();
			if( !phys_addr ){
				sched_state_restore(state);
				KERROR("##no page for allocation.");
			}
			addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
			//map the new page
			temp_addr = map_temp_page( phys_addr );
			//copy data
			memcpyd( (void*)temp_addr, (void*)addr, PAGE_SIZE>>2 );
			unmap_temp_page( temp_addr );
			//映射到分配的物理地址去。
			map_one_page( proc->page_dir, addr, phys_addr, P_WRITE | P_USER );
			sched_state_restore(state);
			return 1;
		}else{
			PERROR("##kernel try to access read only page at 0x%X.", addr );
		}
	}
	sched_state_restore(state);
	//未能处理，则BSOD
	return 0;
}

