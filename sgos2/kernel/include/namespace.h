#ifndef _NAMESPACE_H_
#define _NAMESPACE_H_

#include <sgos.h>


typedef struct NAME{
	void*			thread;			//thread
	char			name[NAME_LEN];	//name
	time_t			ctime;			//create time
	uint			used;			//used times
}NAME, name_t;

struct THREAD;

void* name_match( const char* name );
int name_remove( struct THREAD* thr, const char* name );
int name_insert( struct THREAD* thr, const char* name );
void name_init();
void name_destroy();

#endif

