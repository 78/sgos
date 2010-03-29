#include <sgos.h>
#include <stdio.h>
#include <api.h>
#include "debug.h"

int fstat(int fd, struct _stat *buf)
{
	DBG("fd=%d", fd );
	return 0;
}
