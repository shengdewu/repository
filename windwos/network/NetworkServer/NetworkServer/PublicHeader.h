#pragma once
#include <vector>
#include <string>
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

namespace IOCP_COM{

	//退出完成端口
	#define EXIT_COMPLETE NULL

	enum NET_MSG{
		NET_MSG_NONE = 0,
		NET_MSG_NOERR = 1,
		NET_MSG_CONNECT,
		NET_MSG_DISCONNECT,
		NET_MSG_ERROR,
		NET_MSG_WAITTIME,
	};
	//完成端口常数
	enum COM_CONST{
		Milliseconds_ZERO = 0,
		MAX_ACCEPT_POST = 10,
		MAX_BUFFER_LEN = 8192, //1024 * 8 //8k

	};

	//完成端口上投递的io操作类型
	typedef enum _temOPERATION_TYPE{
		NULL_POSTED = 0,
		ACCEPT_POSTED,
		SEND_POSTED,
		RECV_POSTED,
	}OPERATION_TYPE;

	//临界锁
	class CSynLock{
	public:
		CSynLock(CRITICAL_SECTION *pLock) : m_pSynLock(pLock)
		{
			EnterCriticalSection(m_pSynLock);
		}

		~CSynLock()
		{
			LeaveCriticalSection(m_pSynLock);
		}
	private:
		CRITICAL_SECTION	*m_pSynLock;
	};

	//工作线程参数
	typedef struct _tagThreadParam_Work
	{
		void	   *pThis; //线程上下文
		int			nThreadNo;
		void		*pRes;
	}ThreadParam_Work;

	//单IO数据结构体定义(用于每一个重叠操作的参数)
	typedef struct _tagPER_IO_CONTEXT{
		OVERLAPPED	m_hOverlapped;  //每个重叠网络操作的重叠结构，针对每个socket，都必须要又一个
		WSABUF		m_wsaBuf;       //WSA类型的缓冲区，用于给重叠结构传参数
		char		m_szBuffer[MAX_BUFFER_LEN]; //WSABUF 具体的缓冲区
		SOCKET		m_hSockAccept;      //网络操作所用的
		OPERATION_TYPE	m_tOpType;  //此重叠结构操作类型

		//初始化
		_tagPER_IO_CONTEXT()
		{
			ZeroMemory(&m_hOverlapped, sizeof(m_hOverlapped));
			ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
			m_hSockAccept = INVALID_SOCKET;
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = MAX_BUFFER_LEN;
			m_tOpType =  NULL_POSTED;
		}

		//释放
		~_tagPER_IO_CONTEXT()
		{
			if(INVALID_SOCKET != m_hSockAccept)
			{
				closesocket(m_hSockAccept);
				m_hSockAccept = INVALID_SOCKET;
			}
		}

		void ResetBuffer(void)
		{
			ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
		}
	}PER_IO_CONTEXT;

	//单句柄，即每个完成端口的参数，也是每个sock的参数
	typedef struct _tagPER_SOCKET_CONTEXT
	{
		SOCKET					m_hSocket;      //每个客户端套接字
		SOCKADDR_IN				m_hClientAddr;  //客户端地址
		std::vector<PER_IO_CONTEXT*>	m_arrayIoContext; //每个客户端地应多个io请求

		_tagPER_SOCKET_CONTEXT()
		{
			m_hSocket = INVALID_SOCKET;
			memset(&m_hClientAddr, 0, sizeof(m_hClientAddr));
		}

		~_tagPER_SOCKET_CONTEXT()
		{
			if(INVALID_SOCKET != m_hSocket)
			{
				closesocket(m_hSocket);
				m_hSocket = INVALID_SOCKET;
			}

			for(unsigned int i = 0; i < m_arrayIoContext.size(); i ++)
			{
				delete m_arrayIoContext.at(i);
			}

			m_arrayIoContext.clear();
		}

		//创建一个新的io
		PER_IO_CONTEXT *GetNewIoContext(void)
		{
			PER_IO_CONTEXT *p = new PER_IO_CONTEXT;

			m_arrayIoContext.push_back(p);

			return p;
		}

		//移除指定的io
		void RemoveIoContext(PER_IO_CONTEXT *pContext)
		{
			if(NULL == pContext){
				return;
			}

			std::vector<PER_IO_CONTEXT*>::iterator it = m_arrayIoContext.begin();
			for(; it != m_arrayIoContext.end(); it++)
			{
				if(*it == pContext)
				{
					delete pContext;
					it = m_arrayIoContext.erase(it);
					break;
				}
			}
		}
	}PER_SOCKET_CONTEXT;

}