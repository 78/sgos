#include "debug.h"

char *getcwd(char *buf, int n)
{
	DBG("buf=%x", buf );
	return "/";
}
