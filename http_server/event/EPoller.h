#pragma once
#include <vector>
#include <map>
#include "ScopedMutexLock.h"

struct epoll_event;
class Channel;
class EPoller
{
	typedef std::vector<Channel*>  ChannelList;
	typedef std::map<int, Channel*> ChannelMap;
	typedef std::map<int, Channel*>::iterator  ChannelMapIter;
	typedef std::vector<struct epoll_event>	EventList;
public:
	EPoller(void);
	~EPoller(void);

	int	 poll(int timeout, ChannelList& activeChannel);

	void updateChannel(Channel *pChannel);

private:
	void update(int opt, Channel *pChannel);
	void fillChannelEvents(int num, ChannelList& activeChannel);
	std::string createlogger(std::string strContext, Channel *pChannel);
	std::string eventToString(int opt);
	
	static const int	KEVENTS_INITS = 16;

	int			_epfd;
	EventList	_events;
	MutexLock	_mutex;
	ChannelMap	_channelMap;
};

