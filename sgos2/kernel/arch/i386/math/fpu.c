// Hope it works!!
// 090828 by Huang Guan
#include <arch.h>
#include <tm.h>
#include <kd.h>
#include <mm.h>
#include <rtl.h>

//为线程分配fpu
void ArPrepareFpuForThread( KThread* thr )
{
	ArchThread* arch;
	arch = &thr->ArchitectureInformation;
	if( !arch->fsave ){
		arch->fsave = (I387_REGISTERS*)MmAllocateKernelMemory( sizeof(I387_REGISTERS) );
		RtlZeroMemory32( arch->fsave, sizeof(I387_REGISTERS)>>2 );
	}
}

//保存fpu寄存器
inline void fpu_save( KThread* thr )
{
	__asm__("fnsave %0; fwait"::"m"(*thr->ArchitectureInformation.fsave));
}

//恢复fpu寄存器
inline void fpu_restore( KThread* thr )
{
	__asm__("frstor %0"::"m"(*thr->ArchitectureInformation.fsave));
}

//在线程切换时，检查fpu是否有必要保存
void ArCheckAndSaveFpu( KThread* thr )
{
	if(thr->ArchitectureInformation.used_fpu){
		fpu_save(thr);
		thr->ArchitectureInformation.used_fpu = 0;
		//捕捉浮点处理异常
		stts();
	}
}

// 注意，在处理过程中可能会发生硬件中断。
static int fpu_handler(int err_code, I386_REGISTERS* r)
{
	KThread* thr;
	uint flags;
	thr =  TmGetCurrentThread();
	//是否数学协处理器已经有内容
	if(thr->UsedMathProcessor){
		//禁止切换线程
		ArLocalSaveIrq(flags);
		//清ts，返回后可以使用fpu
		clts();
		//恢复
		fpu_restore(thr);
		thr->ArchitectureInformation.used_fpu = 1;
		ArLocalRestoreIrq(flags);
	}else{
		ArPrepareFpuForThread(thr);
		//禁止切换线程
		ArLocalSaveIrq(flags);
		//then it would save fpu when TmSchedule
		thr->ArchitectureInformation.used_fpu = 1;
		//标记已初始化数学协处理器的内容
		thr->UsedMathProcessor = 1;
		clts();
		//init fpu and return
		__asm__ __volatile__("fninit");
		ArLocalRestoreIrq(flags);
	}
	return 1;
}

//初始化fpu
void ArInitializeMathProcessor()
{
	//安装浮点异常处理函数
	ArInstallIsr( FPU_INTERRUPT, (void*)fpu_handler );
	//设置ts位，使用浮点时触发异常。
	stts();
}
