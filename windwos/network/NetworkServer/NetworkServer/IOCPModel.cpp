#include "IOCPModel.h"


CIOCPModel::CIOCPModel(void)
{
	LoadSocketLib();
}


CIOCPModel::~CIOCPModel(void)
{
	UnloadSocketLib();
}

//≥ı ºªØ Winsock 2.2
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
