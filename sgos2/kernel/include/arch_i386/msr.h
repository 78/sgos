#ifndef _MSR_H_
#define _MSR_H_

#define rdmsr(msr,val1,val2) \
	__asm__ __volatile__("rdmsr" \
	: "=a" (val1), "=d" (val2) : "c" (msr))

#define wrmsr(msr,val1,val2) \
	__asm__ __volatile__("wrmsr" : : "c" (msr), "a" (val1), "d" (val2))

#define MSR_IA32_SYSENTER_CS	0x174	//CS Selector of the target segment
#define MSR_IA32_SYSENTER_ESP	0x175	//Target ESP
#define MSR_IA32_SYSENTER_EIP	0x176	//Target EIP

#endif //_MSR_H_
