#ifndef _THREAD_H_
#define _THREAD_H_

enum ThreadState{
	
};

class Thread{
private:
	//线程句柄
	uint handle;
	//结束码
	uint exitCode;
	//执行函数
	void* procedure;
public:
	Thread();
	Thread(void* proc);
	~Thread();
	/* 静态函数 */
	//返回当前线程
	static Thread ThisThread();
	//当前线程等待
	static void Sleep(uint ms);
	//结束当前线程
	static void Exit(uint code);
	/* 非静态函数 */
	// 终止线程
	void abort();
	// 开始线程
	void resume();
	void start();
	// 挂起线程
	void suspend();
	// 等待线程结束
	void join();
	// 获取句柄
	uint getHandle();
	
	//命名系统
	int createName( const char* name );
	void deleteName( const char* name );
	static Thread find( const char* name );
};

#endif
