#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <sgos.h>
#include <semaphore.h>

struct THREAD;

typedef struct MUTEX{
	struct THREAD_LIST*	list;
	uint			lock;
}mutex_t;

void mutex_remove_thread( mutex_t* mut, struct THREAD* thr );
int mutex_trylock( mutex_t *mut );
void mutex_lock( mutex_t *mut );
void mutex_init( mutex_t *mut );
void mutex_unlock( mutex_t *mut );
void mutex_destroy( mutex_t *mut );

#endif
