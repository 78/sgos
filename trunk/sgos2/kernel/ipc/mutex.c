//mutex

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <thread.h>
#include <mutex.h>
#include <mm.h>


//尝试
int mutex_trylock( mutex_t *mut )
{	
	uint eflags;
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

//请求
void mutex_lock( mutex_t *mut )
{
	uint eflags;
	THREAD* thr;
	THREAD_LIST* tl, *tl2;
	//we don't want to be interrupt
	local_irq_save( eflags );
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
		thread_sleep();
		//已经有人帮我们从mut的线程列表里拆除了
		local_irq_save( eflags );
	}
	local_irq_restore(eflags);
}

//初始化
void mutex_init( mutex_t *mut )
{
	memset( mut, 0, sizeof(mutex_t) );
	mut->lock = 1;
}

//解除
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
	mut->lock ++;
	if( thr ){
		thr->sleepon = NULL;
		thread_wakeup( thr );
	}
}

//释放一个mutex
void mutex_destroy( mutex_t *mut )
{
	THREAD_LIST* tl, *tl_next;
	THREAD* thr;
	uint eflags;
	local_irq_save( eflags );
	for( tl=mut->list; tl;  ){
		tl_next = tl->next;
		thr = tl->thread;
		kfree( tl );
		tl = tl_next;
		thread_terminate( thr, -1 );
	}
	mut->lock = 0;
	local_irq_restore( eflags );
}

//删除一个链表中的项
void mutex_remove_thread( mutex_t* mut, THREAD* thr )
{
	THREAD_LIST* tl;
	uint eflags;
	local_irq_save(eflags);
	if( mut->list ){
		if( thr==mut->list->thread ){//简单情况
			kfree( mut->list );
			mut->list = NULL;
		}else{ //复杂情况
			for(tl=mut->list; tl->next; tl=tl->next ){
				if( tl->next->thread == thr ){
					//catched it
					THREAD_LIST* tl2;
					tl2 = tl->next;
					tl->next = tl->next->next;
					kfree(tl->next);
					break;
				}
			}
			PERROR("thread not found.");
		}
	}
	local_irq_restore(eflags);
}
