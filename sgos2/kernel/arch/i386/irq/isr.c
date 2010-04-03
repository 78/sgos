
#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <tm.h>
#include <mm.h>
#include <rtl.h>

static char *exception_msg[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};


extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

static void *isr_routines[32] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// install a isr handler
int ArInstallIsr( int isr, void (*handler)(int err_code, const I386_REGISTERS *r) )
{
	if( isr_routines[isr] ){
		PERROR("ISR: %d is already installed.", isr );
		return -1;
	}
	isr_routines[isr] = handler;
	return 0;
}


void ArUninstallIsr( int isr )
{
	isr_routines[isr] = NULL;
}


void ArInitializeIsr()
{
	RtlZeroMemory( isr_routines, sizeof(isr_routines) );
	/* ISR 0x0~0x1F */
	SET_TRAP_GATE(0, (void*)isr0);
	SET_TRAP_GATE(1, (void*)isr1);
	SET_TRAP_GATE(2, (void*)isr2);
	SET_TRAP_GATE(3, (void*)isr3);
	SET_TRAP_GATE(4, (void*)isr4);
	SET_TRAP_GATE(5, (void*)isr5);
	SET_TRAP_GATE(6, (void*)isr6);
	SET_TRAP_GATE(7, (void*)isr7);

	SET_TRAP_GATE(8, (void*)isr8);
	SET_TRAP_GATE(9, (void*)isr9);
	SET_TRAP_GATE(10, (void*)isr10);
	SET_TRAP_GATE(11, (void*)isr11);
	SET_TRAP_GATE(12, (void*)isr12);
	SET_TRAP_GATE(13, (void*)isr13);
	
	//注意，页面异常处理时，在保存cr2之前禁止中断。
	//也可以在中断时检查线程是否发生了页面异常。
	SET_INTR_GATE(14, (void*)isr14);
	
	SET_TRAP_GATE(15, (void*)isr15);
	SET_TRAP_GATE(16, (void*)isr16);
	SET_TRAP_GATE(17, (void*)isr17);
	SET_TRAP_GATE(18, (void*)isr18);
	SET_TRAP_GATE(19, (void*)isr19);
	SET_TRAP_GATE(20, (void*)isr20);
	SET_TRAP_GATE(21, (void*)isr21);
	SET_TRAP_GATE(22, (void*)isr22);
	SET_TRAP_GATE(23, (void*)isr23);

	SET_TRAP_GATE(24, (void*)isr24);
	SET_TRAP_GATE(25, (void*)isr25);
	SET_TRAP_GATE(26, (void*)isr26);
	SET_TRAP_GATE(27, (void*)isr27);
	SET_TRAP_GATE(28, (void*)isr28);
	SET_TRAP_GATE(29, (void*)isr29);
	SET_TRAP_GATE(30, (void*)isr30);
	SET_TRAP_GATE(31, (void*)isr31);
	
}

//异常处理入口。
void ArHandleIsr(const I386_REGISTERS *r)
{
	int (*handler)(int err_code, const I386_REGISTERS *r);
	if ( r->int_no < 32 )
	{
		handler = isr_routines[r->int_no];
		if (!handler ||
			!handler(r->err_code, r) ){
			KThread* cur = TmGetCurrentThread();
			if( cur && !cur->IsKernelThread )
				TmSleepThread( cur, 1000 );
			KdPrintf("## Unhandled Exception ##\t"
				"%s\tCode: %d\tcr3:%x\n", 
				exception_msg[ r->int_no ], r->err_code, MmGetCurrentSpace()->PageDirectory.PhysicalAddress );
			ArDumpRegisters( r );
			KERROR("Termiating %s[%x:%x]", "Thread", MmGetCurrentSpace()->SpaceId, TmGetCurrentThread()->ThreadId );
		}
	}
}

