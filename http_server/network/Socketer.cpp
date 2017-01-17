#include "Socketer.h"
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
#include "Logger.h"
#include <string>
#include "Logger.h"

Socketer::Socketer(void):
	SOCKET_ERR(-1),
	MAX_BACKLOG(10)
{
	_fd = SOCKET_ERR;
}

Socketer::~Socketer(void)
{
	if(SOCKET_ERR != _fd)
		::close(_fd);

	Logger::releseLogger(LOGGER_NAME);
}


int Socketer::listen(const char *ip, const int port, int backlog)
{
	_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(SOCKET_ERR == _fd)
	{
		LOGGER(Logger::getLogger(LOGGER_NAME)) << "The Socketer is socketing that is failed"
			<< ":ip = " << ip << ",port:" << port << "\n";
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

int Socketer::accept()
{
	if(SOCKET_ERR == _fd)
	{
		return SOCKET_ERR;
	}

	struct sockaddr_in client;
	socklen_t len = sizeof(struct sockaddr_in);
	int cfd = ::accept(_fd, (struct sockaddr*)(&client), &len);
	if(SOCKET_ERR == cfd)
	{
		return SOCKET_ERR;
	}

	setOpt(cfd);

	return cfd;
}

int Socketer::send(const int fd, const char *pbuff, const int len)
{
	if(SOCKET_ERR == fd)
	{
		return SOCKET_ERR;
	}
	return ::send(fd, pbuff, len, 0);
}

int Socketer::read(const int fd, char *pbuff, const int len)
{
	if(SOCKET_ERR == fd)
	{
		return SOCKET_ERR;
	}
	return ::recv(fd, pbuff, len, 0);
}

void Socketer::close(const int fd)
{
	if(SOCKET_ERR == fd)
	{
		return;
	}
	::close(fd);
}

int Socketer::setOpt(const int fd)
{
	if(SOCKET_ERR == fd)
	{
		return SOCKET_ERR;
	}

	int flags = fcntl(fd, F_GETFL, 0);

	if(SOCKET_ERR == flags)
	{
		logger_log(Logger::getLogger(LOGGER_NAME), "The Socketer is setOpting that is failed");
		return SOCKET_ERR;
	}

	//ÉèÖÃÎª·Ç×èÈû
	flags |= O_NONBLOCK;
	flags = fcntl(fd, F_SETFL, flags);

	if(SOCKET_ERR == flags)
	{
		logger_log(Logger::getLogger(LOGGER_NAME), "The Socketer is setOpting that is failed");
		return SOCKET_ERR;
	}

	return 0;
}

