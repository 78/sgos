#ifndef _LOADER_H_
#define _LOADER_H_

#include <module.h>

struct PROCESS;
int loader_process( struct PROCESS* proc, char* file, uchar* data, uchar share, MODULE** ret_mod );

#endif
