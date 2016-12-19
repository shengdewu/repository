#include "MutexLock.h"
#include <cstring>

#include "Exception.h"

MutexLock::MutexLock(void)
{
	std::memset(&_mutex, 0, sizeof(_mutex));
	if(pthread_mutex_init(&_mutex, nullptr))
		throw Exception("MutexLock", "create mutex failed");
}

MutexLock::~MutexLock(void)
{
	pthread_mutex_destroy(&_mutex);
}

void MutexLock::lock()
{
	if(pthread_mutex_lock(&_mutex))
		throw Exception("MutexLock", "wait mutex failed");
}

void MutexLock::unlock()
{
	if(pthread_mutex_unlock(&_mutex))
		throw Exception("MutexLock", "release mutex failed");
}