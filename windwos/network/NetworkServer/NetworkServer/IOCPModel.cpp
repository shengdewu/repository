#include <process.h>
#ifndef __RELEASE__
#include <iostream>
#endif
#include "IOCPModel.h"
#include "ErrorCode.h"

long CIOCPModel::snClient_Count = 0;

CIOCPModel::CIOCPModel(void):
	m_phListenContext(NULL),
	m_hIoCompletionPort(NULL),
	m_phWorkHandle(NULL),
	m_hExitHandle(NULL),
	DEFAULT_PORT(12345),
	DEFAULT_IP("127.0.0.1"),
	m_lpfnAcceptEx(NULL),
	m_lpfnGetAcceptExSocketAddrs(NULL)
{
	LoadSocketLib();
}


CIOCPModel::~CIOCPModel()
{
	UnloadSocketLib();
}

//初始化 Winsock 2.2
bool CIOCPModel::LoadSocketLib()
{
	WSADATA	wsaData;
	WORD	sockVersion = MAKEWORD(2, 2);

	int nRet = WSAStartup(sockVersion, &wsaData);

	if(NO_ERROR != nRet)
	{
		return false;
	}

	return true;
}

void CIOCPModel::UnloadSocketLib()
{
	WSACleanup();
}


void CIOCPModel::UninitCompeletionPort()
{
	DeleteCriticalSection(&m_arrayWinLock);
	CloseHandle(m_hExitHandle);
	for(int i = 0; i < m_nWorkNum; i ++)
	{
		CloseHandle(m_phWorkHandle[i]);
	}
	delete m_phWorkHandle;
	m_phWorkHandle = nullptr;

	CloseHandle(m_hIoCompletionPort);

	delete m_phListenContext;
	m_phListenContext = nullptr;
	
}


long CIOCPModel::InitCompeletionPort(int nThread /* = 0 */)
{
	
	m_hIoCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(NULL == m_hIoCompletionPort)
	{
		return Net_Com::NS_ERR_CREATE_IOCP;
	}

	int nProcess = EN_MAX_MUTIL * GetProcessNum();

	if(nThread > 0)
	{
		nProcess = nThread > nProcess ? nProcess : nThread;
	}

	//记录工作线程个数，方便释放资源
	m_nWorkNum = nProcess;

	printDebug("建立了工作线程:", m_nWorkNum);

	m_hExitHandle = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_phWorkHandle = new HANDLE[nProcess];
	for (int i = 0; i < m_nWorkNum; i ++)
	{
		IOCP_COM::ThreadParam_Work	*pParm = new IOCP_COM::ThreadParam_Work;
		pParm->pThis = reinterpret_cast<void*>(this);
		pParm->nThreadNo = i + 1;
		m_phWorkHandle[i] = ::CreateThread(NULL, 0, _WorkThread, (void*)pParm, 0, NULL);
	}

	return Net_Com::NS_ERR_OK;
}

long CIOCPModel::InitListen(const char *pSvrIp, const int nPort)
{
	InitializeCriticalSection(&m_arrayWinLock);

	m_strIP = (nullptr == pSvrIp) ? GetLocalIP() : pSvrIp;
	m_nPort = (0 == nPort) ? DEFAULT_PORT : nPort;
	
	//创建监听socket
	m_phListenContext = new IOCP_COM::PER_SOCKET_CONTEXT;
	m_phListenContext->m_hSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	if (INVALID_SOCKET == m_phListenContext->m_hSocket) 
	{
		printDebug("初始化Socket失败，错误代码:", WSAGetLastError());
		return Net_Com::NS_ERR_ACCEPT;
	}

	printDebug("服务器socket = ", m_phListenContext->m_hSocket);

	//绑定到完成端口
	if(NULL == CreateIoCompletionPort((HANDLE)m_phListenContext->m_hSocket, m_hIoCompletionPort, (DWORD)m_phListenContext, 0))
	{
		printDebug("绑定完成端口失败， 错误码 = ", WSAGetLastError());
		return Net_Com::NS_ERR_BIND_IOCP;
	}

	//服务器地址，拥有绑定hListenSock
	struct sockaddr_in	stuServerAddr;
	memset(&stuServerAddr, 0, sizeof(stuServerAddr));
	stuServerAddr.sin_family = AF_INET;
	stuServerAddr.sin_addr.S_un.S_addr = inet_addr(m_strIP.c_str()); 
	stuServerAddr.sin_port = htons(m_nPort);

	if(SOCKET_ERROR == bind(m_phListenContext->m_hSocket, (struct sockaddr*)(&stuServerAddr), sizeof(stuServerAddr))) // err - 1
	{
		printDebug("绑定套接字失败， 错误码 = ", WSAGetLastError());
		return Net_Com::NS_ERR_BIND;
	}

	if(SOCKET_ERROR == listen(m_phListenContext->m_hSocket, SOMAXCONN))
	{
		printDebug("监听失败， 错误码 = ", WSAGetLastError());
		return Net_Com::NS_ERR_LISTEN;
	}

	//获取函数指针
	if(!GetlpfnAccept(m_phListenContext->m_hSocket))
	{
		//释放资源
		UninitCompeletionPort();
		return Net_Com::NS_ERR_GETFN;
	}

	// 为AcceptEx 准备参数，然后投递AcceptEx I/O请求
	for(int i = 0; i < IOCP_COM::MAX_ACCEPT_POST; i ++)
	{
		IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext = m_phListenContext->GetNewIoContext();
		if(!PostAccept(pAcceptIoContext))
		{
			m_phListenContext->RemoveIoContext(pAcceptIoContext);
			printDebug("投递 AcceptEx 失败，错误码 = ", WSAGetLastError());
			return Net_Com::NS_ERR_POST_ACCEPT;
		}
	}

	printDebug("初始化监听完成：",0);
	return Net_Com::NS_ERR_OK;
}

DWORD CIOCPModel::_WorkThread(LPVOID lpParam)
{
	IOCP_COM::ThreadParam_Work *pManage = reinterpret_cast<IOCP_COM::ThreadParam_Work*>(lpParam);
	CIOCPModel *pThis = reinterpret_cast<CIOCPModel*>(pManage->pThis);

	int nThreadNo = (int)pManage->nThreadNo;
	pThis->printDebug("工作者线程启动，ID:",nThreadNo);

	OVERLAPPED					 *pOverlapped = nullptr;
	IOCP_COM::PER_SOCKET_CONTEXT *pSockContext = nullptr;
	DWORD	dwBytesTransfered = 0;

	while(WAIT_OBJECT_0 != WaitForSingleObject(pThis->m_hExitHandle, IOCP_COM::Milliseconds_ZERO))
	{
		BOOL bReturn = GetQueuedCompletionStatus(pThis->m_hIoCompletionPort, 
												 &dwBytesTransfered, 
												(PULONG_PTR)(&pSockContext), 
												&pOverlapped, 
												INFINITE);

		if(EXIT_COMPLETE == (DWORD)pSockContext)
		{
			break;
		}

		if(FALSE == bReturn)
		{
			DWORD nErr = GetLastError();
			UINT nRet = pThis->ErrorHandle(pSockContext,nErr);
			if(IOCP_COM::NET_MSG_ERROR == nRet)
			{
				break;
			}
			continue;
		}

		//读取传入的参数
		IOCP_COM::PER_IO_CONTEXT *pIoContext = CONTAINING_RECORD(pOverlapped, IOCP_COM::PER_IO_CONTEXT, m_hOverlapped);

		if(0 == dwBytesTransfered && (IOCP_COM::RECV_POSTED == pIoContext->m_tOpType || IOCP_COM::SEND_POSTED == pIoContext->m_tOpType))
		{
			char cLog[1024] = {'\0'};

			sprintf_s(cLog, sizeof(cLog), "Work:%s,%d:断开连接\n", inet_ntoa(pSockContext->m_hClientAddr.sin_addr), ntohs(pSockContext->m_hClientAddr.sin_port),pIoContext->m_wsaBuf.buf);
			//OutputDebugStringA(cLog);
			std::cout << cLog << std::endl;
			pThis->RemoveContext(pSockContext);
			continue;	
		}

		switch(pIoContext->m_tOpType)
		{
		case IOCP_COM::ACCEPT_POSTED:
			pThis->DoAccept(pSockContext, pIoContext);
			break;
		case IOCP_COM::RECV_POSTED:
			pThis->DoRecv(pSockContext, pIoContext);
			break;
		case IOCP_COM::SEND_POSTED:
			break;
		default:
			pThis->printDebug("_WorkThread中的 pIoContext->m_OpType 参数异常.", 0);
			break;
		}
	}

	pThis->printDebug("退出工作线程 id= ", pManage->nThreadNo);
	delete pManage;
	pManage = nullptr;

	return 0;
}

bool CIOCPModel::DoAccept(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext, IOCP_COM::PER_IO_CONTEXT *pIoContext)
{
	SOCKADDR_IN* pClientAddr = NULL;
	SOCKADDR_IN* pLocalAddr = NULL;  
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

	///////////////////////////////////////////////////////////////////////////
	// 1. 首先取得连入客户端的地址信息
	// 这个 m_lpfnGetAcceptExSockAddrs 不得了啊~~~~~~
	// 不但可以取得客户端和本地端的地址信息，还能顺便取出客户端发来的第一组数据，老强大了...
	m_lpfnGetAcceptExSocketAddrs(pIoContext->m_wsaBuf.buf, 
			                    pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2),  
		                        sizeof(SOCKADDR_IN)+16, 
								sizeof(SOCKADDR_IN)+16, 
								(LPSOCKADDR*)&pLocalAddr, 
								&localLen, 
								(LPSOCKADDR*)&pClientAddr, 
								&remoteLen);

	char cLog[1024] = {'\0'};

	sprintf_s(cLog, sizeof(cLog), "DoAccept:%s,%d:%s\n", inet_ntoa(pClientAddr->sin_addr), ntohs(pClientAddr->sin_port),pIoContext->m_wsaBuf.buf);
	//OutputDebugStringA(cLog);
	std::cout << cLog << std::endl;

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. 这里需要注意，这里传入的这个是ListenSocket上的Context，这个Context我们还需要用于监听下一个连接
	// 所以我还得要将ListenSocket上的Context复制出来一份为新连入的Socket新建一个SocketContext
	IOCP_COM::PER_SOCKET_CONTEXT *pNewSocketContext = new IOCP_COM::PER_SOCKET_CONTEXT;
	pNewSocketContext->m_hSocket = pIoContext->m_hSockAccept;
	memcpy_s(&(pNewSocketContext->m_hClientAddr), sizeof(pNewSocketContext->m_hClientAddr), pClientAddr, sizeof(SOCKADDR_IN));
	// 参数设置完毕，将这个Socket和完成端口绑定(这也是一个关键步骤)
	if(!AssociateWithIOCP(pNewSocketContext))
	{
		delete pNewSocketContext;
		pNewSocketContext = nullptr;
		return false;
	}

	// 3. 继续，建立其下的IoContext，用于在这个Socket上投递第一个Recv数据请求
	IOCP_COM::PER_IO_CONTEXT *pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_tOpType = IOCP_COM::RECV_POSTED;
	pNewIoContext->m_hSockAccept = pNewSocketContext->m_hSocket;
	// 如果Buffer需要保留，就自己拷贝一份出来
	//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN);
	if(!PostRecv(pNewIoContext))
	{
		pNewSocketContext->RemoveIoContext(pNewIoContext);
		return false;
	}
	
	// 4. 如果投递成功，那么就把这个有效的客户端信息，加入到ContextList中去(需要统一管理，方便释放资源)
	AddToContextList(pNewSocketContext);

	// 5. 使用完毕之后，把Listen Socket的那个IoContext重置，然后准备投递新的AcceptEx
	pIoContext->ResetBuffer();

	return PostAccept(pIoContext);
}

bool CIOCPModel::DoRecv(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext, IOCP_COM::PER_IO_CONTEXT *pIoContext)
{
	SOCKADDR_IN *ClientAddr = &pSocketContext->m_hClientAddr;

	char cLog[1024] = {'\0'};

	sprintf_s(cLog, sizeof(cLog), "DoRecv:%s,%d:%s\n", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf);
	//OutputDebugStringA(cLog);
	std::cout << cLog << std::endl;

	// 然后开始投递下一个WSARecv请求
	return PostRecv(pIoContext);  //important
}


bool CIOCPModel::AssociateWithIOCP(IOCP_COM::PER_SOCKET_CONTEXT *pSockContext)
{
	// 将用于和客户端通信的SOCKET绑定到完成端口中
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pSockContext->m_hSocket, m_hIoCompletionPort, (DWORD)pSockContext, 0);

	if(NULL == hTemp)
	{
		printDebug("支持绑定完成端口失败：", WSAGetLastError());
		return false;
	}

	return true;
}

long CIOCPModel::Start(const unsigned short nPort /* = 0 */, const char *pSvrIp /* = nullptr */)
{
	long lRet = Net_Com::NS_ERR_OK;
	lRet = InitCompeletionPort();
	if(Net_Com::NS_ERR_OK != lRet)
	{
		return lRet;
	}

	lRet = InitListen(pSvrIp, nPort);
	if(Net_Com::NS_ERR_OK != lRet)
	{
		return lRet;
	}

	return lRet;
}

void CIOCPModel::Stop()
{
	if(m_hExitHandle)
	{
		SetEvent(m_hExitHandle);

		for(int i = 0; i < m_nWorkNum; i ++)
		{
			//通知所有完成端口退出
			PostQueuedCompletionStatus(m_hIoCompletionPort, 0,(ULONG_PTR)EXIT_COMPLETE, NULL);
		}

		WaitForMultipleObjects(m_nWorkNum, m_phWorkHandle, TRUE, INFINITE);
	}

	ClearContextList();
	UninitCompeletionPort();
}

bool CIOCPModel::PostAccept(IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext)
{
	//套接字必须是无效的
	if(INVALID_SOCKET == m_phListenContext->m_hSocket ||
		NULL == m_lpfnAcceptEx)
	{
		return false;
	}

	pAcceptIoContext->m_tOpType = IOCP_COM::ACCEPT_POSTED;
	WSABUF	*p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED	*p_ol = &pAcceptIoContext->m_hOverlapped;

	//为以后的新连入的客户端准备套接字，这是与传统的accept最大的差别
	pAcceptIoContext->m_hSockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	char cLog[1024] = {'\0'};
	sprintf_s(cLog, sizeof(cLog), "新连入套接字:ndex=%d,socket=%d\n", CIOCPModel::snClient_Count, pAcceptIoContext->m_hSockAccept);
	std::cout << cLog << std::endl;
	CIOCPModel::snClient_Count ++;

	if(INVALID_SOCKET == pAcceptIoContext->m_hSockAccept)
	{
		printDebug("创建用于AcceptEx的套接字失败，错误码 : ", WSAGetLastError());
		return false;
	}

	DWORD dwBytes = 0;
	if(FALSE == m_lpfnAcceptEx(m_phListenContext->m_hSocket, pAcceptIoContext->m_hSockAccept, 
					         p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2), 
					         sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			printDebug("投递 AcceptEx 请求失败，错误代码:", WSAGetLastError());
			return false;
		}
	}

	return true;
}

bool CIOCPModel::PostRecv(IOCP_COM::PER_IO_CONTEXT *pIoContext)
{
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_hOverlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_tOpType = IOCP_COM::RECV_POSTED;

	//初始化完成后，投递wsarecv请求
	int nBytesRecv = WSARecv(pIoContext->m_hSockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

	//如果返回值错误，并且错误代码并非是Pending的话，那就说明这个重叠请求失败了
	if((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		printDebug("投递第一个wsarecv失败", 0);
		return false;
	}
	return true;
}

void CIOCPModel::AddToContextList(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext )
{
	IOCP_COM::CSynLock	L(&m_arrayWinLock);

	m_arrayClientContext.push_back(pSocketContext);	
}

////////////////////////////////////////////////////////////////
//	移除某个特定的Context
void CIOCPModel::RemoveContext(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext )
{
	IOCP_COM::CSynLock	L(&m_arrayWinLock);

	std::vector<IOCP_COM::PER_SOCKET_CONTEXT*>::iterator it = m_arrayClientContext.begin();
	for(; it != m_arrayClientContext.end(); it ++)
	{
		if( pSocketContext == *it)
		{
			delete *it;
			*it= nullptr;
			it = m_arrayClientContext.erase(it);			
			break;
		}
	}
}

void CIOCPModel::ClearContextList()
{
	IOCP_COM::CSynLock	L(&m_arrayWinLock);
	std::vector<IOCP_COM::PER_SOCKET_CONTEXT*>::iterator it = m_arrayClientContext.begin();
	for(; it != m_arrayClientContext.end(); it ++)
	{
		delete *it;
		*it= nullptr;
	}

	m_arrayClientContext.clear();
}

/*====================================================
				辅助函数：
====================================================*/
long CIOCPModel::GetProcessNum()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

std::string CIOCPModel::GetLocalIP()
{
	// 获得本机主机名
	char hostname[MAX_PATH] = {0};
	gethostname(hostname,MAX_PATH);                
	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
	if(lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];      

	// 将IP地址转化成字符串形式
	struct in_addr inAddr;
	memmove(&inAddr,lpAddr,4);
	m_strIP = inet_ntoa(inAddr);        

	return m_strIP;
}

bool CIOCPModel::GetlpfnAccept(SOCKET &hSock)
{
	if(INVALID_SOCKET == hSock)
	{
		printDebug("WSAIoctl 未能获取AcceptEx函数指针。套接字无效",0); 
		return false;
	}

	// AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 

	// 使用AcceptEx函数，因为这个是属于WinSock2规范之外的微软另外提供的扩展函数
	// 所以需要额外获取一下函数的指针，
	// 获取AcceptEx函数指针
	DWORD dwBytes = 0;  
	if(SOCKET_ERROR == WSAIoctl(
		hSock, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx), 
		&m_lpfnAcceptEx, 
		sizeof(m_lpfnAcceptEx), 
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		printDebug("WSAIoctl 未能获取AcceptEx函数指针。错误代码 = ", WSAGetLastError()); 
		
		return false;  
	}  

	// 获取GetAcceptExSockAddrs函数指针，也是同理
	if(SOCKET_ERROR == WSAIoctl(
		hSock, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), 
		&m_lpfnGetAcceptExSocketAddrs, 
		sizeof(m_lpfnGetAcceptExSocketAddrs),   
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		printDebug("WSAIoctl 未能获取GetAcceptExSocketAddrs函数指针。错误代码 = ", WSAGetLastError());  
		return false; 
	}  

	return true;
}

void CIOCPModel::printDebug(const char *pInfo, const char *pResult, bool bFlag)
{
	char cLog[1024] = {'\0'};
	if(!bFlag)
	{
		sprintf_s(cLog, sizeof(cLog), "%s%s\n", pInfo, pResult);
		//OutputDebugStringA(cLog);
		std::cout << cLog << std::endl;
	}
	else
	{
		sprintf_s(cLog, sizeof(cLog), "%s%s", pInfo, pResult);
		//OutputDebugStringA(cLog);
		std::cout << cLog;
	}
}
void CIOCPModel::printDebug(const char *pInfo, const int nResult, bool bFlag)
{
	char cLog[1024] = {'\0'};
	if(!bFlag)
	{
		sprintf_s(cLog, sizeof(cLog), "%s%d\n", pInfo, nResult);
		//OutputDebugStringA(cLog);
		std::cout << cLog << std::endl;
	}
	else
	{
		sprintf_s(cLog, sizeof(cLog), "%s%d;", pInfo, nResult);
		//OutputDebugStringA(cLog);
		std::cout << cLog;
	}
}

BOOL CIOCPModel::IsSocketAlive(const SOCKET &hSocket)
{
	int nBytes = send(hSocket, "", 0, 0);
	if(-1 == nBytes)
	{
		return FALSE;
	}

	return TRUE;
}

UINT CIOCPModel::ErrorHandle(const IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext, const DWORD &dwErr)
{
	//超时继续等待
	if(WAIT_TIMEOUT == dwErr)
	{
		//确认客户端还活着
		if( FALSE == IsSocketAlive(pSocketContext->m_hSocket))
		{
			printDebug("检测到客户端异常退出！", pSocketContext->m_hSocket);
			
			return IOCP_COM::NET_MSG_DISCONNECT;
		}
		else
		{
			printDebug("检测到网络超时！，重试中。。。", pSocketContext->m_hSocket);

			return IOCP_COM::NET_MSG_WAITTIME;
		}
	}

	//客户端异常退出
	else if(ERROR_NETNAME_DELETED == dwErr)
	{
		printDebug("检测到客户端异常退出！", pSocketContext->m_hSocket);

		return IOCP_COM::NET_MSG_DISCONNECT;
	}
	else
	{
		printDebug("完成端口出现错误，线程退出！", pSocketContext->m_hSocket);

		return IOCP_COM::NET_MSG_ERROR;
	}
	
}



