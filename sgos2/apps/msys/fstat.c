#include <sgos.h>
#include <stdio.h>
#include <api.h>
#include "debug.h"

int fstat(int fd, struct _stat *buf)
{
	FILEBUF* fb;
	DBG("fd=%d", fd );
	memset( buf, 0, sizeof(*buf) );
	if( fd == STDOUT )
		return 0;
	fb = (FILEBUF*) fd;
	
	buf->st_mode |= S_IRWXU; // for fat system
	buf->st_mode |= _IFMT; //is a file.
	if( fb->attr & FILE_ATTR_DIR )
		buf->st_mode |= _IFDIR; //is a directory
	else
		buf->st_mode |= _IFREG; //regular file.
	buf->st_mode |= _IFBLK | _IFCHR; //char dev & block dev
	
	buf->st_ino = fd;
	buf->st_dev = 0;
	buf->st_rdev = 0;
	buf->st_nlink = 1;
	buf->st_uid = 0;
	buf->st_gid = 0;
	buf->st_size = fb->filesize;
	buf->st_blksize = PAGE_SIZE;
	buf->st_blocks = buf->st_size%buf->st_blksize==0 ? buf->st_size/buf->st_blksize : buf->st_size/buf->st_blksize+1;
	
	buf->st_ctime = fb->ctime;
	buf->st_mtime = fb->mtime;
	buf->st_atime = fb->mtime;
	
	return 0;
}
