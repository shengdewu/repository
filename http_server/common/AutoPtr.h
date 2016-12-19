#pragma once
#include "Exception.h"

template <class T>
class AutoPtr
{
public:
	AutoPtr(T *ptr):_ptr(ptr)
	{
	}

	~AutoPtr()
	{
		if(_ptr)
			delete _ptr;
	}

	T *operator->()
	{
		if(_ptr)
			return _ptr;

		throw Exception("null pointer exception");
	}

	T operator*()
	{
		if(_ptr)
			return *_ptr;
		throw Exception("null pointer exception");
	}

	T *get()
	{
		return _ptr;
	}

	bool &operator==(T *pt)
	{
		if(_ptr == pt)
		{
			return true;
		}

		return false;
	}
private:
	T	*_ptr;
};