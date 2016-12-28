#pragma once
#include <deque>
#include <vector>
#include <functional>
#include "Condition.h"

class Thread;
class ThreadPool
{
public:
	typedef std::function<void()>	Task;

public:
	ThreadPool(int minCapacity = 2,
		int maxCapacity = 16,
		int idleTime = 60,
		int stackSize = 0);
	////
	////
	////

	~ThreadPool();

	void start(int numThread);

private:
	void runThred(void *pThis);
	int	_minCapacity;
	int _maxCapacity;
	int _idleTime;
	int _serial;
	int _stackSize;
	Condition				_notEmpty;
	std::deque<Task>		_queue;
	std::vector<Thread*>	_thread;
};