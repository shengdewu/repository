#include "Condition.h"
#include <cstring>

#include "Exception.h"

Condition::Condition(bool autoRest):
	_state(false),
	_auto(autoRest)
{
	std::memset(&_cond, 0, sizeof(_cond));
	std::memset(&_mutex, 0, sizeof(_mutex));
	if(pthread_cond_init(&_cond, nullptr))
		throw Exception("init condition failed");
	if(pthread_mutex_init(&_mutex, nullptr))
		throw Exception("init mutex failed");
}


Condition::~Condition(void)
{
	pthread_cond_destroy(&_cond);
	pthread_mutex_destroy(&_mutex);
}

void Condition::wait()
{
	if(pthread_mutex_lock(&_mutex))
		throw Exception("wait for condition failed(lock)");
	while(!_state) //避免惊群效应
	{
		if(pthread_cond_wait(&_cond, &_mutex))
		////一旦进入wait _mutex 自动被释放
		////唤醒后自动 获取_mutex
		{
			pthread_mutex_unlock(&_mutex);
			throw Exception("wait for condition failed");
		}
	}
	if(_auto)
		_state = false;
	pthread_mutex_unlock(&_mutex);
}

void Condition::notify()
{
	if(pthread_mutex_lock(&_mutex))
		throw Exception("wait for condition failed(lock)");
	_state = true;
	if(pthread_cond_signal(&_cond))
	{
		throw Exception("reset for condition failed");
	}
	pthread_mutex_unlock(&_mutex);
}

void Condition::notifyAll()
{
	if(pthread_mutex_lock(&_mutex))
		throw Exception("wait for condition failed(lock)");
	_state = true;
	if(pthread_cond_broadcast(&_cond))
	{
		throw Exception("reset for condition failed");
	}
	pthread_mutex_unlock(&_mutex);

}

void Condition::reset()
{
	if(pthread_mutex_lock(&_mutex))
		throw Exception("wait for condition failed(lock)");
	_state = false;
	pthread_mutex_unlock(&_mutex);
}