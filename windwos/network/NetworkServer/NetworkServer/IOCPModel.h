#pragma once
#include <vector>
#include <string>
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

class CIOCPModel;

namespace IOCP_COM{
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
		CIOCPModel	*pIOCPModel;
		int			nThreadNo;
		void		*pRes;
	}ThreadParam_Work;

	//单IO数据结构体定义(用于每一个重叠操作的参数)
	typedef struct _tagPER_IO_CONTEXT{
		OVERLAPPED	m_hOverlapped;  //每个重叠网络操作的重叠结构，针对每个socket，都必须要又一个
		WSABUF		m_wsaBuf;       //WSA类型的缓冲区，用于给重叠结构传参数
		char		m_szBuffer[MAX_BUFFER_LEN]; //WSABUF 具体的缓冲区
		SOCKET		m_hSocket;      //网络操作所用的
		OPERATION_TYPE	m_tOpType;  //此重叠结构操作类型

		//初始化
		_tagPER_IO_CONTEXT()
		{
			ZeroMemory(&m_hOverlapped, sizeof(m_hOverlapped));
			ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
			m_hSocket = INVALID_SOCKET;
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = MAX_BUFFER_LEN;
			m_tOpType =  NULL_POSTED;
		}

		//释放
		~_tagPER_IO_CONTEXT()
		{
			if(INVALID_SOCKET != m_hSocket)
			{
				closesocket(m_hSocket);
				m_hSocket = INVALID_SOCKET;
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

class CIOCPModel
{
public:
	CIOCPModel(void);
	~CIOCPModel(void);

	//开启完成端口服务
	//pSvrIp : 监听ip 默认本机ip
	//nPort  ： 监听端口 m默认12345
	//成功返回0，失败返回错误码
	long Start(const unsigned short nPort = 0, const char *pSvrIp = nullptr);

	//停止监听
	void Stop();
private:
	
	//初始化网络库,在开始网络初始化之前必须调用，构造函数里已经调用
	//设置标志，成功返回true，否则false
	bool LoadSocketLib();

	//释放网络库，在关闭所有连接后在调用，析构函数里已经调用
	void UnloadSocketLib();

	//初始化完成端口及线程
	//nThread : 工作线程个数，nThread 等于0是，默认为cpu内核个数 × 2
	//成功返回0，失败返回错误码
	long InitCompeletionPort(int nThread = 0);

	//初始化监听
	//pSvrIp : 监听ip
	//nPort  ： 监听端口
	//成功返回0，失败返回错误码
	long InitListen(const char *pSvrIp, const int nPort);

	//获取本机处理器个数
	long GetProcessNum();

	//获取本机ip
	std::string GetLocalIP(void);

	//线程处理函数
	static DWORD __stdcall _WorkThread(LPVOID lpParam);

	//日志
	void printDegug(const char *pInfo, const int nResult, bool bFlag = false);

	//获取AcceptEx和GetAcceptExSockaddrs函数指针
	bool GetlpfnAccept(SOCKET &hSock);

	//初始化失败后，释放资源
	void ReleaseIOCP();

	//删除指针
	void Release(void *p);

	//投递AcceptEx请求
	bool PostAccept(IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext);

	//投递Recv请求

	//清除客户端列表
	void ClearContextList(void);

private:
	//常量
	enum{
		EN_MAX_MUTIL = 2,
	};
	//完成端口
	HANDLE	m_hIoCompletionPort;
	//工作线程局部
	HANDLE	*m_phWorkHandle;
	//工作线程退出事件
	HANDLE	m_hExitHandle;

	//服务器监听套接字
	IOCP_COM::PER_SOCKET_CONTEXT			*m_phListenContext;
	std::vector<IOCP_COM::PER_SOCKET_CONTEXT*>	m_arrayClientContext;

	//套接字数组临界区
	CRITICAL_SECTION	m_arrayWinLock;

	//服务器ip和端口
	unsigned short	m_nPort;
	std::string		m_strIP;

	//默认端口和ip
	const unsigned short DEFAULT_PORT;
	const char *DEFAULT_IP;

	//记录工作线程个数
	long	m_nWorkNum;

	//acceptex，和 GetAcceptExSockaddrs函数指针，后面会用wsaioctl获取其地址
	LPFN_ACCEPTEX	m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS	m_lpfnGetAcceptExSocketAddrs;

	//客户端连入个数
	static long	snClient_Count;
};

