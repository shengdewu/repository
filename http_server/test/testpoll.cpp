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

	std::map<int, Event> availEvent;

	PollImp p;
	p.Init();
	Event ev(lfd, EventConst::EVENT_ADD, EPOLLIN);
	p.Update(&ev);
	
	availEvent.insert(std::pair<int, Event>(lfd, ev));

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
				Event ev(cfd, EventConst::EVENT_ADD, EPOLLIN);
				availEvent.insert(std::pair<int, Event>(cfd, ev));
				p.Update(&ev);
			}
			else
			{
				memset(pbuffer, '\0', nsize);
				int num = server.Read(elist[i].Getfd(), pbuffer, nsize);
				if(num == 0)
				{
					std::cout << "the client" << elist[i].Getfd() << "is disconect"<<std::endl;
					Event ev(elist[i].Getfd(), EventConst::EVENT_DEL);
					std::map<int, Event>::iterator it = availEvent.find(elist[i].Getfd());
					if(it != availEvent.end())
						availEvent.erase(it);
					p.Update(&ev);
				}
				else
				{
				        std::cout << "the server receive bytes = " << num << "the context = " << pbuffer << std::endl;
					server.Send(elist[i].Getfd(), pbuffer, num);
				}
			}
		}
	}

	return 0;
}
