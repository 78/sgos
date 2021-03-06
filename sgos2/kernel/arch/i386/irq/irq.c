// ported from SGOS1

#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <tm.h>
#include <rtl.h>
#include <ke.h>

//中断捕获函数
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

//中断处理函数数组
static void *irq_routines[16] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/* This installs a custom IRQ handler for the given IRQ */
int ArInstallIrq( int irq, void (*handler)() )
{
	if( irq_routines[irq] ){
		PERROR("IRQ: %d is already installed.", irq );
		return -1;
	}
	irq_routines[irq] = handler;
	return 0;
}

/* This clears the handler for a given IRQ */
void ArUninstallIrq( int irq )
{
	irq_routines[irq] = NULL;
}

static void irq_remap(void)
{
	// 给中断寄存器编程
	// 发送 ICW1 : 使用 ICW4，级联工作
	ArOutByte( 0x20 , 0x11 ) ;
	ArOutByte( 0xA0 , 0x11 ) ;
	// 发送 ICW2，中断起始号从 0x20 开始（第一片）及 0x28开始（第二片）
	ArOutByte( 0x21 , 0x20 ) ;
	ArOutByte( 0xA1 , 0x28 ) ;
	// 发送 ICW3
	ArOutByte( 0x21 , 0x4 ) ;
	ArOutByte( 0xA1 , 0x2 ) ;
	// 发送 ICW4
	ArOutByte( 0x21 , 0x1 ) ;
	ArOutByte( 0xA1 , 0x1 ) ;
	// 设置中断屏蔽位 OCW1 ，屏蔽所有中断请求
	// 由于主片的 IRQ2 与从边相连,故不屏蔽它
	ArOutByte( 0x21 , 0xFB ) ;
	ArOutByte( 0xA1 , 0xFF ) ;
}

// 设置IRQ屏蔽位图
void ArSetIrqMask( int irq, int enabled )
{
	t_8 mask_word;
	if (irq <0 || irq > 15 ) 
		return;
	if( irq < 8 )
		mask_word = ArInByte( 0x21 ) ;
	else
		mask_word = ArInByte( 0xA1 ) ;
	switch( irq )
	{
	case 1:
	case 9:
		if( enabled )
			mask_word &= 0xFD ; // 0xFD = 1111 1101
		else
			mask_word |= 0x02 ; // 0x01 = 0000 0010
	break ;
	case 0:
	case 8:
		if( enabled )
			mask_word &= 0xFE ; // 0xFE = 1111 1110
		else
			mask_word |= 0x01 ; // 0x00 = 0000 0001
		break ;
	case 2 :
	case 10:
		if( enabled )
			mask_word &= 0xFB ; // 0xFB = 1111 1011
		else
			mask_word |= 0x04 ; // 0x04 = 0000 0100
	break ;
	case 3 :
	case 11:
		if( enabled )
			mask_word &= 0xF7 ; // 0xF7 = 1111 0111
		else
			mask_word |= 0x08 ; // 0x08 = 0000 1000
	break ;
	case 4 :
	case 12:
		if( enabled )
			mask_word &= 0xef ; // 0xEF = 1110 1111
		else
			mask_word |= 0x10 ; // 0x10 = 0001 0000
	break ;
	case 5 :
	case 13:
		if( enabled )
			mask_word &= 0xdf ; // 0xDF = 1101 1111
		else
			mask_word |= 0x20 ; // 0x20 = 0010 0000
	break ;
	case 6 :
	case 14:
	if( enabled )
			mask_word &= 0xBF ; // 0xBF = 1011 1111
		else
			mask_word |= 0x40 ; // 0x40 = 0100 0000
	break ;
	case 7 :
	case 15:
		if( enabled )
			mask_word &= 0x7F ; // 0x7f = 0111 1111
		else
			mask_word |= 0x80 ; // 0x80 = 1000 0000
	break ;
	}
	if( irq < 8 )
	{
		ArOutByte( 0x21, mask_word );
	}else{
		ArOutByte( 0xA1, mask_word );
	}
}

//irq处理(关中断情况下处理)
//返回1，表示线程切换。
int ArHandleIrq(const I386_REGISTERS *r)
{
	void (*handler)(const I386_REGISTERS *r);
	//设置irq模式
	TmGetCurrentThread()->IsInterrupted = 1;
	//获取处理函数句柄
	handler = irq_routines[r->int_no - 32];
	//调用特定处理函数。
	if( handler )
		handler(r);
	//Call kernel to handle it.
	KeHardwareInterruptMessage( r->int_no - 32 );
	//清irq模式
	TmGetCurrentThread()->IsInterrupted = 0;
	//处理完毕! 发送EOI
	if (r->int_no >= 40)
		ArOutByte( 0xA0, 0x20 );
	ArOutByte( 0x20, 0x20 );
	//是否线程切换
	if( ThreadingBox.next ){
		//更新线程环境，如是否需要重装页目录
		ArPrepareForNextThread();
		return 1;
	}
	return 0;
}

//硬件中断初始化
void ArInitializeIrq()
{
	RtlZeroMemory( irq_routines, sizeof(irq_routines) );
	//重新映射irq
	irq_remap();
	// IDT初始化
	SET_INTR_GATE(32, (void*)irq0);
	SET_INTR_GATE(33, (void*)irq1);
	SET_INTR_GATE(34, (void*)irq2);
	SET_INTR_GATE(35, (void*)irq3);
	SET_INTR_GATE(36, (void*)irq4);
	SET_INTR_GATE(37, (void*)irq5);
	SET_INTR_GATE(38, (void*)irq6);
	SET_INTR_GATE(39, (void*)irq7);
	SET_INTR_GATE(40, (void*)irq8);
	SET_INTR_GATE(41, (void*)irq9);
	SET_INTR_GATE(42, (void*)irq10);
	SET_INTR_GATE(43, (void*)irq11);
	SET_INTR_GATE(44, (void*)irq12);
	SET_INTR_GATE(45, (void*)irq13);
	SET_INTR_GATE(46, (void*)irq14);
	SET_INTR_GATE(47, (void*)irq15);
}
