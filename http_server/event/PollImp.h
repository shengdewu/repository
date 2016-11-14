#pragma once
#include <sys/epoll.h>
#include <vector>
#include "Event.h"

class PollImp
{
public:
	typedef std::vector<Event> EvList;
private:
	typedef std::vector<struct epoll_event> EpList;
public:
	PollImp();
	~PollImp();
	
	int Init(const int max = 0);
	int Poll(int timeout, EvList &evl);
	int Update(Event *ev);
private:
	bool IsAvaild();

private:
	const int MAX_EVENT;
	const int POLL_ERR;

private:
	int  		m_fd;
	EpList		m_EpList;
};
