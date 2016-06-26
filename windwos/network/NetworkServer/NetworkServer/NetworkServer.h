#pragma once

#ifdef _THIS_IS_DLL
#ifdef NETWORKSERVER_EXPORTS
#define NETWORKSERVER_API __declspec(dllexport)
#else
#define NETWORKSERVER_API __declspec(dllimport)
#endif
#else
#define NETWORKSERVER_API
#endif

class NETWORKSERVER_API CNetworkServer
{
public:
	CNetworkServer(void);
	~CNetworkServer(void);

public:
	/*!
	*@name 
	*@brief ��������������
	*@param [in] 
				pcIp   ��������ip ������Ĭ�ϼ�������ip
				nPort  : �����˿� 
	*@return [out]
				���ñ�־���ɹ�����0��ʧ�ܷ��ط�0
	*/
	long	StartListen(const long nPort, const char *pcIp);

	/*!
	*@name 
	*@brief ֹͣ����������
	*@param [in] 
	*@return [out]
	*/
	void	StopListen(void);

private:
	void		*m_pNetworkServer;
};

