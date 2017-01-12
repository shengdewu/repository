#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#include "SocketImp.h"

SocketImp::SocketImp():
	SOCKET_ERR(-1),
	MAX_BACKLOG(10)
{
	_fd = SOCKET_ERR;
}

SocketImp::~SocketImp()
{
	if(SOCKET_ERR != _fd)
		::close(_fd);
}

int SocketImp::setOpt(const int fd)
{
	if(SOCKET_ERR == fd)
	{
		return SOCKET_ERR;
	}

	int flags = fcntl(fd, F_GETFL, 0);

	if(SOCKET_ERR == flags)
	{
		printf("fcntl F_GETFL on descriptor %d\n", fd);
		return SOCKET_ERR;
	}

	//设置为非阻塞
	flags |= O_NONBLOCK;
	flags = fcntl(fd, F_SETFL, flags);

	if(SOCKET_ERR == flags)
	{
		printf("fcntl F_SETFL on descriptor %d\n", fd);
		return SOCKET_ERR;
	}

	return 0;
}

int SocketImp::listen(const char *ip, const int port, int backlog)
{
	_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(SOCKET_ERR == _fd)
	{
		return SOCKET_ERR;
	}
	
	int nret = -1;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	socklen_t sellen = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	nret = ::bind(_fd, (sockaddr*)(&server_addr), sellen);
	if(SOCKET_ERR == nret)
	{

		return SOCKET_ERR;
	}

	if(SOCKET_ERR == setOpt(_fd))
	{
		return SOCKET_ERR;
	}

	backlog =(backlog == 0)? MAX_BACKLOG : backlog; 
	nret = ::listen(_fd, backlog);	
	if(SOCKET_ERR == nret)
	{

		return SOCKET_ERR;
	}

	return _fd;

}

int SocketImp::accept(const int fd)
{
	struct sockaddr_in client;
	socklen_t len = sizeof(struct sockaddr_in);
	int cfd = ::accept(fd, (struct sockaddr*)(&client), &len);
	if(SOCKET_ERR == cfd)
	{
		return SOCKET_ERR;
	}


	setOpt(cfd);

	return cfd;
}

int SocketImp::send(const int fd, const char *pbuff, const int len)
{
	return ::send(fd, pbuff, len, 0);
}

int SocketImp::read(const int fd, char *pbuff, const int len)
{
	return ::recv(fd, pbuff, len, 0);
}

void SocketImp::close(const int fd)
{
	::close(fd);
}
