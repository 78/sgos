// Hope it works!!
// 090828 by Huang Guan
#include <arch.h>
#include <string.h>
#include <thread.h>
#include <debug.h>
#include <mm.h>

//为线程分配fpu
void fpu_init_thread( THREAD* thr )
{
	ARCH_THREAD* arch;
	arch = &thr->arch;
	if( !arch->fsave ){
		arch->fsave = (I387_REGISTERS*)kmalloc( sizeof(I387_REGISTERS) );
		memsetd( arch->fsave, 0, sizeof(I387_REGISTERS)>>2 );
	}
}

//保存fpu寄存器
inline void fpu_save( THREAD* thr )
{
	__asm__("fnsave %0; fwait"::"m"(*thr->arch.fsave));
}

//恢复fpu寄存器
inline void fpu_restore( THREAD* thr )
{
	__asm__("frstor %0"::"m"(*thr->arch.fsave));
}

//在线程切换时，检查fpu是否有必要保存
void fpu_check_and_save( THREAD* thr )
{
	if(thr->arch.used_fpu){
		fpu_save(thr);
		thr->arch.used_fpu = 0;
		//捕捉浮点处理异常
		stts();
	}
}

// 注意，在处理过程中可能会发生硬件中断。
static int fpu_handler(int err_code, I386_REGISTERS* r)
{
	THREAD* thr;
	uint flags;
	thr =  current_thread();
	//是否数学协处理器已经有内容
	if(thr->used_math){
		//禁止切换线程
		local_irq_save(flags);
		//清ts，返回后可以使用fpu
		clts();
		//恢复
		fpu_restore(thr);
		thr->arch.used_fpu = 1;
		local_irq_restore(flags);
	}else{
		fpu_init_thread(thr);
		//禁止切换线程
		local_irq_save(flags);
		//then it would save fpu when schedule
		thr->arch.used_fpu = 1;
		//标记已初始化数学协处理器的内容
		thr->used_math = 1;
		clts();
		//init fpu and return
		__asm__ __volatile__("fninit");
		local_irq_restore(flags);
	}
	return 1;
}

//初始化fpu
void fpu_init()
{
	//安装浮点异常处理函数
	isr_install( FPU_INTERRUPT, (void*)fpu_handler );
	//设置ts位，使用浮点时触发异常。
	stts();
}
