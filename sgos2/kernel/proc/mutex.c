//mutex

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <thread.h>
#include <mutex.h>

int mutex_trylock( mutex_t *mut )
{	
	uint eflags;
	THREAD* thr;
	THREAD_LIST* tl;
	//we don't want to be interrupt
	local_irq_save( eflags );
	while( --mut->lock ){
		local_irq_restore(eflags);
		mut->lock ++;
		return 0;
	}
	local_irq_restore(eflags);
	return 1;
}

void mutex_lock( mutex_t *mut )
{
	uint eflags;
	THREAD* thr;
	THREAD_LIST* tl, *tl2;
	//we don't want to be interrupt
	local_irq_save( eflags );
//	kprintf("{%d come}", current_thread()->id );
	while( --mut->lock ){
		mut->lock ++;
		tl = (THREAD_LIST*)kmalloc(sizeof(THREAD_LIST));
		tl->thread = thr = (THREAD*)current_thread();
		//跟队末尾
		tl2 = mut->list;
		if(tl2){
			while( tl2->next )
				tl2 = tl2->next;
			tl2->next = tl;
		}else{
			mut->list = tl;
		}
		tl->next = NULL;
		local_irq_restore(eflags);
		thr->sleepon = mut;
//		kprintf("[%d sleep]", thr->id );
		thread_sleep();
//		kprintf("[%d wokeup]", thr->id );
		//已经有人帮我们从mut的线程列表里拆除了
		//back
		local_irq_save( eflags );
	}
	local_irq_restore(eflags);
}

void mutex_init( mutex_t *mut )
{
	memset( mut, 0, sizeof(mutex_t) );
	mut->lock = 1;
}

void mutex_unlock( mutex_t *mut )
{
	//
	uint eflags;
	THREAD* thr = NULL;
	THREAD_LIST* tl;
	local_irq_save( eflags );
	if( mut->list ){
		tl = mut->list;
		mut->list = tl->next;
		thr = tl->thread;
		kfree( tl );
	}
	local_irq_restore( eflags );
//	kprintf("{%d lev}", current_thread()->id );
	mut->lock ++;
	if( thr ){
		thr->sleepon = NULL;
		thread_wakeup( thr );
	}
}

void mutex_destroy( mutex_t *mut )
{
	THREAD_LIST* tl, *tl_next;
	uint eflags;
	local_irq_save( eflags );
	for( tl=mut->list; tl;  ){
		tl_next = tl->next;
		thread_terminate( tl->thread );
		kfree( tl );
		tl = tl_next;
	}
	mut->lock = 0;
	local_irq_restore( eflags );
}

