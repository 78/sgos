#ifndef _SEMA_H_
#define _SEMA_H_

#include <types.h>
#include <pthread.h>

//sema.c
#define	down(a)	sem_down(a)
#define up(a)	sem_up(a)

typedef struct SEMAPHORE{
	struct THREAD_LIST*	list;
	uint			value;
}sem_t;

int sem_trywait( sem_t *sem );
void sem_wait( sem_t *sem );
void sem_init( sem_t *sem, int pshared, uint value );
void sem_post( sem_t *sem );
void sem_destroy( sem_t *sem );

#endif

