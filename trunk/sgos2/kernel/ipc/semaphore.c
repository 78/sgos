//semaphore

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <thread.h>
#include <semaphore.h>
#include <mm.h>


//尝试P
int sema_trydown( sema_t *sem )
{	
	uint eflags;
	//we don't want to be interrupt
	local_irq_save( eflags );
	while( --sem->value ){
		//don't have it..
		local_irq_restore(eflags);
		sem->value ++;
		return 0;
	}
	//have it
	local_irq_restore(eflags);
	return 1;
}

//P
void sema_down( sema_t *sem )
{
	uint eflags;
	THREAD* thr;
	THREAD_LIST* tl, *tl2;
	//we don't want to be interrupt
	local_irq_save( eflags );
	if( --sem->value ){
		//dont have it, then sleep
		local_irq_restore( eflags );
		tl = (THREAD_LIST*)kmalloc(sizeof(THREAD_LIST));
		tl->thread = thr = (THREAD*)current_thread();
		local_irq_save( eflags );
		//跟队末尾
		tl2 = sem->list;
		if(tl2){
			while( tl2->next )
				tl2 = tl2->next;
			tl2->next = tl;
		}else{
			sem->list = tl;
		}
		tl->next = NULL;
		local_irq_restore(eflags);
		thread_sleep();
		//now, we have it!!
	}
	local_irq_restore(eflags);
}

//初始化
void sema_init( sema_t *sem )
{
	memset( sem, 0, sizeof(sema_t) );
	sem->value = 1;
}

//V
void sema_up( sema_t *sem )
{
	uint eflags;
	THREAD* thr = NULL;
	THREAD_LIST* tl;
	local_irq_save( eflags );
	if( sem->list ){
		tl = sem->list;
		sem->list = tl->next;
		thr = tl->thread;
		kfree( tl );
	}
	local_irq_restore( eflags );
	sem->value ++;
	if( thr )
		thread_wakeup( thr );
}

//释放一个sema
void sema_destroy( sema_t *sem )
{
	THREAD_LIST* tl, *tl_next;
	THREAD* thr;
	uint eflags;
	local_irq_save( eflags );
	for( tl=sem->list; tl;  ){
		tl_next = tl->next;
		thr = tl->thread;
		kfree( tl );
		tl = tl_next;
		thread_terminate( thr, -1 );
	}
	sem->value = 0;
	local_irq_restore( eflags );
}

//删除一个链表中的项
void sema_remove_thread( sema_t* sem, THREAD* thr )
{
	THREAD_LIST* tl;
	uint eflags;
	local_irq_save(eflags);
	if( sem->list ){
		if( thr==sem->list->thread ){//简单情况
			kfree( sem->list );
			sem->list = NULL;
		}else{ //复杂情况
			for(tl=sem->list; tl->next; tl=tl->next ){
				if( tl->next->thread == thr ){
					//catched it
					THREAD_LIST* tl2;
					tl2 = tl->next;
					tl->next = tl->next->next;
					kfree(tl->next);
					break;
				}
			}
			PERROR("##thread not found.");
		}
	}
	local_irq_restore(eflags);
}
