//sgos.h
/*
 *  SGOS Definitions
 *
 */

#ifndef __SGOS__H__
#define __SGOS__H__

#include <types.h>


#define RTC_FREQUENCY	1000	//1000Hz  这是时钟频率，具体在arch/i386/clock/rtc.c
#ifndef NULL
#define NULL		((void*)0)	//
#endif
#define FILE_NAME_LEN	128

#endif //__SGOS__H__
