//pthread
#include <sgos.h>
#include <api.h>
#include <stdlib.h>
#include <pthread.h>


void __pthread_init()
{
}

int pthread_create (pthread_t * thread, const pthread_attr_t * attr,
	void *(*start) (void *), void *arg)
{
}

int pthread_detach (pthread_t thread)
{
}

int pthread_equal (pthread_t t1, pthread_t t2)
{
}

void pthread_exit (void *value_ptr)
{
}

int pthread_join (pthread_t thread, void **value_ptr)
{
}

int pthread_kill(pthread_t thread, int sig)
{
}

int pthread_cancel (pthread_t thread)
{
}

