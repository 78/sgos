#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <sgos.h>
#include <thread.h>
#include <mutex.h>

#define MAX_PROCESS_NUM	1024
#define PROCESS_NAME_LEN	32

#define IS_KERNEL_PROCESS( p ) (p->user)
typedef struct PROCESS{
	uint				id;
	mutex_t				mutex;
	uint				user;
	struct PROCESS*		pre, *next;
	struct PROCESS*		parent, *child;
	struct THREAD*		thread;
	void*				message;
	char				name[PROCESS_NAME_LEN];
	void*				information;
	uint				page_dir;	//page_dir
	uint				exit_code;
	void*				module;		//module informations
	void*				environment;	//process startup environment
	
}PROCESS;

void process_init();
PROCESS* current_proc();

#endif
