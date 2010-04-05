#include <stdio.h>
#include <sgos.h>
#include <api.h>
#include <time.h>

#define BCD_TO_HEX(bcd)	((bcd)&0xf)+(((bcd)>>4)&0xf)*10

size_t mapAddress;
ThreadContext vmRegs={0};

int Vm86Call( void* c )
{
	return Api_InvokeBiosService( 0x1A, c, sizeof(ThreadContext) );
}

int GetBiosTime(int &h, int &m, int &s)
{
	vmRegs.eax = 0x0200;
	if( Vm86Call( &vmRegs ) < 0 )
		return -1;
	h = BCD_TO_HEX(vmRegs.ecx>>8);
	m = BCD_TO_HEX(vmRegs.ecx&0xff);
	s = BCD_TO_HEX(vmRegs.edx>>8);
	return 0;
}

int GetBiosDate(int &c, int &y, int &m, int &d)
{
	vmRegs.eax = 0x0400;
	if( Vm86Call( &vmRegs ) < 0 )
		return -1;
	c = BCD_TO_HEX(vmRegs.ecx>>8);
	y = BCD_TO_HEX(vmRegs.ecx&0xff);
	m = BCD_TO_HEX(vmRegs.edx>>8);
	d = BCD_TO_HEX(vmRegs.edx&0xff);
	return 0;
}

int GetDateTime( struct tm &t )
{
	int c, y, mo, d, h, m, s;
	GetBiosDate( c, y, mo, d );
	GetBiosTime( h, m, s );
	t.tm_sec = s;
	t.tm_min = m;
	t.tm_hour = h;
	t.tm_mday = d;
	t.tm_mon = mo;
	t.tm_year = y;
	t.tm_mon --;
	return 0;
}

time_t GetUnixTime()
{
	struct tm t;
	GetDateTime(t);
	return mktime(&t);
}


int InitializeBiosTime()
{
	uint sid = SysGetCurrentSpaceId();
	mapAddress = (size_t)SysAllocateMemory( sid, MB(1), 0, ALLOC_VIRTUAL );
	if( mapAddress==0 ){
		printf("[time]failed to allocate 1 mb memory.\n");
		return -ERR_NOMEM;
	}
	if( SysMapMemory( sid, mapAddress, MB(1), 0, 0, MAP_ADDRESS ) < 0 ){
		printf("[time]failed to map 1 mb memory.\n");
		return -ERR_NOMEM;
	}
	time_t unixTime = GetUnixTime();
	char timestr[100];
	strtime(&unixTime, timestr);
	printf("[time]System time: %s\n", timestr );
	return 0;
}

