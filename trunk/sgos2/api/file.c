#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <stdlib.h>

#define FILE_BUFSIZE_MAX	KB(64)
#ifndef MIN
#define MIN(a,b) (a>b?b:a)
#define MAX(a,b) (a>b?a:b)
#endif

FILEBUF* FsOpenFile( const char* fname, uint mode, uint flag )
{
	FILEBUF* fb;
	uint sid = SysGetCurrentSpaceId();
	fb = (FILEBUF*)malloc( sizeof(FILEBUF) );
	if( fb == NULL )
		return fb;
	Message msg;
	while( (msg.ThreadId = SmGetServiceThreadById(FileSystemId) ) == 0 )
		;
	msg.Command = File_Open;
	msg.Arguments[0] = mode;
	msg.Arguments[1] = flag;
	msg.Large[0] = (size_t)SysAllocateMemory( sid, PAGE_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
	strncpy( (char*)msg.Large[0], fname, PATH_LEN );
	int ret;
	ret = Api_Send(&msg, 0);
	ret = Api_Receive(&msg, 3000);
	if( ret >= 0 && msg.Code==0 ){
		//reuse the memory
		fb->bufsize = PAGE_SIZE;
		fb->bufptr = (void*)msg.Large[0];
		fb->bufpos = 0;
		fb->curpos = 0;
		fb->fd = msg.Arguments[0];
		fb->filesize = msg.Arguments[1];
		fb->attr = msg.Arguments[2];
		fb->ctime = msg.Arguments[3];
		fb->mtime = msg.Arguments[4];
		fb->flag = flag | FILE_FLAG_NOBUF;
		fb->mode = mode;
		return fb;
	}
	//failed.
	SysFreeMemory( sid, (void*)msg.Large[0] );
	free(fb);
	return (FILEBUF*)0;
}

void FsCloseFile( FILEBUF* fb )
{
	if( fb == NULL )
		return;
	if( fb->bufptr ){
		SysFreeMemory( SysGetCurrentSpaceId(), fb->bufptr );
		fb->bufptr = 0;
		fb->bufsize = 0;
	}
	free( fb );
	//tell file server to release the file id.
	int fd = fb->fd;
	SendMessage( 
		SmGetServiceThreadById(FileSystemId), File_Close,
		&fd, NULL, NULL, NULL, NULL );
}

static int ReadWriteBuffer( FILEBUF* fb, int count, uint cmd )
{
	Message msg;
	int ret;
	msg.ThreadId = SmGetServiceThreadById(FileSystemId);
	msg.Command = cmd;
	msg.Arguments[0] = fb->fd;
	msg.Arguments[1] = count;
	msg.Arguments[2] = fb->bufsize;
	msg.Large[0] = (size_t)fb->bufptr;
	ret = Api_Send( &msg, 0 );
	ret = Api_Receive( &msg, 3000 );
	if( ret < 0 )
		return ret;
	return msg.Code; 
}


int FsReadFile( FILEBUF* fb, uchar* buf, int count )
{
	if( fb->flag & FILE_FLAG_NOBUF ){
		int bytesRead = 0, read=0, ret;
		while( bytesRead < count ){
			read = count - bytesRead;
			if( read > PAGE_SIZE )
				read = PAGE_SIZE;
			ret = ReadWriteBuffer( fb, read, File_Read );
			if( ret <= 0 )
				return bytesRead;
			memcpy( buf, (void*)fb->bufptr, ret );
			buf += ret;
			fb->curpos += ret;
			bytesRead += ret;
		}
		return bytesRead;
	}else{
		printf("FsReadFile ##error: file buffering is not implemented.\n");
	}
	return -ERR_NOIMP;
}

int FsWriteFile( FILEBUF* fb, uchar* buf, int count )
{
	if( fb->flag & FILE_FLAG_NOBUF ){
		int bytesWritten = 0, write=0, ret;
		while( bytesWritten < count ){
			write = count - bytesWritten;
			if( write > PAGE_SIZE )
				write = PAGE_SIZE;
			memcpy( (void*)fb->bufptr, buf, write );
			ret = ReadWriteBuffer( fb, write, File_Write );
			if( ret <= 0 )
				return bytesWritten;
			buf += ret;
			fb->curpos += ret;
			bytesWritten += ret;
		}
		return bytesWritten;
	}else{
		printf("FsReadFile ##error: file buffering is not implemented.\n");
	}
	return -ERR_NOIMP;
}

int FsSetFilePointer( FILEBUF* fb, int pos, int method )
{
	Message msg;
	int ret = 0, id = fb->fd;
	SendMessage( SmGetServiceThreadById(FileSystemId), 
		File_Seek, &id, &pos, &method, NULL, &ret );
	return ret;
}

int FsSetFileSize( FILEBUF* fb, size_t newsize )
{
	Message msg;
	int ret = 0, id = fb->fd;
	SendMessage( SmGetServiceThreadById(FileSystemId), 
		File_SetSize, &id, &newsize, NULL, NULL, &ret );
	return ret;
}

int FsControlFile( FILEBUF* fb, uint cmd, uint arg )
{
	Message msg;
	int ret = 0, id = fb->fd;
	SendMessage( SmGetServiceThreadById(FileSystemId), 
		File_Control, &id, &cmd, &arg, NULL, &ret );
	return ret;
}

int FsReadDirectory( FILEBUF* fb, DIRENTRY* buf, int count )
{
}


