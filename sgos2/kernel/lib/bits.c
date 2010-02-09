// bits.c

#include <sgos.h>
#include <arch.h>

void * RtlCopyMemory(void * dest, const void * src, int n)
{
	return memcpy8( dest, src, n );
}

void * RtlCopyMemory16(void * dest, const void * src, int n)
{
	return memcpy16( dest, src, n );
}

void * RtlCopyMemory32(void * dest, const void * src, int n)
{
	return memcpy32( dest, src, n );
}

void * RtlZeroMemory(void * s, int count)
{
	return memset8( s, 0, count );
}

void * RtlZeroMemory16(void * s, int count)
{
	return memset16( s, 0, count );
}

void * RtlZeroMemory32(void * s, int count)
{
	return memset32( s, 0, count );
}

