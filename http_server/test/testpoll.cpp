#include "SocketImp.h"
#include "PollImp.h"
#include <stdlib.h>
#include <vector>

int main(int argc, char *argv[])
{
	int port = atoi(argv[2]);
	char *ip = argv[1];

	SocketImp  server;
	int lfd = server.Listen(ip, port);

	PollImp p;
	p.Init();
	Event ev(lfd, EventConst::EVENT_ADD, EPOLLIN);
	p.Update(&ev);

	const int  nsize = 1024 * 1024;
	char pbuffer[nsize] = {'\0'};

	std::vector<Event> elist(20);	
	while(true)
	{
		int num = p.Poll(500, elist);
		for(int i=0; i<num; i++)
		{
			if(lfd == elist[i].Getfd())
			{
				int cfd = server.Accept(lfd);
				Event ev(cfd, EventConst::EVENT_ADD, EPOLLIN);
				p.Update(&ev);
			}
			else
			{
				int num = server.Read(elist[i].Getfd(), pbuffer, nsize);
				if(num == 0)
				{
					Event ev(elist[i].Getfd(), EventConst::EVENT_DEL);
					p.Update(&ev);
				}
				else
				{
					server.Send(elist[i].Getfd(), pbuffer, num);
				}
			}
		}
	}

	return 0;
}
