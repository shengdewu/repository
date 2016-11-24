#include "SocketImp.h"
#include "PollImp.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>

int main(int argc, char *argv[])
{
	int port = atoi(argv[2]);
	char *ip = argv[1];

	SocketImp  server;
	int lfd = server.Listen(ip, port);
	if(-1 == lfd)
	{
	   std::cout << "the server listen is failed!" << std::endl;
	}

	std::map<int, Event*> availEvent;

	PollImp p;
	p.Init();
	Event *ev = new Event(lfd, EventConst::EVENT_ADD, EPOLLIN);
	p.Update(ev);
	
	availEvent.insert(std::pair<int, Event*>(lfd, ev));

	const int  nsize = 1024 * 1024;
	char pbuffer[nsize] = {'\0'};

	std::vector<Event> elist;	
	while(true)
	{
		elist.clear();
		int num = p.Poll(500, elist);
		if(num <= 0)
		{
			continue;
		}
		std::cout << "the fd size = " << num << std::endl;
		for(int i=0; i<num; i++)
		{
			if(lfd == elist[i].Getfd())
			{
				int cfd = server.Accept(lfd);
				if(cfd < 0 )
				{
					std::cout << "the server accept is failed!" << cfd << std::endl;	
			                continue;
				}
				else
				{
					std::cout << "the server accpet is success!" << cfd << std::endl;
				}
				Event *ev = new Event(cfd, EventConst::EVENT_ADD, EPOLLIN);
				availEvent.insert(std::pair<int, Event*>(cfd, ev));
				p.Update(ev);
			}
			else
			{
				memset(pbuffer, '\0', nsize);
				int num = server.Read(elist[i].Getfd(), pbuffer, nsize);
				if(num == 0)
				{
					std::cout << "the client " << elist[i].Getfd() << "is disconect"<<std::endl;
					Event ev(elist[i].Getfd(), EventConst::EVENT_DEL);
					p.Update(&ev);
					server.Close(elist[i].Getfd());
				}
				else if(num < 0)
				{
					server.Close(elist[i].Getfd());
					std::cout << "the sever receive error " << elist[i].Getfd()
					<<strerror(errno)<<std::endl;
				}
				else
				{
				        std::cout << "the server receive " << elist[i].Getfd() <<" bytes = "
						 << num << "the context = " << pbuffer << std::endl;
					server.Send(elist[i].Getfd(), pbuffer, num);
				}
			}
		}
	}

	return 0;
}
