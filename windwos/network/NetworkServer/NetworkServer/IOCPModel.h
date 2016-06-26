#pragma once
#include "PublicHeader.h"

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
	void printDebug(const char *pInfo, const int nResult, bool bFlag = false);
	void printDebug(const char *pInfo, const char *pResult, bool bFlag = false);
	//获取AcceptEx和GetAcceptExSockaddrs函数指针
	bool GetlpfnAccept(SOCKET &hSock);

	//初始化失败后，释放资源
	void ReleaseIOCP();

	//删除指针
	void Release(void *p);

	//投递AcceptEx请求
	bool PostAccept(IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext);

	//投递Recv请求
	bool PostRecv();

	//清除客户端列表
	void ClearContextList(void);

	//增加客户端信息
	void AddToContextList(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext);

	//删除客户端信息
	void RemoveContext(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext);

	bool DoAccept();

	//
	bool DoRecv();

	//网络错误处理
	UINT ErrorHandle(const IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext, const DWORD &dwErr);

	// 判断客户端Socket是否已经断开，否则在一个无效的Socket上投递WSARecv操作会出现异常
	// 使用的方法是尝试向这个socket发送数据，判断这个socket调用的返回值
	// 因为如果客户端网络异常断开(例如客户端崩溃或者拔掉网线等)的时候，服务器端是无法收到客户端断开的通知的
	BOOL IsSocketAlive(const SOCKET &hSocket);

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

