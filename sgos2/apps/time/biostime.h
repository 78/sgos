#ifndef _BIOSTIME_H
#define _BIOSTIME_H

#include <time.h>

#define Time_GetTimeString	0x00000101
#define Time_SetLocale		0x00000102
#define Time_GetLocale		0x00000103
#define Time_GetUnixTime	0x00000104
#define Time_GetTimeOfDay	0x00000105
#define Time_SetTimeOfDay	0x00000106


int InitializeBiosTime();
time_t GetUnixTime();
int GetDateTime( struct tm &t );

#endif
