#pragma once

#include <pthread.h>
class MutexLock
{
public:
	MutexLock(void);
	~MutexLock(void);

	void lock();
	void unlock();

private:
	pthread_mutex_t	 _mutex;
};

