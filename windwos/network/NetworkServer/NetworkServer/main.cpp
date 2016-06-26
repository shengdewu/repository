#include <Windows.h>
#include <iostream>
#include "NetworkServer.h"

int main(int argc, char **argv)
{
	system("pause");
	CNetworkServer	*pNetServer;
	pNetServer = new CNetworkServer;

	char c = 0;
	std::cin >> c;

	std::cout << "start listen" << std::endl;
	pNetServer->StartListen(12345);

	std::cin >> c;
	std::cout << "stop listen" << std::endl;
	pNetServer->StopListen();

	std::cin >> c;
	std::cout << "release net" << std::endl;
	delete pNetServer;

	system("pause");
	return 0;
}