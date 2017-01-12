#pragma once
class EPoller;
class Channel;
class EventLoop
{
public:
	EventLoop(void);
	~EventLoop(void);

	void updateChannel(Channel *pChannel);

private:
	EPoller  *_ePoll;
};

