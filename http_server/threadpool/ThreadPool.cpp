#include "ThreadPool.h"

ThreadPool::ThreadPool(int minCapacity,
	int maxCapacity,
	int idleTime,
	int stackSize):
	_minCapacity(minCapacity), 
	_maxCapacity(maxCapacity), 
	_idleTime(idleTime),
	_serial(0),
	_stackSize(stackSize)
{

}

ThreadPool::~ThreadPool()
{

}