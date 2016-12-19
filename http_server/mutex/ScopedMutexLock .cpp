#include "ScopedMutexLock.h"

ScopedMutexLock::ScopedMutexLock(MutexLock *pLock):
	_pLock(pLock)
{
	if(_pLock)
	{
		_pLock->lock();
	}
}

ScopedMutexLock::~ScopedMutexLock()
{
	if(_pLock)
	{
		_pLock->unlock();
	}
}