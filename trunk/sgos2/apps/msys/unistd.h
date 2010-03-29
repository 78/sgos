#ifndef _UNISTD_H
#define _UNISTD_H

#define __rtems__

#include "features.h"

#define STDIN_FILENO    0       /* standard input file descriptor */
#define STDOUT_FILENO   1       /* standard output file descriptor */
#define STDERR_FILENO   2       /* standard error file descriptor */

/*
 *  4.8.1 Get Configurable System Variables, P1003.1b-1993, p. 96
 *
 *  NOTE: Table 4-2, Configurable System Variables, p. 96
 */

#define _SC_ARG_MAX                 0
#define _SC_CHILD_MAX               1
#define _SC_CLK_TCK                 2
#define _SC_NGROUPS_MAX             3
#define _SC_OPEN_MAX                4
  /* no _SC_STREAM_MAX */
#define _SC_JOB_CONTROL             5
#define _SC_SAVED_IDS               6
#define _SC_VERSION                 7
#define _SC_PAGESIZE                8
/* CYGWIN and MSYS-specific values .. do not touch */
#define _SC_NPROCESSORS_CONF        9
#define _SC_NPROCESSORS_ONLN       10
#define _SC_PHYS_PAGES             11
#define _SC_AVPHYS_PAGES           12
/* end of CYGWIN and MSYS-specific values */
#define _SC_MQ_OPEN_MAX            13
#define _SC_MQ_PRIO_MAX            14
#define _SC_RTSIG_MAX              15
#define _SC_SEM_NSEMS_MAX          16
#define _SC_SEM_VALUE_MAX          17
#define _SC_SIGQUEUE_MAX           18
#define _SC_TIMER_MAX              19
#define _SC_TZNAME_MAX             20

#define _SC_ASYNCHRONOUS_IO        21
#define _SC_FSYNC                  22
#define _SC_MAPPED_FILES           23
#define _SC_MEMLOCK                24
#define _SC_MEMLOCK_RANGE          25
#define _SC_MEMORY_PROTECTION      26
#define _SC_MESSAGE_PASSING        27
#define _SC_PRIORITIZED_IO         28
#define _SC_REALTIME_SIGNALS       29
#define _SC_SEMAPHORES             30
#define _SC_SHARED_MEMORY_OBJECTS  31
#define _SC_SYNCHRONIZED_IO        32
#define _SC_TIMERS                 33
#define _SC_AIO_LISTIO_MAX         34
#define _SC_AIO_MAX                35
#define _SC_AIO_PRIO_DELTA_MAX     36
#define _SC_DELAYTIMER_MAX         37

/*
 *  P1003.1c/D10, p. 52 adds the following.
 */

#define _SC_THREAD_KEYS_MAX              38
#define _SC_THREAD_STACK_MIN             39
#define _SC_THREAD_THREADS_MAX           40
#define _SC_TTY_NAME_MAX                 41

#define _SC_THREADS                      42
#define _SC_THREAD_ATTR_STACKADDR        43
#define _SC_THREAD_ATTR_STACKSIZE        44
#define _SC_THREAD_PRIORITY_SCHEDULING   45
#define _SC_THREAD_PRIO_INHERIT          46
/* _SC_THREAD_PRIO_PROTECT was _SC_THREAD_PRIO_CEILING in early drafts */
#define _SC_THREAD_PRIO_PROTECT          47
#define _SC_THREAD_PRIO_CEILING          _SC_THREAD_PRIO_PROTECT
#define _SC_THREAD_PROCESS_SHARED        48
#define _SC_THREAD_SAFE_FUNCTIONS        49
#define _SC_GETGR_R_SIZE_MAX             50
#define _SC_GETPW_R_SIZE_MAX             51
#define _SC_LOGIN_NAME_MAX               52
#define _SC_THREAD_DESTRUCTOR_ITERATIONS 53


#endif
