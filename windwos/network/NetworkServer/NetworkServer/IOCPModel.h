#pragma once
#include <vector>
#include <string>
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

class CIOCPModel;

namespace IOCP_COM{
	//��ɶ˿ڳ���
	enum COM_CONST{
		Milliseconds_ZERO = 0,
		MAX_ACCEPT_POST = 10,
		MAX_BUFFER_LEN = 8192, //1024 * 8 //8k

	};

	//��ɶ˿���Ͷ�ݵ�io��������
	typedef enum _temOPERATION_TYPE{
		NULL_POSTED = 0,
		ACCEPT_POSTED,
		SEND_POSTED,
		RECV_POSTED,
	}OPERATION_TYPE;

	//�ٽ���
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

	//�����̲߳���
	typedef struct _tagThreadParam_Work
	{
		CIOCPModel	*pIOCPModel;
		int			nThreadNo;
		void		*pRes;
	}ThreadParam_Work;

	//��IO���ݽṹ�嶨��(����ÿһ���ص������Ĳ���)
	typedef struct _tagPER_IO_CONTEXT{
		OVERLAPPED	m_hOverlapped;  //ÿ���ص�����������ص��ṹ�����ÿ��socket��������Ҫ��һ��
		WSABUF		m_wsaBuf;       //WSA���͵Ļ����������ڸ��ص��ṹ������
		char		m_szBuffer[MAX_BUFFER_LEN]; //WSABUF ����Ļ�����
		SOCKET		m_hSocket;      //����������õ�
		OPERATION_TYPE	m_tOpType;  //���ص��ṹ��������

		//��ʼ��
		_tagPER_IO_CONTEXT()
		{
			ZeroMemory(&m_hOverlapped, sizeof(m_hOverlapped));
			ZeroMemory(m_szBuffer, MAX_BUFFER_LEN);
			m_hSocket = INVALID_SOCKET;
			m_wsaBuf.buf = m_szBuffer;
			m_wsaBuf.len = MAX_BUFFER_LEN;
			m_tOpType =  NULL_POSTED;
		}

		//�ͷ�
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

	//���������ÿ����ɶ˿ڵĲ�����Ҳ��ÿ��sock�Ĳ���
	typedef struct _tagPER_SOCKET_CONTEXT
	{
		SOCKET					m_hSocket;      //ÿ���ͻ����׽���
		SOCKADDR_IN				m_hClientAddr;  //�ͻ��˵�ַ
		std::vector<PER_IO_CONTEXT*>	m_arrayIoContext; //ÿ���ͻ��˵�Ӧ���io����

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

		//����һ���µ�io
		PER_IO_CONTEXT *GetNewIoContext(void)
		{
			PER_IO_CONTEXT *p = new PER_IO_CONTEXT;

			m_arrayIoContext.push_back(p);

			return p;
		}

		//�Ƴ�ָ����io
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

	//������ɶ˿ڷ���
	//pSvrIp : ����ip Ĭ�ϱ���ip
	//nPort  �� �����˿� mĬ��12345
	//�ɹ�����0��ʧ�ܷ��ش�����
	long Start(const unsigned short nPort = 0, const char *pSvrIp = nullptr);

	//ֹͣ����
	void Stop();
private:
	
	//��ʼ�������,�ڿ�ʼ�����ʼ��֮ǰ������ã����캯�����Ѿ�����
	//���ñ�־���ɹ�����true������false
	bool LoadSocketLib();

	//�ͷ�����⣬�ڹر��������Ӻ��ڵ��ã������������Ѿ�����
	void UnloadSocketLib();

	//��ʼ����ɶ˿ڼ��߳�
	//nThread : �����̸߳�����nThread ����0�ǣ�Ĭ��Ϊcpu�ں˸��� �� 2
	//�ɹ�����0��ʧ�ܷ��ش�����
	long InitCompeletionPort(int nThread = 0);

	//��ʼ������
	//pSvrIp : ����ip
	//nPort  �� �����˿�
	//�ɹ�����0��ʧ�ܷ��ش�����
	long InitListen(const char *pSvrIp, const int nPort);

	//��ȡ��������������
	long GetProcessNum();

	//��ȡ����ip
	std::string GetLocalIP(void);

	//�̴߳�����
	static DWORD __stdcall _WorkThread(LPVOID lpParam);

	//��־
	void printDegug(const char *pInfo, const int nResult, bool bFlag = false);

	//��ȡAcceptEx��GetAcceptExSockaddrs����ָ��
	bool GetlpfnAccept(SOCKET &hSock);

	//��ʼ��ʧ�ܺ��ͷ���Դ
	void ReleaseIOCP();

	//ɾ��ָ��
	void Release(void *p);

	//Ͷ��AcceptEx����
	bool PostAccept(IOCP_COM::PER_IO_CONTEXT *pAcceptIoContext);

	//Ͷ��Recv����

	//����ͻ����б�
	void ClearContextList(void);

private:
	//����
	enum{
		EN_MAX_MUTIL = 2,
	};
	//��ɶ˿�
	HANDLE	m_hIoCompletionPort;
	//�����ֲ߳̾�
	HANDLE	*m_phWorkHandle;
	//�����߳��˳��¼�
	HANDLE	m_hExitHandle;

	//�����������׽���
	IOCP_COM::PER_SOCKET_CONTEXT			*m_phListenContext;
	std::vector<IOCP_COM::PER_SOCKET_CONTEXT*>	m_arrayClientContext;

	//�׽��������ٽ���
	CRITICAL_SECTION	m_arrayWinLock;

	//������ip�Ͷ˿�
	unsigned short	m_nPort;
	std::string		m_strIP;

	//Ĭ�϶˿ں�ip
	const unsigned short DEFAULT_PORT;
	const char *DEFAULT_IP;

	//��¼�����̸߳���
	long	m_nWorkNum;

	//acceptex���� GetAcceptExSockaddrs����ָ�룬�������wsaioctl��ȡ���ַ
	LPFN_ACCEPTEX	m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS	m_lpfnGetAcceptExSocketAddrs;

	//�ͻ����������
	static long	snClient_Count;
};

