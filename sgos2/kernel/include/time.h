// time structures
#ifndef __TIME_H__
#define __TIME_H__

#include <sgos.h>

//! the old time struct
#ifndef _TM_DEFINED
struct tm
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
};
#define _TM_DEFINED
#endif  //_TM_DEFINED

//! ...
time_t time(time_t* );
//! Get a format string by a time,
//! The function returns the length of the string, 0 means failed, others > 0 means success
//! Note: the str buffer must be enough big, normally 48 bytes
int strtime( time_t*, char* str );
//! From struct tm to time_t
time_t mktime( const struct tm* );
//! From time_t to struct tm
//! If failed, it will return 0, others > 0 means success
int gettime( const time_t*, struct tm* );

#endif

