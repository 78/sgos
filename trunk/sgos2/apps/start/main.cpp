
#include <api.h>
#include <system.h>
#include <stdio.h>
#include <bxml.h>

using namespace System;

int main()
{
	Messenger msger;
	BXML_DATA* xml;
	//
	//request file until it works
	int flen;
	for(;;Thread::Sleep(100)/*wait 0.1s*/){
		//加载startup.xml文件，然后取出要加载的项目
		msger.parse("<msg to=\"vfs\"><readall file=\"/c:/sgos/startup.xml\" /></msg>");
		if( msger.request() < 0 )
			continue;
		//read file data
		flen = msger.getUInt("/readall:size");
		if( flen<=0 )
			continue;
		break;
	}
	char* buf = new char[flen];
	msger.read( "/readall", buf, flen-1 );
	xml = bxml_parse( buf );
	if( bxml_redirect( xml, "...", 0 ) < 0 )
		return -ERR_UNKNOWN;
	do{
		char* name = bxml_readname(xml,".");
		if( name && strcmp( name, "item")== 0 ){
			int ret;
			uint proc;
			ret = sys_process_create( bxml_readstr(xml, "."), 
				(const char**)0, NULL, &proc );
			if( ret<0 ){
				printf("process %s create failed.\n", bxml_readstr(xml, ".") );
			}
		}
	}while(bxml_movenext(xml));
	return 0;
}

