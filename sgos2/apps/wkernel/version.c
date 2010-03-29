#include <sgos.h>
#include <api.h>
#include "debug.h"

typedef struct _OSVERSIONINFO {
  uint dwOSVersionInfoSize;
  uint dwMajorVersion;
  uint dwMinorVersion;
  uint dwBuildNumber;
  uint dwPlatformId;
  uchar szCSDVersion[128];
} OSVERSIONINFO;

void WINAPI GetVersion()
{
	NOT_IMPLEMENTED();
}


//Simulate a Windows XP
int WINAPI GetVersionExA( OSVERSIONINFO * info )
{
	if( info->dwOSVersionInfoSize < sizeof(OSVERSIONINFO) ){
		DBG("info buffer too small.");
		return 0;
	}
	info->dwMajorVersion = 5; 
	info->dwMinorVersion = 1;
	info->dwBuildNumber = 0;
	info->dwPlatformId = 2;
	strcpy( info->szCSDVersion, "Service Pack -1" );
	return sizeof(OSVERSIONINFO);
}
