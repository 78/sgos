/*
 ********************************************************************
 *                C 文件说明
 *
 * 模块：Virtual File System
 * 作用：Virtual Resource System
 * 更新日期：2007-1-2 14:30
 	2007-1-2 14:34	Huang Guan
 	增加rootfsControlFile里几个操作，完善rootfsOpenFile。
 *	Ported from SGOS1 for SGOS2
 * 作者：Huang Guan
 *
 **********************************************************************
 */

#include <sgos.h>
#include <stdio.h>
#include <stdlib.h>
#include "fsservice.h"
/*
 * 计算机资源管理系统
 */

typedef struct PATH_NODE{
	char	name[NAME_LEN];	//路径名称最大是64。
	uchar 	type;		//类型 Node目录 or 设备文件 or 子文件系统
	uchar	flag;		//属性 可读？ 可写？ 临时文件？
	uint	value;		//
	struct PATH_NODE *pre;
	struct PATH_NODE *next;
	struct PATH_NODE *parent;
	struct PATH_NODE *child;
}PNode, *PPNode;

// 通过标准文件接口进行控制
static int rootfsSetupSystem(file_t* file, device_t* dev);
static int rootfsOpenFile(file_t* file, const char* name);
static int rootfsReadFile(file_t* file, uchar* buf, size_t count);
static int rootfsWriteFile(file_t* file, const uchar* buf, size_t count);
static int rootfsControlFile(file_t* file, size_t cmd, size_t arg);
static int rootfsRenameFile(file_t* file, char* name);

static int rootfsSetupSystem(file_t* f, device_t* dev)
{
	if( strcmp( dev->driver, "root" ) != 0 )
		return -ERR_INVALID;
	//对设备创建一个根节点
	dev->devFSInfo = (void*)malloc(sizeof(PNode));
	PPNode p = (PPNode)(dev->devFSInfo);
	strcpy( p->name, "rootfs" );
	p->type = ROOTFS_TYPE_DIR;
	p->flag = 0;
	p->value = 0;
	//节点链表
	p->pre = p->next = p->parent = p->child = (PPNode)0;
	//设置根目录
	f->device = dev;
	f->flag = FILE_READ;
	f->data = (t_32)p;
	f->attribute = FILE_ATTR_DIR;
	f->dir = 1;
	return 0;
}

static int rootfsOpenFile(file_t* f, const char* name)
{
	//printf("rootfsOpenFile: %s parentid:%d\n", name, f->parent );
	PPNode gNod = (PPNode)f->device->devFSInfo;
	PPNode pNod = (PPNode)f->parent->data;
	if( pNod->type ==ROOTFS_TYPE_DIR )
	{
		PPNode p = pNod->child;
		for(;p;p=p->next ){
			if( strcmp(p->name, name)==0 )
			{
				switch( p->type ){
				case ROOTFS_TYPE_DIR:
				//是目录
					f->attribute |= FILE_ATTR_DIR;
					f->dir = 1;
					f->data = (uint)p;
					break;
				case ROOTFS_TYPE_FS:
				//是文件系统？
				{
					file_t* f2 = (file_t*)p->value;
					f->device = f2->device;
					f->data = f2->data;
					f->attribute |= FILE_ATTR_DIR;
					break;
				}
				default:
					printf("[rootfs]unknown node type p->name:%s p->type=%d\n", 
						p->name, p->type );
					return -ERR_WRONGARG;
				}
				f->size = 0;
				f->pos = 0;
				//是只读？
				if( !(p->flag&ROOTFS_FLAG_WRITE) )
					f->attribute|=FILE_ATTR_RDONLY;
				//printf("p->name:%s p->type=%d\n", p->name, p->type );
				return 0;
			}
		}
		//找不到
		if(f->flag&FILE_FLAG_CREATE)
		{
			//创建文件
			p = (PPNode)malloc(sizeof(PNode));
			if(p)
			{
				memset( p, 0, sizeof(PNode) );
				strncpy(p->name, name, NAME_LEN-1);
//				printf("Create node %s in %s\n", p->name, pNod->name );
				p->parent = pNod;
				p->pre=(PPNode)0;
				p->next = pNod->child;
				if( pNod->child )
					pNod->child->pre = p;
				pNod->child = p;
				f->data=(t_32)p;
				f->size=f->pos=0;
				return 0;
			}
		}
		printf("[rootfs]rootfsOpenFile: File not found %s in %s.\n", name, pNod->name);
		return -ERR_WRONGARG;
	}//非目录，不可搜索
	printf("[rootfs]rootfsOpenFile: not a directory %s in %s.\n", name, pNod->name);
	return -ERR_WRONGARG;
}

static int rootfsReadFile(file_t* f, uchar* buf, size_t count)
{
	PPNode gNod = (PPNode)f->device->devFSInfo;
	PPNode p = (PPNode)f->data;
	return -ERR_NOIMP;
}

static int rootfsWriteFile(file_t* f, const uchar* buf, size_t count)
{
	PPNode gNod = (PPNode)f->device->devFSInfo;
	PPNode p = (PPNode)f->data;
	return -ERR_NOIMP;
}

static int rootfsRenameFile(file_t* f, char* name)
{
	PPNode gNod = (PPNode)f->device->devFSInfo;
	PPNode p = (PPNode)f->data;
	return -ERR_NOIMP;
}

static int rootfsControlFile(file_t* f, t_32 cmd, t_32 arg)
{
	PPNode gNod = (PPNode)f->device->devFSInfo;
	PPNode p = (PPNode)f->data;
	switch( cmd ){
	case ROOTFS_SET_TYPE:			//set the type of node
		p->type = arg;
		break;
	case ROOTFS_SET_VALUE:		//give the node a value!!
		p->value = arg;
		break;
	default:
		return -ERR_WRONGARG;
	}
	return 0;
}

//rootfs文件系统
//直接供文件系统服务使用
fs_t fs_rootfs = {
	"rootfs", 
	rootfsSetupSystem,		//SetupSystem
	rootfsOpenFile,			//OpenFile
	rootfsReadFile,			//ReadFile
	rootfsWriteFile,		//WriteFile
	rootfsControlFile,		//Control
	rootfsRenameFile,		//Rename
};

