#pragma once
#include <vector>
#include <string>
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

namespace IOCP_COM{

	//�˳���ɶ˿�
#define EXIT_COMPLETE NULL
	enum NET_MSG{
		NET_MSG_NONE = 0,
		NET_MSG_NOERR = 1,
		NET_MSG_CONNECT,
		NET_MSG_DISCONNECT,
		NET_MSG_ERROR,
	};
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
		void	   *pThis; //�߳�������
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