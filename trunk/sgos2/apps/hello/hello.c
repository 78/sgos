
#include <stdio.h>
#include <api.h>

unsigned int TIME64 (void)
{
	unsigned int dummy,low;
        __asm__("rdtsc"
                :"=a" (low),
                 "=d" (dummy));
	return low;
}

int entry()
{
	int t1, t2, t3, pid;
	t1 = TIME64();
	pid = sys_process_self();
	t2 = TIME64();
	printf("pid: %d  sysenter cycles:%d\n", pid, t2-t1 );
	return 0;
}
