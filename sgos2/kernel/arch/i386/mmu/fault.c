//pagefault
//sgos2
//Xiaoxia 090806

#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <mm.h>
#include <tm.h>

//if in MmAllocateKernelMemory, we can not allow irqs
extern int in_allocator; 
//pagefault
//页面缺页、权限等异常处理
int pagefault_handler( int err_code, I386_REGISTERS* r )
{
	uint addr;
	uint state;
	KSpace* space;
	//被访问或修改的内存地址
	__asm__ __volatile__("movl %%cr2, %0" : "=r"( addr ) );
	space = MmGetCurrentSpace();
	if( space->SpaceId )
		PERROR("Pagefault at 0x%X: code:%x", addr, err_code);
	//stop scheduling, 为什么不允许调度？
	TmSaveScheduleState(state);
	//Enable IRQ
	/*
		If TmGetCurrentThread()==NULL, it means the kernel is sitll 
		initializing. If in MmAllocateKernelMemory, we should disable IRQ.
	*/
	if( TmGetCurrentThread() && !in_allocator )
		ArLocalEnableIrq();
	if( !(err_code&PAGE_ATTR_PRESENT ) ){	
		//not present this page
		if( IS_KERNEL_MEMORY( addr ) ){	//如果是在内核空间
			//判断是否用户态违规访问。
			if( err_code&PAGE_ATTR_USER ){
				TmRestoreScheduleState(state);
				PERROR("##user trying to write the kernel memory!!");
				return 0;
			}
			// space==NULL表示地址空间尚未初始化，此时无需检查边界
			if( addr==0xC0400004 || MmIsKernelMemoryAllocated( addr )  ){//是否允许？
				int ret;
				//末位清零，可以用&优化
				addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
				ret = MmAcquirePhysicalPage( space, addr, 
					PAGE_ATTR_ALLOCATED|PAGE_ATTR_WRITE, MAP_ATTRIBUTE );
				TmRestoreScheduleState(state);
				if( ret != 0 ){
					KERROR("##no page for allocation.");
				}
				return 1;
			}else{
				PERROR("trying to write the unallocated KERNEL memory!!");
			}
		}else if( IS_USER_MEMORY( addr ) ){ //用户态
			if( 0 && MmIsUserMemoryAllocated( space, addr ) ){	//
				int ret;
				addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
				ret = MmAcquirePhysicalPage( space, addr, 
					PAGE_ATTR_ALLOCATED|PAGE_ATTR_WRITE, MAP_ATTRIBUTE );
				TmRestoreScheduleState(state);
				return 1;
			}else{
				PERROR("trying to write the unallocated user memory at 0x%X!!", addr);
			}
		}else{
			PERROR("##write critical memory at 0x%X.", addr);
		}
	}else if(err_code&PAGE_ATTR_WRITE){//写了只读页面
		if(0 && err_code&PAGE_ATTR_USER){
			uint eflags;
			//Copy On Write
			addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
			// 分配物理页面
			uint phys_addr, temp_addr;
			phys_addr= MmGetPhysicalPage();
			if( !phys_addr ){
				TmRestoreScheduleState(state);
				KERROR("##no page for allocation.");
			}
			ArLocalSaveIrq( eflags ); //spinlock too...
			//map the new page
			temp_addr = ArMapTemporaryPage( phys_addr );
			//copy data
			RtlCopyMemory32( (void*)temp_addr, (void*)addr, PAGE_SIZE>>2 );
			ArLocalRestoreIrq( eflags );
			//映射到分配的物理地址去。
			ArMapOnePage( &space->PageDirectory, addr, phys_addr, 
				PAGE_ATTR_WRITE | PAGE_ATTR_USER, MAP_ADDRESS|MAP_ATTRIBUTE );
			TmRestoreScheduleState(state);
			return 1;
		}else{
			PERROR("##kernel try to access read only page at 0x%X.", addr );
		}
	}else if(err_code&PAGE_ATTR_USER){
		//
		PERROR("##user try to access kernel memory.");
	}
	TmRestoreScheduleState(state);
	//未能处理，则BSOD
	return 0;
}

