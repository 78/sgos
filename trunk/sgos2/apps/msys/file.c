#include <sgos.h>
#include <api.h>
#include "debug.h"

void close( int fd )
{
	FsCloseFile( (FILEBUF*)fd );
}

long lseek( int fd, long offset, int fromwhere)
{
	return FsSetFilePointer( (FILEBUF*)fd, offset, fromwhere );
}

int open( const char* path, int oflag, int mode )
{
	FILEBUF* fb = FsOpenFile( path, oflag, mode );
	if( fb )
		return (int)fb;
	return -1;
}


int write(int fd, void *buf, int nbyte)
{
	return FsWriteFile( (FILEBUF*)fd, buf, nbyte );
}

int read(int fd, void *buf, int nbyte)
{
	return FsReadFile( (FILEBUF*)fd, buf, nbyte );
}

