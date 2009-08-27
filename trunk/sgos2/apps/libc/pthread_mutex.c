//pthread_mutex
#include <sgos.h>
#include <api.h>
#include <pthread.h>


int pthread_mutex_init (pthread_mutex_t * mutex,
                                const pthread_mutexattr_t * attr)
{
}

int pthread_mutex_destroy (pthread_mutex_t * mutex)
{
}

int pthread_mutex_lock (pthread_mutex_t * mutex)
{
}

int pthread_mutex_timedlock(pthread_mutex_t *mutex,
                                    const struct timespec *abstime)
{
}

int pthread_mutex_trylock (pthread_mutex_t * mutex)
{
}

int pthread_mutex_unlock (pthread_mutex_t * mutex)
{
}
