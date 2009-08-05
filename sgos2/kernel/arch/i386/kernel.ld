/* Link script for SGOS2 */

ENTRY(_start)

K_VIR_ADDR = 0xC0100000;
K_PHY_ADDR = 0x100000;
PAGE_SIZE    = 0x1000;
/* 8KB stack size */
K_STACK_SIZE   = 0x1000;

SECTIONS
{
	.text K_VIR_ADDR : AT(K_PHY_ADDR)
	{
		INIT_BEG = . ;
		*(.init)
		. = ALIGN(PAGE_SIZE);
		INIT_END = . ;
		*(.text)
		*(.rodata.*)
		. = ALIGN(PAGE_SIZE);
	}
	.data : AT(K_PHY_ADDR + (DATA_BEG - INIT_BEG) )
	{
		DATA_BEG = . ;
		*(.data)
		*(.rodata)
		. = ALIGN(PAGE_SIZE);
	}
	.bss : AT(K_PHY_ADDR + (BSS_BEG - INIT_BEG))
	{
		BSS_BEG = . ;
		*(.bss)
		. = ALIGN(PAGE_SIZE);
	}
	.stack : AT(K_PHY_ADDR +(_stack_end - INIT_BEG))
	{
		_stack_end = . ;
		. += K_STACK_SIZE;
		_stack_start = . ;
		K_STACK_ADDR = . ;
		. = ALIGN( PAGE_SIZE );
	}
	KERNEL_END = . ;
	. = ALIGN(PAGE_SIZE);
}
