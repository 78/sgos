#ifndef _APIDEF_H_
#define _APIDEF_H_

#include <types.h>

//system call for SGOS2
//调试
SYSCALL0( 0, uint, test );
SYSCALL1( 1, int, dprint, const char*, buf );
//消息处理
SYSCALL4( 2, int, send, void*, session, void*, content, size_t, len, uint, flag );
SYSCALL4( 3, int, recv, void*, session, void*, content, size_t*, siz, uint, flag );
//内存管理
SYSCALL1( 4, void*, virtual_alloc, size_t, siz );
SYSCALL1( 5, void, virtual_free, void*, p );
//线程管理
SYSCALL1( 6, void, thread_exit, int, code );	//退出当前线程
SYSCALL2( 7, int, thread_create, size_t, addr, uint*, thread );	//创建线程，成功则返回线程id退出当前线程
SYSCALL0( 8, uint, thread_self );	//返回当前线程ID
SYSCALL1( 9, int, thread_detach, uint, thread );	//脱离线程
SYSCALL2( 10, int, thread_join, uint, thread, int*, code );	//等待线程结束
SYSCALL1( 11, int, thread_wait, time_t, ms );	//线程睡眠一段时间
SYSCALL1( 12, int, thread_suspend, uint, thread );	//挂起线程
SYSCALL1( 13, int, thread_resume, uint, thread );	//启动线程
SYSCALL2( 14, int, thread_terminate, uint, thread, int, code );	//结束线程
SYSCALL2( 15, int, thread_set_priority, uint, thread, int, pri );	//设置线程优先级
SYSCALL2( 16, int, thread_get_priority, uint, thread, int*, pri );	//获取线程优先级
SYSCALL1( 17, int, thread_semget, int, value );	//获取信号灯
SYSCALL2( 18, int, thread_semop, int, sem, int, op );	//操作信号灯
SYSCALL2( 19, int, thread_semctl, int, sem, int, cmd );	//设置信号灯
//进程管理
SYSCALL4( 20, int, process_create, char*, file, void*, env, void*, cinfo, uint*, proc );
SYSCALL2( 21, int, process_terminate, uint, proc, int, code );	//结束进程
SYSCALL1( 22, int, process_suspend, uint, proc );	//挂起进程 
SYSCALL1( 23, int, process_resume, uint, proc );	//启动进程
SYSCALL0( 24, uint, process_self );	//当前进程ID
//加载器
SYSCALL2( 25, int, loader_load, char*, file, uint*, mod );	//返回库id
SYSCALL1( 26, int, loader_unload, uint, mod );	//卸载库
SYSCALL2( 27, size_t, loader_get_proc, uint, mod, char*, name );	//获得过程

//命名空间
SYSCALL2( 28, int, namespace_register, uint, thread, char*, name );
SYSCALL2( 29, int, namespace_unregister, uint, thread, char*, name );
SYSCALL1( 30, uint, namespace_match, char*, name );
//....
//特权调用
SYSCALL2( 35, int, iomap_get, uchar*, buf, size_t, buf_size );
SYSCALL2( 36, int, iomap_set, uchar*, buf, size_t, buf_len  );
SYSCALL2( 37, int, irq_register, int, tid, int, irq );
SYSCALL2( 38, int, irq_unregister, int, tid, int, irq );
SYSCALL3( 39, int, vm_map, size_t, vaddr, size_t, paddr, size_t, map_size );

#endif //_APIDEF_H_
