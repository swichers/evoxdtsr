#include "wsock.h"

bool WSOCK::Send(SOCKET sck,char buf[],int iBufSize)
{
	int iRet = 0;
	int WSAError = 0;
	iRet = send(sck,buf,iBufSize,0);
	if (iRet == SOCKET_ERROR) {
		WSAError = WSAGetLastError();
		if (WSAError != WSAEWOULDBLOCK) {
			//sock_error(WSAError);
			return FALSE;
		}
	}
	return TRUE;
}
int WSOCK::Recv(SOCKET sck, char buf[],int iBufSize)
{
	int iRet = recv( sck, buf, iBufSize, 0 );
	//if (iRet == SOCKET_ERROR) {
	//	int WSAError = WSAGetLastError();
	//	if(WSAError != WSAEWOULDBLOCK)
	//		//sock_error(WSAError);
	//}
	return iRet;
}
bool WSOCK::Startup(int major, int minor)
{
	WORD wVersionRequired = MAKEWORD(major,minor);
	int iRet = 0;
	iRet = WSAStartup(wVersionRequired, &wsaData);
	if (iRet != 0 ) {
		//sock_error(WSAGetLastError());
		return FALSE;
	} else if (wsaData.wVersion != wVersionRequired) {
		//sock_error(WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}
bool WSOCK::Shutdown()
{
	int iRet = 0;
	if(sktMain || sktClient || sktServer) {
		iRet = Disconnect();
		if(iRet == FALSE) {
			return FALSE;
		}
	}

	iRet = WSACleanup();
	if (iRet == SOCKET_ERROR) {
//		sock_error(WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

bool WSOCK::Create()
{
	sktMain = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if(sktMain == INVALID_SOCKET) {
		//sock_error(WSAGetLastError());
		return FALSE;	
	}
	sktClient = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if(sktClient == INVALID_SOCKET) {
		//sock_error(WSAGetLastError());
		return FALSE;	
	}

	return TRUE;
}
bool WSOCK::Disconnect(){
	if(sktMain != NULL) {
		closesocket( sktMain );
		sktMain = NULL;
	}

	if(sktClient != NULL) {
		closesocket( sktClient );
		sktClient = NULL;
	}

	if(sktServer != NULL) {
		closesocket( sktServer );
		sktServer = NULL;
	}
	return TRUE;
}

bool WSOCK::Connect(HWND hWnd, char ip[], int iPort)
{
	int iRet = 0;
	saClient.sin_family			= AF_INET;
	saClient.sin_addr.s_addr	= inet_addr ( ip );
	saClient.sin_port			= htons ( iPort );
	iRet = WSAConnect(sktClient,(LPSOCKADDR)&saClient,sizeof sockaddr,NULL,NULL,NULL,NULL);
	//iRet = connect( sktClient, (LPSOCKADDR)&saClient, sizeof sockaddr );
	if(iRet == SOCKET_ERROR) {
		//sock_error(WSAGetLastError());
		return FALSE;
	}

	iRet = WSAAsyncSelect( sktClient, hWnd, WM_SOCKET_NOTIFY, FD_READ|FD_CLOSE );
	if(iRet == SOCKET_ERROR) {
		//sock_error(WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

bool WSOCK::Listen(HWND hWnd, int iPort)
{
	int iRet = 0;
	saServer.sin_family	= AF_INET;
	saServer.sin_port = htons(iPort);
	saServer.sin_addr.s_addr = htonl(INADDR_ANY);
	iRet = WSAAsyncSelect(sktMain, hWnd, WM_SOCKET_NOTIFY, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE );
	if(iRet == SOCKET_ERROR) {
		//sock_error(WSAGetLastError());
		return FALSE;
	}
	iRet = bind(sktMain, (LPSOCKADDR)&saServer, sizeof sockaddr);
	if(iRet == SOCKET_ERROR) {
		//sock_error(WSAGetLastError());
		return FALSE;
	}

	iRet = listen(sktMain, MAX_CON);
	if(iRet == SOCKET_ERROR) {
		//sock_error(WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

bool WSOCK::Accept(HWND hWnd, SOCKET skt)
{
	int	iLen = sizeof(SOCKADDR_IN);
	int iRet = 0;

	sktServer = accept(skt,(LPSOCKADDR)&saServer,&iLen);
	if(sktServer == INVALID_SOCKET){
		//sock_error(WSAGetLastError());
		return FALSE;
	}

	iRet = WSAAsyncSelect(sktServer, hWnd, WM_SOCKET_NOTIFY, FD_READ | FD_WRITE | FD_CLOSE);
	if(sktServer == SOCKET_ERROR){
		//sock_error(WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

bool WSOCK::Close(SOCKET skt)
{
	closesocket(skt);
	if(skt == this->sktClient)
		sktClient = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	else if(skt == this->sktMain)
		sktMain = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	return TRUE;
}