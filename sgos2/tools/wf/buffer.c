#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

static FILE* fp;

//硬盘分区表
typedef struct PARTITION
{
	t_8		bootable;
	t_8		head;
	t_16	sec_cyl; 		/* 0-5 bit -- sector,6-15 bit -- cylinder */
	t_8		system_id;
	t_8		head2;
	t_16	sec_cyl2; /* 0-5 bit -- sector,6-15 bit -- cylinder */
	t_32	skip_sectors;
	t_32	total_sectors;
}__attribute__((packed)) PARTITION;

typedef struct HD_DRIVE{
	unsigned int start;	//其实扇区
	unsigned int sectors;	//扇区数
}HD_DRIVE;

static HD_DRIVE drives[5*2];

typedef struct BUFFER{
	unsigned int block;
	void* buf;
	unsigned short dev;
}BUFFER;

#define MAX_BUFFER 10240
BUFFER* buffers;
int buffer_count = 0;

int buffer_init( const char* fname )
{
	int i=0, ret;
	buffers = (BUFFER*)malloc( MAX_BUFFER*sizeof(BUFFER) );
	memset( buffers, 0,  MAX_BUFFER*sizeof(BUFFER) );
	PARTITION* p;
	fp = fopen( fname, "rb+" );
	if( fp == NULL ){
		return -1;
	}
	unsigned char boot[512];
	if( fread( boot, 512, 1, fp )!= 1 ){
		printf("read file error. %s\n", fname );
		fclose(fp );
		return -2;
	}
	p = (PARTITION*)(boot + 0x1BE);
	if( p->bootable )
	{
		int j;
		for(j=1; j<5; j++,p++ )
		{
			drives[i*5+j].start = p->skip_sectors;	//相对扇区起始位置
			drives[i*5+j].sectors = p->total_sectors;	//总扇区数
			if( p->total_sectors){
				printf("disk[%d]: start_sector:0x%X   total_sectors:0x%X\n", i*5+j, drives[i*5+j].start, drives[i*5+j].sectors);
				if( (ret=fat32_init( i*5+j, 0 ))==0 )
					return ret;
			}
		}
	}
	printf("fat32 partition not found.\n");
	return -2;
}

void* buffer_read( unsigned short dev, unsigned int block )
{
	void* data;
	int i;
	for( i=0; i<MAX_BUFFER; i++ ){
		if( buffers[i].buf && buffers[i].block == block )
			return buffers[i].buf;
	}
	data = malloc( BLOCK_SIZE );
	if(data){
		fseek( fp, (block*2 + drives[dev].start)*512, SEEK_SET );
		fread( data, BLOCK_SIZE, 1, fp );
	}
	if( buffer_count < MAX_BUFFER ){
		buffers[buffer_count].buf = data;
		buffers[buffer_count].block = block;
		buffers[buffer_count].dev = dev;
		buffer_count++;
	}else{
		printf("error: file size is too big. \n");
	}
	return data;
}

int buffer_write( unsigned short dev, unsigned int block, void* data )
{
	int i;
	for( i=0; i<MAX_BUFFER; i++ ){
		if( buffers[i].buf && buffers[i].block == block ){
			memcpy( buffers[i].buf, data, BLOCK_SIZE );
//			printf("write at: 0x%x\n", (block*2 + drives[dev].start)*512 );
			return 0;
		}
	}
	printf("error: file size is too big. \n");
	return 0;
}

void buffer_release( void* data )
{
}

void buffer_cleanup()
{	
	int i;
	unsigned int block;
	for( i=0; i<MAX_BUFFER; i++ ){
		if( buffers[i].buf ){
			block = buffers[i].block;
			fseek( fp, (block*2 + drives[buffers[i].dev].start)*512, SEEK_SET );
			if( fwrite( buffers[i].buf, BLOCK_SIZE, 1, fp ) !=1 ){
				printf("write image failed at block: 0x%X\n", block );
				return;
			}
			free( buffers[i].buf );
		}
	}
	free( buffers );
	fclose( fp );
}
