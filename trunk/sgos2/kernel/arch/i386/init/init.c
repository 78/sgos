//Xiaoxia
//090803

#include <arch.h>
#include <multiboot.h>
#include <kd.h>
#include <ke.h>

//multiboot.S结束时调用这个函数
void ArSaveMultibootInformation( size_t info_addr )
{
	//
	ArLocalDisableIrq();
	//call KeStartOs, never return
	KeStartOs( info_addr );
}

//进一步设置x86保护模式
#define SET_SYSTEM_GATE( vector, handle ) ArSetGate( vector, DA_386IGate | DA_DPL3, handle )
extern int* SystemCallService;
int ArInitializeSystem()
{
	//init i386
	//重新设置gdt
	ArInitializeGdt();
	//设置isr，捕获各种机器异常
	ArInitializeIsr();
	//设置irq，捕获硬、软件中断
	ArInitializeIrq();
	//调试器
	ArInitializeDebugger();
	//i387数学协处理器
	ArInitializeMathProcessor();
	//vm86
	ArInitializeVm86();
	//设置软中断
	SET_SYSTEM_GATE( SYSTEM_INTERRUPT, &SystemCallService );
	//初始化fastcall
	ArInitializeFastcall();
	//Real time clock
	ArStartRealTimeClock();
	return 0;
}
