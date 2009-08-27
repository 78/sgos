#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <sgos.h>

#define	down(a)	sema_down(a)
#define up(a)	sema_up(a)

struct THREAD;

typedef struct THREAD_LIST{
	struct THREAD*		thread;
	struct THREAD_LIST*	next;
}THREAD_LIST;

typedef struct SEMAPHORE{
	struct THREAD_LIST*	list;
	uint			value;
}sema_t;

void sema_remove_thread( sema_t* mut, struct THREAD* thr );
int sema_trydown( sema_t *mut );
void sema_down( sema_t *mut );
void sema_init( sema_t *mut );
void sema_init_ex( sema_t *mut, int value );
void sema_up( sema_t *mut );
void sema_destroy( sema_t *mut );

#endif
