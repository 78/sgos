//real time clock
#include <sgos.h>
#include <arch.h>
#include <time.h>
#include <debug.h>
#include <thread.h>

#define BCD_TO_HEX(bcd)	(bcd&0xf)+((bcd>>4)&0xf)*10

//global variables, they may be accessed by another file
unsigned rtc_time;      //unit: ms
unsigned rtc_second;    //unit: s

//设置时钟频率
void rtc_set_freq(unsigned freq)
{
	unsigned div = 1193180/freq;
	out_byte(0x43, 0x36);
	out_byte(0x40, div & 0xFF);
	out_byte(0x40, div>>8);
}

//时钟中断
void rtc_interrupt(const I386_REGISTERS* r)
{
	rtc_time ++;
	if( !(rtc_time % RTC_FREQUENCY) ){
		rtc_second ++;
	}
	sched_clock();
}

//实时钟初始化
void rtc_init()
{
	struct tm t;
	char timestr[48];

	/* initialize RTC */
	rtc_set_freq( RTC_FREQUENCY );
	irq_install( RTC_INTERRUPT, rtc_interrupt);
	irq_mask( RTC_INTERRUPT, 1 );

	/* initialize system time */
	out_byte(0x70, 0);	/* seconds */
	t.tm_sec = BCD_TO_HEX(in_byte(0x71));
	out_byte(0x70, 2);	/* minutes */
	t.tm_min = BCD_TO_HEX(in_byte(0x71));
	out_byte(0x70, 4);	/* hours */
	t.tm_hour = BCD_TO_HEX(in_byte(0x71));
	out_byte(0x70, 7);	/* date of month */
	t.tm_mday = BCD_TO_HEX(in_byte(0x71));
	out_byte(0x70, 8);	/* month */
	t.tm_mon = BCD_TO_HEX(in_byte(0x71));
	out_byte(0x70, 9);	/* year */
	t.tm_year = BCD_TO_HEX(in_byte(0x71));
	t.tm_mon --;
	/* calculate to time calculated in ms from 1970-01-01 */
	rtc_second = mktime( &t );
	rtc_time = rtc_second * 1000;
	// Show time
	strtime( &rtc_second, timestr );
	kprintf("System start time: %s\n", timestr );
}

