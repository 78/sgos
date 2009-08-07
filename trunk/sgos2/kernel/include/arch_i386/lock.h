#ifndef _LOCK_H_
#define _LOCK_H_

#define local_irq_disable() __asm__ __volatile__("cli")

#define local_irq_enable() __asm__ __volatile__("sti")

#define local_irq_save( x ) __asm__ __volatile__ \
	("pushfl ; popl %0 ; cli":"=g" (x): :"memory")

#define local_irq_restore(x)    __asm__ __volatile__ \
	("pushl %0 ; popfl" : :"g" (x):"memory")



#endif

