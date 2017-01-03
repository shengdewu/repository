#pragma once
#include <pthread.h>

class Condition
{
public:
	Condition(bool autoRest = true);
	~Condition(void);

	void wait();
	////�˲�����������
	////����pthread_cond_broadcast �ͷ����е���������

	void notify();

	void notifyAll();

	void reset();

private:
	pthread_cond_t _cond;
	pthread_mutex_t _mutex;
	volatile bool _state;
	bool		  _auto;
};

