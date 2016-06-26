#include "NetworkServer.h"
#include "IOCPModel.h"

CNetworkServer::CNetworkServer(void):
	m_pNetworkServer(nullptr)
{
	m_pNetworkServer = new CIOCPModel;
}


CNetworkServer::~CNetworkServer(void)
{

	delete reinterpret_cast<CIOCPModel*>(m_pNetworkServer);
}


long CNetworkServer::StartListen(const long nPort, const char *pcIp)
{
	if(nullptr == m_pNetworkServer)
	{
		return FALSE;
	}
	return TRUE;
}

void CNetworkServer::StopListen()
{

}