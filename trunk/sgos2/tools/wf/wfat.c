// write fat32 for sgos2
// Huang Guan(gdxxhg@gmail.com)
// 090804

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat32.h"
#include "buffer.h"
#include "unicode.h"

#define FILE_NAME_LEN 256

static void print_help()
{
	printf("wfat sgos2.img -src kernel/kernel.bin -dest sgos2/kernel.bin\n" );
}

static int writefile( char* fname, char* src, char* dest )
{
	int ret;
	FILE* fp;
	FILE_DESC fat[16];
	char* buf;
	unsigned int size;
	int n=0;
	ret = buffer_init( fname );
	if(  ret<0 ){
		printf("buffer_init failed.\n");
		return ret;
	}
	memset( fat, 0, 16*sizeof(FILE_DESC) );
	memcpy( &fat[n], root, sizeof(FILE_DESC) );
	//下面代码可以优化。
	fat[n+1].dev = fat[n].dev;
	fat[n+1].dev_info = fat[n].dev_info;
	fat[n+1].parent = &fat[n];
	fat[n+1].flag |= FILE_FLAG_CREATE;
	n++;
	while( strchr( dest, '/' )!=NULL ){
		char* p = strchr( dest, '/' );
		*p = '\0';
		fat[n].flag = 0;
		ret = fat32_open( &fat[n], dest );
		if( ret<0 ){
			printf("fat32_open failed.\n");
			return -1;
		}
		fat[n+1].dev = fat[n].dev;
		fat[n+1].dev_info = fat[n].dev_info;
		fat[n+1].parent = &fat[n];
		fat[n+1].flag |= FILE_FLAG_CREATE;
		n++;
		dest = p+1;
	}
	ret = fat32_open( &fat[n], dest );
	if( ret<0 ){
		printf("fat32_open failed.\n");
		return -1;
	}
	fp = fopen( src, "rb" );
	if( fp==NULL ){
		printf("open src file failed.\n");
		return -1;
	}
	fseek( fp, 0, SEEK_END );
	size  = ftell ( fp );
	buf = malloc( size );
	fseek( fp, 0, SEEK_SET );
	if( fread( buf, size, 1, fp )!= 1 ){
		printf("read src file failed.\n");
		return -1;
	}
	fclose( fp );
	if( fat32_write( &fat[n], buf, size ) != size ){
		printf("write dest file failed.\n");
	}
	while( n>0 ){
		fat32_close( &fat[n] );
		n--;
	}
	free( buf );
	buffer_cleanup();
	return 0;
}

//
// cmd:  wfat sgos2.img -src kernel/kernel.bin -dest sgos2/kernel.bin
int main( int argc, const char** argv )
{
	int i;
	char filename[FILE_NAME_LEN], src[FILE_NAME_LEN], dest[FILE_NAME_LEN];
	if( argc != 6 ){
		print_help();
		return -1;
	}
	memset( filename, 0, FILE_NAME_LEN );
	memset( src, 0, FILE_NAME_LEN );
	memset( dest, 0, FILE_NAME_LEN );
	for(i=1; i<5; i++ ){
		if( strcmp( argv[i], "-src" ) == 0 ){
			i++;
			strncpy( src, argv[i], FILE_NAME_LEN-1 );
		}else if( strcmp( argv[i], "-dest" ) == 0 ){
			i++;
			strncpy( dest, argv[i], FILE_NAME_LEN-1 );
		}else{
			strncpy( filename, argv[i], FILE_NAME_LEN-1 );
		}
	}
	if( !*filename || !*src || !*dest ){
		printf("arguments not correct..\n");
		print_help();
		return -2;
	}
	//work
	unicode_init();
	return writefile( filename, src, dest );
}

