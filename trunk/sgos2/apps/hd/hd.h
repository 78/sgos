//sgos2
#ifndef _HD_H
#define _HD_H

#include <types.h>

//硬盘端口，状态码及分区表
#define HD_DATA		0x1f0
#define HD_CMD		0x3f6
#define HD_STATUS	0x1f7
#define HD_COMMAND HD_STATUS
//状态码
#define STAT_ERR	0x01
#define STAT_INDEX	0x02
#define STAT_ECC	0x04
#define STAT_DRQ	0x08
#define STAT_SEEK	0x10
#define STAT_WRERR	0x20
#define STAT_READY	0x40
#define STAT_BUSY	0x80
//命令
#define WIN_RESTORE	0x10
#define WIN_READ	0x20
#define WIN_WRITE	0x30
#define WIN_VERIFY	0x40
#define WIN_FORMAT	0x50
#define WIN_INIT	0x60
#define WIN_SEEK 	0x70
#define WIN_DIAGNOSE	0x90
#define WIN_SPECIFY	0x91


//硬盘分区表
typedef struct PARTITION
{
	t_8	bootable;
	t_8	head;
	t_16	sector_cylinder; 	/* 0-5 bit -- sector,6-15 bit -- cylinder */
	t_8	sysID;
	t_8	endHead;
	t_16	endSectorCylinder;	/* 0-5 bit -- sector,6-15 bit -- cylinder */
	t_32	skipSectors;
	t_32	allSectors;
}__attribute__((packed)) PARTITION;


//主设备号 3
//RM to LINEAR 从带段和偏移式的地址转换为内核线性地址
#define R2L(addr) ( (((uint)addr & 0xFFFF0000) >> 12) + ((t_32)addr & 0xFFFF))

#define MAX_HD 2	//最多两个硬盘
//硬盘参数表
typedef struct HD_PARAMS{
	t_16	cyls;	//柱面数
	t_8	heads;	//磁头数
	t_16	null0;	//
	t_16	WPC;	//预补偿柱面号
	t_8	null1;	//
	t_8	controlByte;	//控制字节
	t_8	null2;
	t_8	null3;
	t_8	null4;
	t_16	lzone;	//磁头登陆柱面号
	t_8	SPT;	//每次道扇区数
	t_8	null5;
}__attribute__((packed)) HD_PARAMS;

//读端口数据
#define READ_WORDS( port, buf, n) \
__asm__("cld;rep;insw"::"d"(port),"D"(buf),"c"(n))
//写端口数据，n个16位数据
#define WRITE_WORDS( port, buf, n) \
asm("cld;rep;outsw"::"d"(port),"S"(buf),"c"(n))

#endif
