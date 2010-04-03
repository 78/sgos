#include <sgos.h>
#include <api.h>
#include "debug.h"

#define	_FREAD		0x0001	/* read enabled */
#define	_FWRITE		0x0002	/* write enabled */
#define	_FAPPEND	0x0008	/* append (writes guaranteed at the end) */
#define	_FCREAT		0x0200	/* open with file create */
#define	_FTRUNC		0x0400	/* open with truncation */
#define	_FEXCL		0x0800	/* error on open if file exists */

#define	O_RDONLY	0		/* +1 == FREAD */
#define	O_WRONLY	1		/* +1 == FWRITE */
#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	O_APPEND	_FAPPEND
#define	O_CREAT		_FCREAT
#define	O_TRUNC		_FTRUNC
#define	O_EXCL		_FEXCL
void close( int fd )
{
	DBG("close %d", fd );
	FsCloseFile( (FILEBUF*)fd );
}

long lseek( int fd, long offset, int fromwhere)
{
	DBG("lseek %x offset:%d", fd, offset );
	return FsSetFilePointer( (FILEBUF*)fd, offset, fromwhere );
}

int open( const char* path, int oflag, int mode )
{
	int myflag=0, mymode=0;
	DBG("open %s oflag:%x", path, oflag );
	if( oflag& _FREAD )
		mymode |= FILE_READ;
	if( oflag& _FWRITE )
		mymode |= FILE_WRITE;
	if( oflag& _FAPPEND )
		myflag |= FILE_FLAG_APPEND;
	if( oflag& _FCREAT )
		myflag |= FILE_FLAG_CREATE;

	FILEBUF* fb = FsOpenFile( path, myflag, mymode );
	if( fb )
		return (int)fb;
	return -1;
}


int _write(int fd, void *buf, int nbyte)
{
	DBG("_write %x %d", fd, nbyte );
	if( fd==STDOUT )
		return Api_Print( buf, nbyte );
	return FsWriteFile( (FILEBUF*)fd, buf, nbyte );
}

int write(int fd, void *buf, int nbyte)
{
	return _write( fd, buf, nbyte );
}

int read(int fd, void *buf, int nbyte)
{
	return _read( fd, buf, nbyte );
}

int _read(int fd, void *buf, int nbyte)
{
	DBG("read bytes: %d", nbyte );
	return FsReadFile( (FILEBUF*)fd, buf, nbyte );
}

