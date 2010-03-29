// time structures
#ifndef _TIME_H__
#define _TIME_H__

#include <sgos.h>

struct tms {
	clock_t tms_utime; /* user time */
	clock_t tms_stime; /* system time */
	clock_t tms_cutime; /* user time of children */
	clock_t tms_cstime; /* system time of children */
};

struct timeval{
	long tv_sec; /*秒*/
	long tv_usec; /*微秒*/
};

struct timezone{
	int tz_minuteswest; 	/*和Greenwich 时间差了多少分钟*/
	int tz_dsttime; 	/*日光节约时间的状态*/
};

//! the old time struct
#ifndef _TM_DEFINED
typedef struct tm
{
	int tm_sec;     //
	int tm_min;     //
	int tm_hour;    //
	int tm_mday;    //day of the month
	int tm_mon;	    //month of the year
	int tm_year;    //
	int tm_wday;    //day of the week?  0~6
	int tm_yday;    //day of the year
	int tm_isdst;   //is leap year?
}TIME;
#define _TM_DEFINED
#endif  //_TM_DEFINED

//! ...
EXTERN time_t time(time_t* );
//! Get a format string by a time,
//! The function returns the length of the string, 0 means failed, others > 0 means success
//! Note: the str buffer must be enough big, normally 48 bytes
EXTERN int strtime( time_t*, char* str );
//! From struct tm to time_t
EXTERN time_t mktime( const struct tm* );
//! From time_t to struct tm
//! If failed, it will return 0, others > 0 means success
EXTERN int gettime( const time_t*, struct tm* );

#endif

