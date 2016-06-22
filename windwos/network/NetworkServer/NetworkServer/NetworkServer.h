#pragma once

#ifdef NETWORKSERVER_EXPORTS
#define NETWORKSERVER_API __declspec(dllexport)
#else
#define NETWORKSERVER_API __declspec(dllimport)
#endif


class NETWORKSERVER_API CNetworkServer
{
public:
	CNetworkServer(void);
	~CNetworkServer(void);

public:
	/*!
	*@name 
	*@brief 启动服务器监听
	*@param [in] 
				pcIp   ：服务器ip 不传入默认监听本机ip
				nPort  : 监听端口 
	*@return [out]
				设置标志，成功返回0，失败返回非0
	*/
	long	StartListen(const long nPort, const char *pcIp);

	/*!
	*@name 
	*@brief 停止服务器监听
	*@param [in] 
	*@return [out]
	*/
	void	StopListen(void);

private:
	void		*m_pNetworkServer;
};

