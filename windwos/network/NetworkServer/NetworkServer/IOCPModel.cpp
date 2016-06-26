#include <process.h>
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

//��ʼ�� Winsock 2.2
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


void CIOCPModel::ReleaseIOCP()
{
	DeleteCriticalSection(&m_arrayWinLock);
	Release(m_hExitHandle);
	for(int i = 0; i < m_nWorkNum; i ++)
	{
		Release(m_phWorkHandle[i]);
	}
	Release(m_phWorkHandle);
	Release(m_hIoCompletionPort);
	Release(m_phListenContext);
}

void CIOCPModel::Release(void *p)
{
	if(NULL == p){
		return;
	}

	delete p;
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

	//��¼�����̸߳����������ͷ���Դ
	m_nWorkNum = nProcess;

	m_hExitHandle = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_phWorkHandle = new HANDLE[nProcess];
	for (int i = 0; i > m_nWorkNum; i ++)
	{
		IOCP_COM::ThreadParam_Work	*pParm = new IOCP_COM::ThreadParam_Work;
		pParm->pThis = reinterpret_cast<void*>(this);
		pParm->nThreadNo = i;
		m_phWorkHandle[i] = CreateThread(NULL, 0, _WorkThread, (void*)pParm, 0, NULL);
	}

	return Net_Com::NS_ERR_OK;
}

long CIOCPModel::InitListen(const char *pSvrIp, const int nPort)
{
	m_strIP = (nullptr == pSvrIp) ? GetLocalIP() : pSvrIp;
	m_nPort = (0 == nPort) ? DEFAULT_PORT : nPort;
	
	//��������socket
	m_phListenContext = new IOCP_COM::PER_SOCKET_CONTEXT;
	m_phListenContext->m_hSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	printDebug("������socket = ", m_phListenContext->m_hSocket);

	//�󶨵���ɶ˿�
	if(NULL == CreateIoCompletionPort((HANDLE)m_phListenContext->m_hSocket, m_hIoCompletionPort, (DWORD)m_phListenContext, 0))
	{
		printDebug("����ɶ˿�ʧ�ܣ� ������ = ", WSAGetLastError());
		return Net_Com::NS_ERR_BIND_IOCP;
	}

	//��������ַ��ӵ�а�hListenSock
	struct sockaddr_in	stuServerAddr;
	memset(&stuServerAddr, 0, sizeof(stuServerAddr));
	stuServerAddr.sin_family = AF_INET;
	stuServerAddr.sin_addr.S_un.S_addr = inet_addr(m_strIP.c_str());
	stuServerAddr.sin_port = htons(m_nPort);

	if(SOCKET_ERROR == bind(m_phListenContext->m_hSocket, (sockaddr*)(&stuServerAddr), sizeof(stuServerAddr)))
	{
		printDebug("���׽���ʧ�ܣ� ������ = ", WSAGetLastError());
		return Net_Com::NS_ERR_BIND;
	}

	if(SOCKET_ERROR == listen(m_phListenContext->m_hSocket, SOMAXCONN))
	{
		printDebug("����ʧ�ܣ� ������ = ", WSAGetLastError());
		return Net_Com::NS_ERR_LISTEN;
	}

	//Ͷ��acceptex����
	if(!GetlpfnAccept(m_phListenContext->m_hSocket))
	{
		//�ͷ���Դ
		ReleaseIOCP();
		return Net_Com::NS_ERR_GETFN;
	}

	for(int i = 0; i < IOCP_COM::MAX_ACCEPT_POST; i ++)
	{
		IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext = m_phListenContext->GetNewIoContext();
		if(!PostAccept(pAcceptIoContext))
		{
			m_phListenContext->RemoveIoContext(pAcceptIoContext);
			printDebug("Ͷ�� AcceptEx ʧ�ܣ������� = ", WSAGetLastError());
			return Net_Com::NS_ERR_POST_ACCEPT;
		}
	}

	printDebug("��ʼ��������ɣ�",0);
	return Net_Com::NS_ERR_OK;
}

DWORD CIOCPModel::_WorkThread(LPVOID lpParam)
{
	IOCP_COM::ThreadParam_Work *pManage = reinterpret_cast<IOCP_COM::ThreadParam_Work*>(lpParam);
	CIOCPModel *pThis = reinterpret_cast<CIOCPModel*>(pManage->pThis);

	DWORD	nReceiveNumber = 0;
	IOCP_COM::PER_SOCKET_CONTEXT *pSockContext = nullptr;
	IOCP_COM::PER_IO_CONTEXT	 *pClientContext = nullptr;
	OVERLAPPED					 *pOverlapped = nullptr;

	while(WAIT_OBJECT_0 != WaitForSingleObject(pThis->m_hExitHandle, IOCP_COM::Milliseconds_ZERO))
	{
		
		BOOL bReturn = GetQueuedCompletionStatus(pThis->m_hIoCompletionPort, 
												 &nReceiveNumber, 
												reinterpret_cast<PULONG_PTR>(&pSockContext), 
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

		pClientContext = nullptr;
		pClientContext = CONTAINING_RECORD(pOverlapped, IOCP_COM::PER_IO_CONTEXT, m_hOverlapped);

		if(0 == nReceiveNumber && (
			IOCP_COM::RECV_POSTED == pClientContext->m_tOpType || IOCP_COM::SEND_POSTED == pClientContext->m_tOpType))
		{
			pThis->printDebug("�ͷ��� : ", inet_ntoa(pSockContext->m_hClientAddr.sin_addr), true);
			pThis->printDebug("���˿� : ", ntohs(pSockContext->m_hClientAddr.sin_port), true);
			pThis->printDebug(",�Ͽ�����... : ", 0);
			pThis->RemoveContext(pSockContext);
			continue;	
		}

		switch(pClientContext->m_tOpType)
		{
		case IOCP_COM::ACCEPT_POSTED:
			break;
		case IOCP_COM::RECV_POSTED:
			break;
		default:
			break;
		}
	}

	delete pManage;
	pManage = nullptr;

	return 0;
}

long CIOCPModel::Start(const unsigned short nPort /* = 0 */, const char *pSvrIp /* = nullptr */)
{
	InitializeCriticalSection(&m_arrayWinLock);

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
			//֪ͨ������ɶ˿��˳�
			PostQueuedCompletionStatus(m_hIoCompletionPort, 0,(ULONG_PTR)EXIT_COMPLETE, NULL);
		}

		WaitForMultipleObjects(m_nWorkNum, m_phWorkHandle, TRUE, INFINITE);
	}

	ClearContextList();
	ReleaseIOCP();
}

bool CIOCPModel::PostAccept(IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext)
{
	//�׽��ֱ�������Ч��
	if(INVALID_SOCKET != pAcceptIoContext->m_hSocket ||
		NULL == m_lpfnAcceptEx)
	{
		return false;
	}

	pAcceptIoContext->m_tOpType = IOCP_COM::ACCEPT_POSTED;
	WSABUF	*p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED	*p_ol = &pAcceptIoContext->m_hOverlapped;

	//Ϊ�Ժ��������Ŀͻ���׼���׽��֣������봫ͳ��accept���Ĳ��
	pAcceptIoContext->m_hSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	printDebug("�������׽��� nIndex = ", CIOCPModel::snClient_Count, true);
	printDebug("Socket = ", pAcceptIoContext->m_hSocket);

	CIOCPModel::snClient_Count ++;

	if(INVALID_SOCKET == pAcceptIoContext->m_hSocket)
	{
		printDebug("��������AcceptEx���׽���ʧ�ܣ������� : ", WSAGetLastError());
		return false;
	}

	DWORD dwBytes = 0;
	if(FALSE == m_lpfnAcceptEx(m_phListenContext->m_hSocket, pAcceptIoContext->m_hSocket, p_wbuf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2), 
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			return false;
		}
	}

	return true;
}

void CIOCPModel::AddToContextList(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext )
{
	IOCP_COM::CSynLock	L(&m_arrayWinLock);

	m_arrayClientContext.push_back(pSocketContext);	
}

////////////////////////////////////////////////////////////////
//	�Ƴ�ĳ���ض���Context
void CIOCPModel::RemoveContext(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext )
{
	IOCP_COM::CSynLock	L(&m_arrayWinLock);

	std::vector<IOCP_COM::PER_SOCKET_CONTEXT*>::iterator it = m_arrayClientContext.begin();
	for(; it != m_arrayClientContext.end(); it ++)
	{
		if( pSocketContext == *it)
		{
			Release(*it);			
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
		Release(*it);
	}

	m_arrayClientContext.clear();
}

/*====================================================
				����������
====================================================*/
long CIOCPModel::GetProcessNum()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

std::string CIOCPModel::GetLocalIP()
{
	// ��ñ���������
	char hostname[MAX_PATH] = {0};
	gethostname(hostname,MAX_PATH);                
	struct hostent FAR* lpHostEnt = gethostbyname(hostname);
	if(lpHostEnt == NULL)
	{
		return DEFAULT_IP;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];      

	// ��IP��ַת�����ַ�����ʽ
	struct in_addr inAddr;
	memmove(&inAddr,lpAddr,4);
	m_strIP = inet_ntoa(inAddr);        

	return m_strIP;
}

bool CIOCPModel::GetlpfnAccept(SOCKET &hSock)
{
	if(INVALID_SOCKET == hSock)
	{
		printDebug("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�롣�׽�����Ч",0); 
		return false;
	}

	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 

	// ʹ��AcceptEx��������Ϊ���������WinSock2�淶֮���΢�������ṩ����չ����
	// ������Ҫ�����ȡһ�º�����ָ�룬
	// ��ȡAcceptEx����ָ��
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
		printDebug("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�롣������� = ", WSAGetLastError()); 
		
		return false;  
	}  

	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
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
		printDebug("WSAIoctl δ�ܻ�ȡGetAcceptExSocketAddrs����ָ�롣������� = ", WSAGetLastError());  
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
		OutputDebugStringA(cLog);
	}
	else
	{
		sprintf_s(cLog, sizeof(cLog), "%s%s;", pInfo, pResult);
		OutputDebugStringA(cLog);
	}
}
void CIOCPModel::printDebug(const char *pInfo, const int nResult, bool bFlag)
{
	char cLog[1024] = {'\0'};
	if(!bFlag)
	{
		sprintf_s(cLog, sizeof(cLog), "%s%d\n", pInfo, nResult);
		OutputDebugStringA(cLog);
	}
	else
	{
		sprintf_s(cLog, sizeof(cLog), "%s%d;", pInfo, nResult);
		OutputDebugStringA(cLog);
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
	//��ʱ�����ȴ�
	if(WAIT_TIMEOUT == dwErr)
	{
		//ȷ�Ͽͻ��˻�����
		if( FALSE == IsSocketAlive(pSocketContext->m_hSocket))
		{
			printDebug("��⵽�ͻ����쳣�˳���", pSocketContext->m_hSocket);
			
			return IOCP_COM::NET_MSG_DISCONNECT;
		}
		else
		{
			printDebug("��⵽���糬ʱ���������С�����", pSocketContext->m_hSocket);

			return IOCP_COM::NET_MSG_WAITTIME;
		}
	}

	//�ͻ����쳣�˳�
	else if(ERROR_NETNAME_DELETED == dwErr)
	{
		printDebug("��⵽�ͻ����쳣�˳���", pSocketContext->m_hSocket);

		return IOCP_COM::NET_MSG_DISCONNECT;
	}
	else
	{
		printDebug("��ɶ˿ڳ��ִ����߳��˳���", pSocketContext->m_hSocket);

		return IOCP_COM::NET_MSG_ERROR;
	}
	
}



