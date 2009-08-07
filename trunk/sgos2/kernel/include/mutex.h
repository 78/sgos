#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <sgos.h>

struct THREAD;

typedef struct THREAD_LIST{
	struct THREAD*		thread;
	struct THREAD_LIST*	next;
}THREAD_LIST;

typedef struct MUTEX{
	struct THREAD_LIST*	list;
	uint				lock;
}mutex_t;

#endif
