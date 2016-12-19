#pragma once

class ThreadPool
{
public:
	ThreadPool(int minCapacity = 2,
		int maxCapacity = 16,
		int idleTime = 60,
		int stackSize = 0);
	////
	////
	////

	~ThreadPool();

private:
	int	_minCapacity;
	int _maxCapacity;
	int _idleTime;
	int _serial;
	int _stackSize;
};