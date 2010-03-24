#include <sgos.h>
#include <api.h>
#include <stdio.h>

int main()
{
	FILEBUF* fb;
	printf("Start hello program.\n");
	
	printf("Testing file operations.\n");
	while( (fb = FsOpenFile("/c:/sgos2/startup.xml", FILE_READ, 0 ) )==NULL )
		SysSleepThread(200);
	char buf[256];
	int ret;
	printf("FsOpenFile fb=%x\n", fb );
	ret = FsReadFile(fb, (uchar*)buf, 256 );
	printf("FsReadFile ret=%d\n", ret );
	if( ret >=0 ){
		buf[ret]=0;
		printf("buf:\n%s\n", buf );
	}
	FsCloseFile( fb );
	return 0;
}
