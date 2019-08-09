#ifndef WSOCK_H
#define WSOCK_H
#include <winsock2.h>

#define WM_SOCKET_NOTIFY	WM_USER+1	//Windows message to watch for
#define MAX_CON				3

class WSOCK {
	WSADATA		wsaData;
	SOCKADDR_IN	saServer;
	SOCKADDR_IN saClient;
public:
	SOCKET		sktMain;
	SOCKET		sktServer;
	SOCKET		sktClient;

	int Recv(SOCKET sck,char buf[], int iBufSize);		//Recieve data
	bool Send(SOCKET sck,char buf[], int iBufSize);		//Send data
	bool Startup(int major, int minor);					//Start WinSock up
	bool Shutdown();									//Shutdown WinSock
	bool Create();										//Create a socket
	bool Disconnect();									//Disconnect all sockets
	bool Connect(HWND hWnd, char ip[], int iPort);		//Connect to a server
	bool Accept(HWND hWnd, SOCKET skt);					//Accept incoming connection
	bool Listen(HWND hWnd, int iPort);					//Listen for incoming connections
	bool Close(SOCKET skt);								//Close a socket
};

#endif
