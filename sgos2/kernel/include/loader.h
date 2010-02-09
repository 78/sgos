#ifndef _LOADER_H_
#define _LOADER_H_

#include <module.h>

struct PROCESS;
// 从文件中加载可执行文件。
int loader_load( struct PROCESS* proc, char* filename, uchar share, MODULE** mod );
int loader_process( struct PROCESS* proc, char* file, uchar* data, uchar share, MODULE** ret_mod );

#endif
