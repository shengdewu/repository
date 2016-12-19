#pragma once

class Runable
{
public:
	Runable();
	virtual ~Runable();

	virtual void run() = 0;
};