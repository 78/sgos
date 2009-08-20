// time

#include <sgos.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

extern unsigned rtc_second;

#define SECS_PER_MIN 60
#define SECS_PER_HOUR (60 * 60)
#define SECS_PER_DAY (SECS_PER_HOUR * 24)
#define SECS_PER_YEAR (365*SECS_PER_DAY)

static const ushort _mon_yday[2][13] =
{
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};
static
int __offtime (const time_t *t, long int offset, struct tm *tp);
#define _isleap(year) ( (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0) )

time_t time(time_t *tloc)
{
	if(tloc != 0) {
		*tloc = rtc_second;
	}
	return rtc_second;
}

time_t mktime( const struct tm *t )
{
    short month, year;
	time_t res;
	month = t->tm_mon;
	if( t->tm_year<70 )  //2000 Problem!!
        year = t->tm_year + month / 12 + 2000;
    else
        year = t->tm_year + month / 12 + 1900;
	month %= 12;
	if (month < 0)
	{
		year -= 1;
		month += 12;
	}
	res = (year - 1970) * 365 + (year - 1969) / 4 + _mon_yday[0][month];
	res = (year - 1970) * 365 + _mon_yday[0][month];
	if (month <= 1)
		year -= 1;
	res += (year - 1968) / 4;
	res -= (year - 1900) / 100;
	res += (year - 1600) / 400;
	res += t->tm_mday;
	res -= 1;
	res *= 24;
	res += t->tm_hour;
	res *= 60;
	res += t->tm_min;
	res *= 60;
	res += t->tm_sec;
	return(res);
}

int gettime( const time_t *t, struct tm* tp )
{
    return __offtime( t, 0, tp );
}

//! ANSI  __offtime
int __offtime (const time_t *t, long int offset, struct tm *tp)
{
    long int days, rem, y;
    const unsigned short int *ip;

    days = *t / SECS_PER_DAY;
    rem = *t % SECS_PER_DAY;
    rem += offset;
    while (rem < 0)
    {
        rem += SECS_PER_DAY;
        --days;
    }
    while (rem >= SECS_PER_DAY)
    {
        rem -= SECS_PER_DAY;
        ++days;
    }
    tp->tm_hour = rem / SECS_PER_HOUR;
    rem %= SECS_PER_HOUR;
    tp->tm_min = rem / 60;
    tp->tm_sec = rem % 60;
    /* January 1, 1970 was a Thursday. */
    tp->tm_wday = (4 + days) % 7;
    if (tp->tm_wday < 0)
        tp->tm_wday += 7;
    y = 1970;

    #define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
    #define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

    while (days < 0 || days >= (_isleap (y) ? 366 : 365))
    {
        /* Guess a corrected year, assuming 365 days per year. */
        long int yg = y + days / 365 - (days % 365 < 0);
        /* Adjust DAYS and Y to match the guessed year. */
        days -= ((yg - y) * 365
                + LEAPS_THRU_END_OF (yg - 1)
                - LEAPS_THRU_END_OF (y - 1) );
        y = yg;
    }
    tp->tm_year = y - 1900;
    if (tp->tm_year != y - 1900){
        return 0;
    }
    tp->tm_yday = days;
    tp->tm_isdst = _isleap(y);
    ip = _mon_yday[tp->tm_isdst];
    for (y = 11; days < (long int) ip[y]; --y)
        continue;
    days -= ip[y];
    tp->tm_mon = y;
    tp->tm_mday = days + 1;
    return 1;
}


int strtime( time_t *t, char* str )
{
    struct tm m;
    int len;
    if( !gettime( t, &m ) )
    {
        return 0;
    }
    len = sprintf( str, "%d-%d-%d %.2d:%.2d:%.2d", m.tm_year+1900,
		m.tm_mon+1, m.tm_mday, m.tm_hour, m.tm_min, m.tm_sec);
    return len;
}
