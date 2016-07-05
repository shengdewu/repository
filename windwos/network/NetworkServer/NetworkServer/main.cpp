#include <Windows.h>
#include <iostream>
#include "NetworkServer.h"


CNetworkServer *gp_networkserver = nullptr;
void ProcessListen(CNetworkServer *pServer)
{
	static bool bListen = false;
	if(!bListen)
	{
		bListen = true;
		pServer->StartListen(12340);
		std::cout << "listen..." << std::endl;
	}
	else
	{
		bListen = false;
		pServer->StopListen();
		std::cout << "listen stop..." << std::endl;
	}
}

void ProcessSend(CNetworkServer *pServer)
{
	char *info = {"this is server"};
	int len = strlen(info);

	gp_networkserver->SendData(info, len);
}

long __stdcall WindowProcedCmd( HWND window, unsigned int msg, WPARAM wp, LPARAM lp )
{
	switch(msg)
	{
	case WM_DESTROY:
		gp_networkserver = nullptr;
		PostQuitMessage(0);
		return 0L ;
	case WM_CHAR:
		{
			if(nullptr == gp_networkserver)
			{
				return 0L;
			}
			switch(wp)
			{
			case ' ':
				{
					ProcessListen(gp_networkserver);
				}
				break;
			case 's':
			case 'S':
				{
					ProcessSend(gp_networkserver);
				}
				break;
			default:
				break;
			}
		}
		break;
	default:
		return DefWindowProc( window, msg, wp, lp );
	}
}

void testNetworkServer()
{
	HINSTANCE hInstance;  
	hInstance = GetModuleHandle(NULL);  
	WNDCLASS Draw;  
	Draw.cbClsExtra = 0;  
	Draw.cbWndExtra = 0;  
	Draw.hCursor = LoadCursor(hInstance, IDC_ARROW);;  
	Draw.hIcon = LoadIcon(hInstance, IDI_APPLICATION);;  
	Draw.lpszMenuName = NULL;  
	Draw.style = CS_HREDRAW | CS_VREDRAW;  
	Draw.hbrBackground = (HBRUSH)COLOR_WINDOW;  
	Draw.lpfnWndProc = WindowProcedCmd;  
	Draw.lpszClassName = "test";  
	Draw.hInstance = hInstance;   


	RegisterClass(&Draw);  

	HWND hwnd = CreateWindow(  "test",  "ÍøÂç·þÎñÆ÷",    WS_OVERLAPPEDWINDOW,   
		38,  20,    640,   480, NULL,     NULL,     hInstance,    NULL);  

	ShowWindow(hwnd, SW_SHOW);      
	UpdateWindow(hwnd);   

	MSG msg;  
	while (GetMessage(&msg, NULL, 0, 0))  
	{  
		TranslateMessage(&msg);  
		DispatchMessage(&msg);  
	} 

	system("pause");
}

int main(int argc, char **argv)
{
	system("pause");
	CNetworkServer	*pNetServer;
	pNetServer = new CNetworkServer;
	gp_networkserver = pNetServer;

	testNetworkServer();

	delete pNetServer;

	system("pause");
	return 0;
}