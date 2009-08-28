#include <stdio.h>
#include <system.h>

using namespace System;

int startService()
{
	return 0;
}

extern int lba_init();
int main()
{
	lba_init();
	return startService();
}

