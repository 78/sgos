/*
 *   Virtual File System
 *
 *   Author: Huang Guan  090830
 *
 */


#include <sgos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <api.h>
#include "buffer.h"
#include "fsservice.h"

//vfs服务线程
uint vfsThread;
//根目录文件
file_t* root;
device_t* rootDev;

#define MAX_FILE 65536
static file_t* fileList;
static int fileCount = 0;

file_t* falloc()
{
	int id;
	file_t* f;
	if( fileCount >= MAX_FILE ){
		printf("[vfs]##too many files.\n");
		return 0;
	}
	for(id=0; id<MAX_FILE; id++ )
		if( fileList[id].reference==0 )
			break;
	if( id==MAX_FILE )
		return 0;
	++ fileCount;
	f = &fileList[id];
	memset( f, 0, sizeof(file_t) );
	++ f->reference;
	return f;
}

void flinkto( file_t* f, file_t* parent)
{
	//add to tree
	if( parent ){
		f->parent = parent;
		if( parent->child )
			parent->child->prev = f;
		f->next = parent->child;
		f->prev = 0;
		parent->child = f;
	}
}

void funlink( file_t* f )
{
	if( f->prev )
		f->prev->next = f->next;
	else if( f->parent )
		f->parent->child = f->next;
	if( f->next )
		f->next->prev = f->prev;
	f->prev = f->next = f->parent = 0;
}

int fgetid(file_t * f)
{
	if( f==0 )
		return -ERR_INVALID;
	int id = ((uint)f-(uint)fileList) / sizeof(file_t);
	return id;
}

file_t* fgetfile(int id)
{
	if( id>=0 && id<MAX_FILE ){
		if(fileList[id].reference==0)
			return 0;
		return &fileList[id];
	}
	return 0;
}

void ffree(file_t* f)
{
	if( !f )
		return;
	if( f->reference == 0 ){
		printf("[vfs]## trying to free free file %d:%s.\n", fgetid(f), f->name);
		SysExitThread(0);
	}
	-- f->reference;
	if( f->parent )
		ffree( f->parent );
	if( f->reference == 0 ){
		if( f->child )
			printf("[vfs]## f->child!=NULL. But f is removed.\n");
		//remove it from parent
		funlink( f );
		//free id
		int id = fgetid( f );
		fileList[id].reference=0;
	}
}

// 尝试打开一级目录
static int try_open( file_t* cur, file_t** next, const char* name, uint flag )
{
	file_t* f;
	if( !cur->dir && !(cur->attribute&FILE_ATTR_DIR))
		return -ERR_NOPATH;
	if( !cur->device->fs || !cur->device->fs->open )
		return -ERR_NOPATH;
	if( *next ){ 
		//已经达到目标文件
		f = *next;
	}else{
		//Check if already open!
		for( f=cur->child; f; f=f->next )
			if( strncmp( f->name, name, FILE_NAME_LEN ) == 0 ){
				f->reference ++;
				*next = f;
				return 0;
			}
		f = falloc();
	}
	f->device = cur->device;
	f->flag = flag;
	f->parent = cur;
	strncpy( f->name, name, FILE_NAME_LEN-1 );
	//调用文件系统操作
	int ret = cur->device->fs->open( f, name );
	if( ret < 0 ){
		f->parent = 0;
		ffree(f);
		return ret;
	}
	flinkto( f, cur );
	*next = f;
	return 0;
}

// 循环打开路径目录
static int name_file( file_t* f, const char* path )
{
	char name[FILE_NAME_LEN];
	int i, j, ret;
	//file_t cur, next;
//	printf("name_file %s \n", path );
	file_t *parent = root;
	file_t *next;
	for( i=0, j=0; path[i]!='\0'; i++ ){
		if( path[i]=='/' ){
			//获取完一级目录
			name[j]='\0';
			if( j==0 ){
				parent = root; //memcpy( &cur, root, sizeof(file_t) );
			}else{
				next = 0;
				//打开一级目录
				ret = try_open( parent, &next, name, f->flag );
				if( ret<0 ){
					printf("open %s failed\n", name );
					ffree( parent );
					return ret;
				}
				parent = next;
			}
			++ parent->reference;
			j=0;
		}else{
			name[j++] = path[i];
		}
	}
	name[j]='\0';
	ret = try_open( parent, &f, name, f->flag );
	if( ret<0 ){
		ffree( parent );
		return ret;
	}
	return 0;
}

int vfs_open( const char* path, uint mode, uint flag, file_t **retf )
{
	file_t* f;
	int ret;
	f = falloc();
	if( !f )
		return -ERR_NOMEM;
	f->mode = mode;
	f->flag = flag;
	ret = name_file( f, path );
	if(  ret < 0 ){
		ffree(f);
		return ret;
	}
	*retf = f;
	//没有可用的描述符
	return 0;
}


int vfs_setsize( int id, size_t siz )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->setsize )
		return file->device->fs->setsize( file, siz );
	return -ERR_NOIMP;
}

int vfs_rename( int id, const char* name )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->rename )
		return file->device->fs->rename( file, (char*)name );
	return -ERR_NOIMP;
}

int vfs_readdir( int id, void* buf, size_t siz )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->dir )
		return file->device->fs->dir( file, buf, siz );
	return -ERR_NOIMP;
}

int vfs_ioctl( int id, uint cmd, uint arg )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->ioctl )
		file->device->fs->ioctl( file, cmd, arg );
	return -ERR_NOIMP;
}

int vfs_read( int id, size_t count, uchar* buf )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->read )
		return file->device->fs->read( file, buf, count );
	return -ERR_NOIMP;
}

int vfs_seek( int id, size_t step, size_t pos )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	switch(pos)
	{
	case SEEK_CUR:
		file->pos += step;
		if( file->pos > file->size )
			file->pos = file->size;
		break;
	case SEEK_SET:
		file->pos = step;
		if( file->pos > file->size )
			file->pos = file->size;
		break;
	case SEEK_END:
		file->pos = file->size;
		file->pos += step;
		if( file->pos > file->size )
			file->pos = file->size;
		break;
	}
	return 0;
}

int vfs_write( int id, size_t count, const uchar* buf )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->write )
		return file->device->fs->write( file, buf, count );
	return -ERR_NOIMP;
}

int vfs_close( int id )
{
	file_t* file;
	file = fgetfile(id);
	if( !file )
		return -ERR_WRONGARG;
	//调用文件系统操作
	if( file->device->fs->close )
		file->device->fs->close(file);
	//释放内存空间
	ffree( file );
	return 0;
}

//VFS服务处理过程
#define BUFFER_PAGES_SIZE	(PAGE_SIZE*16)
static void vfs_service()
{
	int ret;
	ret = SmNotifyService( FileSystemId, 0, "VFS" );
	if( ret < 0 ){
		printf("[vfs] failed to notify service. ret=%d\n", ret);
		SysExitThread(ret);
	}
	char* bufferPages = (char*)SysAllocateMemory( SysGetCurrentSpaceId(), BUFFER_PAGES_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
	Message msg;
	printf("[vfs]vfs_service started.\n");
	for(;;){
		file_t *fp;
		ret = WaitMessage( &msg );
		if( ret<0 ){
			printf("[vfs]vfs_service recv error.\n");;
			break;
		}
		msg.Code = 0;
		switch( msg.Command ){
		case File_Open: //open("", mode, flag )
			SysSwapMemory( SPACEID(msg.ThreadId), msg.Large[0], (size_t)bufferPages, KB(4), MAP_ADDRESS );
			msg.Code = vfs_open( bufferPages, msg.Arguments[0], msg.Arguments[1], &fp );
			if( msg.Code==0 ){
				msg.Arguments[0] = fgetid(fp); //fd
				msg.Arguments[1] = fp->size;
				msg.Arguments[2] = fp->attribute;
				msg.Arguments[3] = fp->ctime;
				msg.Arguments[4] = fp->mtime;
			}
			break;
		case File_Close:
			msg.Code = vfs_close( msg.Arguments[0] );
			break;
		case File_Control:
			msg.Code = vfs_ioctl( msg.Arguments[0], msg.Arguments[1], msg.Arguments[2] );
			break;
		case File_Seek:
			msg.Code = vfs_seek( msg.Arguments[0], msg.Arguments[1], msg.Arguments[2] );
			break;
		case File_SetSize:
			msg.Code = vfs_setsize( msg.Arguments[0], msg.Arguments[1] );
			break;
		case File_Read: //read( id, count, buf, bufsiz )
		{
			size_t siz = msg.Arguments[2]; //buffer size
			if( siz > BUFFER_PAGES_SIZE )
				siz = BUFFER_PAGES_SIZE;
			if( siz % PAGE_SIZE )
				siz -= (siz%PAGE_SIZE); //对齐，去掉余数
			if( msg.Arguments[1] > siz )
				msg.Arguments[1] = siz;
			msg.Code = vfs_read( msg.Arguments[0], msg.Arguments[1], (uchar*)bufferPages );
			if( msg.Code >0 )
				SysSwapMemory( SPACEID(msg.ThreadId), msg.Large[0], (size_t)bufferPages, siz, MAP_ADDRESS );
			break;
		}
		case File_Write: //write( id, count, buf )
		{
			size_t siz = msg.Arguments[2]; //buffer size
			if( siz > BUFFER_PAGES_SIZE )
				siz = BUFFER_PAGES_SIZE;
			if( siz % PAGE_SIZE )
				siz -= (siz%PAGE_SIZE); //对齐，去掉余数
			if( msg.Arguments[1] > siz )
				msg.Arguments[1] = siz;
			SysSwapMemory( SPACEID(msg.ThreadId), msg.Large[0], (size_t)bufferPages, siz, MAP_ADDRESS );
			msg.Code = vfs_write( msg.Arguments[0], msg.Arguments[1], (uchar*)bufferPages );
			break;
		}
		}
		ReplyMessage( &msg );
	}
	SmRemoveService(FileSystemId);
	SysExitThread(0);
}

// 虚拟文件系统服务初始化
extern fs_t fs_rootfs;
extern fs_t fs_fat32;
int vfs_init()
{
	root = 0;
	//fileList
	fileList = (file_t*)SysAllocateMemory( SysGetCurrentSpaceId(), sizeof(file_t)*MAX_FILE, MEMORY_ATTR_WRITE, 0 );
	if( fileList == NULL ){
		printf("[vfs]Failed to allocate fileList\n");
		return -ERR_NOMEM;
	}
	memset( fileList, 0, sizeof( file_t ) * MAX_FILE );
	fileCount = 0;
	//创建服务线程
	vfsThread = SysCreateThread( SysGetCurrentSpaceId(), (size_t)&vfs_service, 0, 0, 0 );
	SysResumeThread( vfsThread );
	// 安装必要文件系统
	fs_register( &fs_rootfs );
	// 注册设备以安装此文件系统
	rootDev = device_register( DEV_TYPE_ROOT, vfsThread, 0, "root" );
	if( rootDev==(device_t*)0 ){
		printf("[vfs]Failed to install root fs.\n");
	}else{
		root = rootDev->devFile;
	}
	// 安装必要文件系统
	fs_register( &fs_fat32 );
	return 0;
}
