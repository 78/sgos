
#include <stdio.h>
#include <api.h>

int t1, t2, t3, pid;
unsigned int TIME64 (void)
{
	unsigned int dummy,low;
        __asm__("rdtsc"
                :"=a" (low),
                 "=d" (dummy));
	return low;
}

void thread_test()
{
	printf("thread_test: %d\n", pid);
	sys_thread_exit(0);
}

int entry()
{
	int tid;
	t1 = TIME64();
	pid = sys_process_self();
	t2 = TIME64();
	printf("pid: %d  sysenter cycles:%d\n", pid, t2-t1 );
	tid = sys_thread_create( (size_t)thread_test );
	sys_thread_resume(tid);
	return 0;
}
