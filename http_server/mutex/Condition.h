#pragma once
#include <pthread.h>

class Condition
{
public:
	Condition(bool autoRest = true);
	~Condition(void);

	void wait();
	////此操作带有锁，
	////避免pthread_cond_broadcast 释放所有的条件变量

	void notify();

	void notifyAll();

private:
	pthread_cond_t _cond;
	pthread_mutex_t _mutex;
	volatile bool _state;
	bool		  _auto;
};

