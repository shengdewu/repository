#pragma once
#include "PublicHeader.h"

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
	bool PostRecv();

	//����ͻ����б�
	void ClearContextList(void);

	bool DoAccept();

	//
	bool DoRecv();

	//���������
	UINT ErrorHandle(const IOCP_COM::PER_SOCKET_CONTEXT *pSocketContext, const DWORD &dwErr);

	// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
	// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
	// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��
	BOOL IsSocketAlive(const SOCKET &hSocket);

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

