//real time clock
#include <sgos.h>
#include <arch.h>
#include <time.h>
#include <kd.h>
#include <tm.h>
#include <ke.h>

#define BCD_TO_HEX(bcd)	(bcd&0xf)+((bcd>>4)&0xf)*10

//global variables, they may be accessed by another file
unsigned rtc_time;      //unit: clock
unsigned rtc_millisecond; //unit: ms
unsigned rtc_second;    //unit: s

//设置时钟频率
//Set PIT Interrupt Frequency
void ArSetRealTimeClockFrequency(unsigned freq)
{
	unsigned div = 1193180/freq;
	ArOutByte(0x43, 0x36);
	ArOutByte(0x40, div & 0xFF);
	ArOutByte(0x40, div>>8);
}

unsigned ArGetMilliSecond()
{
	return rtc_millisecond;
}

//时钟中断
static void rtc_interrupt(const I386_REGISTERS* r)
{
	rtc_time ++;
	rtc_millisecond += 1000/RTC_FREQUENCY ;
	//屏蔽时钟中断
	ArSetIrqMask( RTC_INTERRUPT, 0 );
	//开启中断 
	ArLocalEnableIrq();
	//更新时间
	if( (rtc_time % RTC_FREQUENCY)==0 )
		rtc_second ++;
	//调度时钟
	TmIncreaseTime();
	//关中断
	ArLocalDisableIrq();
	//允许时钟中断
	ArSetIrqMask( RTC_INTERRUPT, 1 );
}

//实时钟初始化
void ArStartRealTimeClock()
{
	struct tm t;
	char timestr[48];

	/* initialize RTC */
	ArSetRealTimeClockFrequency( RTC_FREQUENCY );
	ArInstallIrq( RTC_INTERRUPT, rtc_interrupt);
	ArSetIrqMask( RTC_INTERRUPT, 1 );

	/* initialize system time */
	ArOutByte(0x70, 0);	/* seconds */
	t.tm_sec = BCD_TO_HEX(ArInByte(0x71));
	ArOutByte(0x70, 2);	/* minutes */
	t.tm_min = BCD_TO_HEX(ArInByte(0x71));
	ArOutByte(0x70, 4);	/* hours */
	t.tm_hour = BCD_TO_HEX(ArInByte(0x71));
	ArOutByte(0x70, 7);	/* date of month */
	t.tm_mday = BCD_TO_HEX(ArInByte(0x71));
	ArOutByte(0x70, 8);	/* month */
	t.tm_mon = BCD_TO_HEX(ArInByte(0x71));
	ArOutByte(0x70, 9);	/* year */
	t.tm_year = BCD_TO_HEX(ArInByte(0x71));
	t.tm_mon --;
	/* calculate to time calculated in ms from 1970-01-01 */
	rtc_second = mktime( &t );
	rtc_time = 0;
	rtc_millisecond = rtc_second * 1000;
	// Show time
	strtime( &rtc_second, timestr );
	KdPrintf("System start time: %s\n", timestr );
}

