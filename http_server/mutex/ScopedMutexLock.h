#pragma once

#include "MutexLock.h"

class ScopedMutexLock
{
public:
	ScopedMutexLock(MutexLock *pLock);
	~ScopedMutexLock();

private:
	MutexLock *_pLock;
};