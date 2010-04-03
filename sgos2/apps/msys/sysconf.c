#include <sgos.h>
#include <errno.h>
#include <api.h>
#include "debug.h"
#include "unistd.h"

size_t getpagesize()
{
	return PAGE_SIZE;
}

/* sysconf: POSIX 4.8.1.1 */
/* Allows a portable app to determine quantities of resources or
   presence of an option at execution time. */
long int
sysconf (int in)
{
  switch (in)
    {
      case _SC_ARG_MAX:
	/* FIXME: what's the right value?  _POSIX_ARG_MAX is only 4K */
	return KB(4);
      case _SC_OPEN_MAX:
	//FIXME
	return 4096;
      case _SC_PAGESIZE:
	return PAGE_SIZE;
      case _SC_CLK_TCK:
	//FIXME
	return 10;
      case _SC_JOB_CONTROL:
	return _POSIX_JOB_CONTROL;
      case _SC_CHILD_MAX:
	//FIXME
	return 1024;
      case _SC_NGROUPS_MAX:
	//FIXME
	return 1024;
      case _SC_SAVED_IDS:
	return _POSIX_SAVED_IDS;
      case _SC_VERSION:
	return _POSIX_VERSION;
      case _SC_NPROCESSORS_CONF:
      case _SC_NPROCESSORS_ONLN:
	//FIXME
	return 1;
	/*FALLTHRU*/
      case _SC_PHYS_PAGES:
      case _SC_AVPHYS_PAGES:
	//FIXME
	return MB(64)/PAGE_SIZE;
    }
  DBG("not implemeted in=%d", in );
  /* Invalid input or unimplemented sysconf name */
  return -1;
}
