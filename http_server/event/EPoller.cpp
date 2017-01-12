#include "EPoller.h"
#include "Channel.h"
#include <sys/epoll.h>
#include "Logger.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

namespace 
{
	const int KADD = -1;
	const int KMOD = 1;
	const int KDELETE = 2;
}

EPoller::EPoller(void):
	_epfd(::epoll_create1(EPOLL_CLOEXEC)),
	_events(KEVENTS_INITS)
{
}


EPoller::~EPoller(void)
{
	Logger::releseLogger("epoll.log");
}

int	 EPoller::poll(int timeout, ChannelList& activeChannel)
{
	activeChannel.clear();
	_events.clear();
	int numEvents = ::epoll_wait(_epfd, &(*_events.begin()), _events.size(), timeout);
	if(numEvents < 0)
	{
		logger_log(Logger::getLogger("epoll.log"), "poll failed:");
		(*Logger::getLogger("epoll.log"))<<"errno = " << errno << strerror(errno);
		return numEvents;
	}

	if(numEvents > 0)
	{
		fillChannelEvents(numEvents, activeChannel);
		if(numEvents >= _events.size())
		{
			_events.resize(numEvents << 1);
		}
	}

	return numEvents;
}

void EPoller::updateChannel(Channel *pChannel)
{
	if(nullptr == pChannel)
	{
		return;
	}

	int index = pChannel->index();
	int fd = pChannel->fd();
	ChannelMapIter iter = _channelMap.find(fd);
	int opt = 0;
	if(iter == _channelMap.end() && KADD == index) //新增
	{
		opt = EPOLL_CTL_ADD;
		ScopedMutexLock lock(&_mutex);
		_channelMap[fd] = pChannel;
	}
	else if(iter != _channelMap.end() && KADD != index) //修改或删除
	{
		if(KDELETE ==index)
		{
			opt = EPOLL_CTL_DEL;
			ScopedMutexLock lock(&_mutex);
			_channelMap.erase(iter);
		}
		else
		{
			opt = EPOLL_CTL_MOD;
		}
	}
	else
	{
		std::string strlog = createlogger("epoll updateChannel is invalid :", pChannel);
		logger_log(Logger::getLogger("epoll.log"), strlog);
		return;
	}

	update(opt, pChannel);
}

void EPoller::update(int opt, Channel *pChannel)
{
	struct epoll_event ev;
	bzero(&ev, sizeof(struct epoll_event));
	ev.events = pChannel->events();
	ev.data.ptr = pChannel;
	//ev.events |= EPOLLET;
	int fd = pChannel->fd();
	if(::epoll_ctl(_epfd, opt, fd, &ev) < 0)
	{
		std::string strlog = createlogger(strlog, pChannel);
		(Logger::getLogger("epoll.log"))->log("epoll update is failed:", __FILE__, __LINE__) 
				<< "opt = " << eventToString(opt) 
				<< ","<<strlog;
		
	}
}

void EPoller::fillChannelEvents(int num, ChannelList& activeChannel)
{
	if(num > _events.size())
	{
		return;
	}

	(Logger::getLogger("epoll.log"))->log("fillChannelEvents:", __FILE__, __LINE__) << "nums = " << num;

	for(int i=0; i<num; i++)
	{
		Channel* pChannel = static_cast<Channel*>(_events[i].data.ptr);
		pChannel->setRevent(_events[i].events);
		activeChannel.push_back(pChannel);
	}
}


std::string EPoller::createlogger(std::string strContext, Channel *pChannel)
{
	std::string strlog = strContext;
	std::string strtmp;
	std::stringstream s;
	s << pChannel->index();
	s >> strtmp;
	strlog.append("index = ").append(strtmp);
	strtmp.clear();
	s << pChannel->fd();
	s >> strtmp;
	strlog.append(",fd = ").append(strtmp);

	return strlog;
}

std::string EPoller::eventToString(int opt)
{
	switch(opt)
	{
		case EPOLL_CTL_ADD:
		  return "ADD";
		case EPOLL_CTL_DEL:
		  return "DEL";
		case EPOLL_CTL_MOD:
		  return "MOD";
		default:
		  return "Unknown Operation";
	}
}

