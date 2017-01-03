#pragma once
#include <deque>
#include <vector>
#include <functional>
#include "MutexLock.h"

class PooledThread;
class ThreadPool
{
public:
	//typedef void (*Task)(void *);
	typedef std::function<void()> Task;
	////外部定义
	//// std::function<void()> func  = 
	////  std::bind(&obj::func, 某个对象的成员函数
	////            obj,   对象
	////            this,  对象指针
	////            std::placeholders::_1, 
	////            std::placeholders::_2);

public:
	ThreadPool(int minCapacity = 2,
		int maxCapacity = 16,
		int idleTime = 60,
		int stackSize = 0);
	////
	////
	////

	~ThreadPool();

	bool start(Task task);
	void stopAll();
	void joinAll();
	void collect();
	int  allocate();
private:
	typedef std::vector<PooledThread*>	ThreadVec;
	typedef std::vector<PooledThread*>::iterator ThreadVecIter;
	typedef std::deque<Task>			TaskDeque;
	
	PooledThread *getThread();
	void houseKeep();

	int	_minCapacity;
	int _maxCapacity;
	int _idleTime;
	int _serial;
	int _stackSize;
	int _age;
	///控制收集线程的频率

	TaskDeque		_queue;
	ThreadVec		_thread;
	MutexLock		_mutex;
};