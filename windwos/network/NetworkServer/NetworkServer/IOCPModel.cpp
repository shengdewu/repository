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

	//��¼�����̸߳����������ͷ���Դ
	m_nWorkNum = nProcess;

	printDebug("�����˹����߳�:", m_nWorkNum);

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
	
	//��������socket
	m_phListenContext = new IOCP_COM::PER_SOCKET_CONTEXT;
	m_phListenContext->m_hSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	if (INVALID_SOCKET == m_phListenContext->m_hSocket) 
	{
		printDebug("��ʼ��Socketʧ�ܣ��������:", WSAGetLastError());
		return Net_Com::NS_ERR_ACCEPT;
	}

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

	if(SOCKET_ERROR == bind(m_phListenContext->m_hSocket, (struct sockaddr*)(&stuServerAddr), sizeof(stuServerAddr))) // err - 1
	{
		printDebug("���׽���ʧ�ܣ� ������ = ", WSAGetLastError());
		return Net_Com::NS_ERR_BIND;
	}

	if(SOCKET_ERROR == listen(m_phListenContext->m_hSocket, SOMAXCONN))
	{
		printDebug("����ʧ�ܣ� ������ = ", WSAGetLastError());
		return Net_Com::NS_ERR_LISTEN;
	}

	//��ȡ����ָ��
	if(!GetlpfnAccept(m_phListenContext->m_hSocket))
	{
		//�ͷ���Դ
		UninitCompeletionPort();
		return Net_Com::NS_ERR_GETFN;
	}

	// ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O����
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

	int nThreadNo = (int)pManage->nThreadNo;
	pThis->printDebug("�������߳�������ID:",nThreadNo);

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

		//��ȡ����Ĳ���
		IOCP_COM::PER_IO_CONTEXT *pIoContext = CONTAINING_RECORD(pOverlapped, IOCP_COM::PER_IO_CONTEXT, m_hOverlapped);

		if(0 == dwBytesTransfered && (IOCP_COM::RECV_POSTED == pIoContext->m_tOpType || IOCP_COM::SEND_POSTED == pIoContext->m_tOpType))
		{
			char cLog[1024] = {'\0'};

			sprintf_s(cLog, sizeof(cLog), "Work:%s,%d:�Ͽ�����\n", inet_ntoa(pSockContext->m_hClientAddr.sin_addr), ntohs(pSockContext->m_hClientAddr.sin_port),pIoContext->m_wsaBuf.buf);
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
			pThis->DoSend(pSockContext, pIoContext, dwBytesTransfered);
			break;
		default:
			pThis->printDebug("_WorkThread�е� pIoContext->m_OpType �����쳣.", 0);
			break;
		}
	}

	pThis->printDebug("�˳������߳� id= ", pManage->nThreadNo);
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
	// 1. ����ȡ������ͻ��˵ĵ�ַ��Ϣ
	// ��� m_lpfnGetAcceptExSockAddrs �����˰�~~~~~~
	// ��������ȡ�ÿͻ��˺ͱ��ض˵ĵ�ַ��Ϣ������˳��ȡ���ͻ��˷����ĵ�һ�����ݣ���ǿ����...
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
	// 2. ������Ҫע�⣬���ﴫ��������ListenSocket�ϵ�Context�����Context���ǻ���Ҫ���ڼ�����һ������
	// �����һ���Ҫ��ListenSocket�ϵ�Context���Ƴ���һ��Ϊ�������Socket�½�һ��SocketContext
	IOCP_COM::PER_SOCKET_CONTEXT *pNewSocketContext = new IOCP_COM::PER_SOCKET_CONTEXT;
	pNewSocketContext->m_hSocket = pIoContext->m_hSockAccept;
	memcpy_s(&(pNewSocketContext->m_hClientAddr), sizeof(pNewSocketContext->m_hClientAddr), pClientAddr, sizeof(SOCKADDR_IN));
	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
	if(!AssociateWithIOCP(pNewSocketContext))
	{
		delete pNewSocketContext;
		pNewSocketContext = nullptr;
		return false;
	}

	// 3. �������������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
	IOCP_COM::PER_IO_CONTEXT *pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_tOpType = IOCP_COM::RECV_POSTED;
	pNewIoContext->m_hSockAccept = pNewSocketContext->m_hSocket;
	// ���Buffer��Ҫ���������Լ�����һ�ݳ���
	//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,MAX_BUFFER_LEN);
	if(!PostRecv(pNewIoContext))
	{
		pNewSocketContext->RemoveIoContext(pNewIoContext);
		return false;
	}
	
	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ(��Ҫͳһ���������ͷ���Դ)
	AddToContextList(pNewSocketContext);

	// 5. ʹ�����֮�󣬰�Listen Socket���Ǹ�IoContext���ã�Ȼ��׼��Ͷ���µ�AcceptEx
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

	// Ȼ��ʼͶ����һ��WSARecv����
	return PostRecv(pIoContext);  //important
}

bool CIOCPModel::DoSend(IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext, IOCP_COM::PER_IO_CONTEXT *pIoContext, DWORD dwTransferByte)
{
	SOCKADDR_IN *ClientAddr = &pSocketContext->m_hClientAddr;

	char cLog[1024] = {'\0'};

	sprintf_s(cLog, sizeof(cLog), "DoSend:%s,%d:%s\n", inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf);
	//OutputDebugStringA(cLog);
	std::cout << cLog << std::endl;	

	return true;
}

bool CIOCPModel::AssociateWithIOCP(IOCP_COM::PER_SOCKET_CONTEXT *pSockContext)
{
	// �����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pSockContext->m_hSocket, m_hIoCompletionPort, (DWORD)pSockContext, 0);

	if(NULL == hTemp)
	{
		printDebug("֧�ְ���ɶ˿�ʧ�ܣ�", WSAGetLastError());
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
			//֪ͨ������ɶ˿��˳�
			PostQueuedCompletionStatus(m_hIoCompletionPort, 0,(ULONG_PTR)EXIT_COMPLETE, NULL);
		}

		WaitForMultipleObjects(m_nWorkNum, m_phWorkHandle, TRUE, INFINITE);
	}

	ClearContextList();
	UninitCompeletionPort();
}

bool CIOCPModel::PostAccept(IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext)
{
	if(INVALID_SOCKET == m_phListenContext->m_hSocket ||
		NULL == m_lpfnAcceptEx)
	{
		return false;
	}

	pAcceptIoContext->m_tOpType = IOCP_COM::ACCEPT_POSTED;
	WSABUF	*p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED	*p_ol = &pAcceptIoContext->m_hOverlapped;

	//Ϊ�Ժ��������Ŀͻ���׼���׽��֣������봫ͳ��accept���Ĳ��
	pAcceptIoContext->m_hSockAccept = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	char cLog[1024] = {'\0'};
	sprintf_s(cLog, sizeof(cLog), "�������׽���:ndex=%d,socket=%d\n", CIOCPModel::snClient_Count, pAcceptIoContext->m_hSockAccept);
	std::cout << cLog << std::endl;
	CIOCPModel::snClient_Count ++;

	if(INVALID_SOCKET == pAcceptIoContext->m_hSockAccept)
	{
		printDebug("��������AcceptEx���׽���ʧ�ܣ������� : ", WSAGetLastError());
		return false;
	}

	DWORD dwBytes = 0;
	if(FALSE == m_lpfnAcceptEx(m_phListenContext->m_hSocket, pAcceptIoContext->m_hSockAccept, 
					         p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2), 
					         sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			printDebug("Ͷ�� AcceptEx ����ʧ�ܣ��������:", WSAGetLastError());
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

	//��ʼ����ɺ�Ͷ��wsarecv����
	int nBytesRecv = WSARecv(pIoContext->m_hSockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

	//�������ֵ���󣬲��Ҵ�����벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		printDebug("Ͷ�ݵ�һ��wsarecvʧ��", 0);
		return false;
	}
	return true;
}

bool CIOCPModel::PostSend(IOCP_COM::PER_IO_CONTEXT *pIoContext)
{
	WSABUF *p_wbuf = &(pIoContext->m_wsaBuf);
	DWORD dwSendBytes = 0, dwFlag = 0;
	OVERLAPPED	*p_ol = &(pIoContext->m_hOverlapped);
	pIoContext->m_tOpType = IOCP_COM::SEND_POSTED;

	int nRecBytes = WSASend(pIoContext->m_hSockAccept,
							p_wbuf,
							1,
							&dwSendBytes,
							dwFlag,
							p_ol,
							NULL);

	std::cout << "postsend byte = " <<dwSendBytes << std::endl;

	if(SOCKET_ERROR == nRecBytes)
	{
		printDebug("Post Send:", WSAGetLastError());
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
//	�Ƴ�ĳ���ض���Context
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

long CIOCPModel::SendData(SOCKET s, const char * pData, const long nSize)
{
	IOCP_COM::PER_IO_CONTEXT *pIoContext = new IOCP_COM::PER_IO_CONTEXT;
	{
		IOCP_COM::CSynLock L(&m_arrayWinLock);
		std::vector<IOCP_COM::PER_SOCKET_CONTEXT*>::iterator it = m_arrayClientContext.begin();
		if(m_arrayClientContext.end() == it)
		{
			return -1;
		}
		


		std::vector<IOCP_COM::PER_IO_CONTEXT*>::iterator io_iter = (*it)->m_arrayIoContext.begin();
		if((*it)->m_arrayIoContext.end() == io_iter)
		{
			return -1;
		}
	
		pIoContext->m_hSockAccept = (*io_iter)->m_hSockAccept;
	}
	

	pIoContext->ResetBuffer();
	pIoContext->m_tOpType = IOCP_COM::SEND_POSTED;

	memcpy_s(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len, pData, nSize);
	pIoContext->m_wsaBuf.len = nSize;

	PostSend(pIoContext);

	//����ɾ����������ɶ˿��̱߳���
	//���ӷ����б��������ڴ浥Ԫ����Դ
	//
	//delete pIoContext;

	return 0;
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



