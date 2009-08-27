#ifndef _SYSTEM_
#define _SYSTEM_

#include <types.h>

//C++ definitions
#ifndef false
#define false 0
#endif
#ifndef true
#define true (!false)
#endif
#ifndef null
#define null ((void*)0)
#endif
#ifndef bool
#define bool int
#endif

namespace System{

#include <basestring.h>
#include <messenger.h>
#include <thread.h>
#include <service.h>

};

#endif
