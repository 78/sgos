#ifndef _LOCK_H_
#define _LOCK_H_

#define ArLocalDisableIrq() __asm__ __volatile__("cli")

#define ArLocalEnableIrq() __asm__ __volatile__("sti")

#define ArLocalSaveIrq( x ) __asm__ __volatile__ \
	("pushfl ; popl %0 ; cli":"=g" (x): :"memory")

#define ArLocalRestoreIrq(x)    __asm__ __volatile__ \
	("pushl %0 ; popfl" : :"g" (x):"memory")



#endif

