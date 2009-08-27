#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include <types.h>

struct timespec {
        long tv_sec;
        long tv_nsec;
};

typedef uint pthread_t;
typedef uint pthread_attr_t;
typedef uint pthread_mutexattr_t;

typedef struct THREAD_LIST{
	pthread_t		thread;
	struct THREAD_LIST*	next;
}THREAD_LIST;

typedef struct _mutex{
	struct THREAD_LIST*	list;
	uint			value;
}pthread_mutex_t;


/*
 * PThread Functions
 */
int pthread_create (pthread_t * thread, const pthread_attr_t * attr,
	void *(*start) (void *), void *arg);
int pthread_detach (pthread_t thread);
int pthread_equal (pthread_t t1, pthread_t t2);
void pthread_exit (void *value_ptr);
int pthread_join (pthread_t thread, void **value_ptr);
int pthread_kill(pthread_t thread, int sig);
int pthread_cancel (pthread_t thread);
/*
 * Mutex Functions
 */
int pthread_mutex_init (pthread_mutex_t * mutex,
                                const pthread_mutexattr_t * attr);
int pthread_mutex_destroy (pthread_mutex_t * mutex);
int pthread_mutex_lock (pthread_mutex_t * mutex);
int pthread_mutex_timedlock(pthread_mutex_t *mutex,
                                    const struct timespec *abstime);
int pthread_mutex_trylock (pthread_mutex_t * mutex);
int pthread_mutex_unlock (pthread_mutex_t * mutex);
#endif
