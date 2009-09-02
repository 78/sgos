#ifndef _FAT32_H_
#define _FAT32_H_

#include <stdlib.h>

//FAT32 引导扇区结构
typedef struct FAT32_BOOTSEC{
	t_8 Jump[3];	//跳转指令	0x0000
	t_8 OEM[8];	//OEM信息	0x0003
	t_16 BytPerSec;	//每扇区字节数	0x000B
	t_8 SecPerClu;	//每cluster的扇区数	0x000D
	t_16 SecReserved;	//保留扇区	0x000E
	t_8 Fats;	//fat表数目	0x0010
	t_16 RootUnits;	//根目录项数	0x0011*
	t_16 SecSmall;	//扇区总数（少于256*256）0x0013*
	t_8 Media;	//媒体描述 0xF8表示硬盘	0x0015*
	t_16 SecPerFat16;	//每fat扇区数	0x0016*
	t_16 SecPerTra;	//每磁道扇区数	0x0018
	t_16 Heads;	//磁头数	0x000A
	t_32 SecHidden;	//隐藏扇区	0x001C
	t_32 SecBig;	//总扇区数（大于等于256*256）	0x0020
	t_32 SecPerFat;	//每fat扇区数 （fat32）0x0024
	t_16 ExtendedFlag;	//扩展标志fat32专有
	/*位0-3：活动 FAT 数(从0 开
		始计数，而不是1).
		只有在不使用镜像时
		才有效
		位4-6：保留
		位7：0 值意味着在运行时
		FAT 被映射到所有的FAT
		1 值表示只有一个FAT
		是活动的
		位8-15：保留*/
	t_16 FSVersion;	//0x002A
	t_32 RootCluster;	//根目录簇号 0x002C
	t_32 FSInfoSector;	//文件系统信息扇区号，一般为1 0x0030
	t_16 BootBackupSector;	//备份引导扇区	0x0034
	t_8	 Reserved[10];	//0x0036
	t_8  Drive;	//物理驱动器号 物理硬盘被标识为0x0040
	t_8	 Reserved1;	//保留 0x0041
	t_8	 EBR;	//扩展引导标志位0x28或0x29 0x0042
	t_32 Volume;	//序列号	0x0043
	t_8 Label[11];	//标签	0x0047
	t_8 System[8];	//文件系统类型	0x0052
	t_8 BootCode[420];	//0x005A	引导代码
	t_16 Sign;	//0xAA55 引导标志位
}__attribute__((packed)) FAT32_BOOTSEC;

//32字节的fat文件结构
typedef struct FAT32_ENTRY{
	t_8 name[8];	//文件名
	t_8 extention[3];	//扩展名
	t_8 attribute;	//属性
	t_8 reserved;	//保留
	t_8 ctime_tenth;	//创建时间的10 毫秒位
	t_16 ctime;
	t_16 cdate;
	t_16 adate;
	t_16 cluster_high;
	t_16 mtime;	//最后修改时间
	t_16 mdate;	//最后修改日期
	t_16 cluster;	//开始簇（这个字拼音不知道怎么打哦 [cu2, 2006-11-5 13:37]）号
	t_32 size;	//文件大小
}__attribute__ ((packed)) FAT32_ENTRY;

typedef struct FAT32_NAME_ENTRY
{
	t_8 id;
	t_16 name1[5];
	t_8 attr;
	t_8 reserved;
	t_8 checksum;
	t_16 name2[6];
	t_16 first_cluster;
	t_16 name3[2];
} __attribute__ ((packed)) FAT32_NAME_ENTRY;

#define IS_FAT32CLU_FREE( c ) (c==0)
#define IS_FAT32CLU_INUSE( c ) (c>=2&&c<0x0fffffef)
#define IS_FAT32CLU_RESERVED( c ) (c>=0x0ffffff0&&c<0x0ffffff6)
#define IS_FAT32CLU_BAD( c ) (c==0xfffffff7)
#define IS_FAT32CLU_END( c ) (c>=0x0ffffff8)

// Fat分配表起始 = BytPerSec * SecReserved  临时大小：8KB
// 根目录起始 = Fat起始 + BytPerSec * SecPerFat * Fats  临时大小：16KB
// 数据起始 = 根目录起始 + 32 * RootUnits
typedef struct FAT32DEV{
	t_32 secPerFat;	//每fat扇区数
	t_32 bytPerSec;	//每扇区字节数 512
	t_32 secReserved;	//保留扇区数
	t_32 rootClu;	//根目录簇
	t_32 fats;	//fat个数
	t_32 secPerClu;	//每cluster扇区数
	//下面需要计算
	t_32 fatAddr;	//fat基址
	t_32 rootAddr;	//根目录基址
	t_32 dataAddr;	//数据区基址
	t_32 bytPerClu;	//每簇大小
	//char *cluData;	//簇数据缓冲
	//t_32 clusterNo; //缓冲中的簇号
	t_32 newCluStart;   //新簇获取的开始位置，这是为了加快搜索速度
}FAT32DEV;

#define RDONLY 1
#define HIDDEN 2
#define SYSTEM 4
#define VOLUME 8
#define SUBDIR 16
#define SAVE 32

#define FAT32_IS_READONLY(a)(a&RDONLY)
#define FAT32_IS_HIDDEN(a)(a&HIDDEN)
#define FAT32_IS_SYSTEM(a)(a&SYSTEM)
#define FAT32_IS_VOLUME(a)(a&VOLUME)
#define FAT32_IS_SUBDIR(a)(a&SUBDIR)

#define FAT32_IS_LNAME(a) (a==0xf)

#endif
