#include "PollImp.h"
#include <strings.h>
#include <unistd.h>

PollImp::PollImp():
	MAX_EVENT(1024),
	POLL_ERR(-1),
	m_EpList(256)
{
	m_fd = POLL_ERR;
}

PollImp::~PollImp()
{
	if(POLL_ERR != m_fd)
		close(m_fd);
}

bool PollImp::IsAvaild()
{
	return (POLL_ERR == m_fd) ? false : true;
}

int PollImp::Init(const int max)
{
	if(IsAvaild())
	{
	   return m_fd;
	}

	int num =((0 == max) ? MAX_EVENT : (max > MAX_EVENT ? MAX_EVENT : max));
	m_fd = epoll_create(num);

	return m_fd;
}

int PollImp::Poll(int timeout, EvList &evl)
{
	if(!IsAvaild())
	{
		return POLL_ERR;
	}
	int num = epoll_wait(m_fd, &*(m_EpList.begin()), m_EpList.size(), timeout);
	if(num <= 0 )
		return POLL_ERR; 

	for(int i=0; i<num; i++)
	{
		Event *pEv = reinterpret_cast<Event*>(m_EpList[i].data.ptr);
		pEv->SetEvent(m_EpList[i].events);
		Event ev(*pEv);
		evl.push_back(ev);
	}	

	if((unsigned int)num >= m_EpList.size())
	{
		int rsize = m_EpList.size() << 1; 
		m_EpList.resize(rsize);
	}

	return num;
}

int PollImp::Update(Event *ev)
{
	if(!IsAvaild())
	{
		return POLL_ERR;
	}

	struct epoll_event eev;
	bzero(&eev, sizeof(eev));

	eev.data.ptr = ev;
	eev.events = ev->GetEvent();
	int fd = ev->Getfd();
	int opt = EPOLL_CTL_ADD;
	switch(ev->GetOpt())
	{
		case EventConst::EVENT_ADD:
			opt = EPOLL_CTL_ADD;
			break;
		case EventConst::EVENT_DEL:
			opt = EPOLL_CTL_DEL;
			break;
		case EventConst::EVENT_MODIFY:
			opt = EPOLL_CTL_MOD;
			break;
		default:
			return POLL_ERR;
			break;
	}

	eev.events |= EPOLLET;
	int ret = 0;
	if(EPOLL_CTL_DEL == opt)
	{
		ret = epoll_ctl(m_fd, opt, fd, NULL);
	}
	else
	{
		ret = epoll_ctl(m_fd, opt, fd, &eev);
	}
	
	return ret; 
}

