#include "wsck_client.h"

bool CLIENT::CloseAll()
{
	while(!ConList.empty()) {			//Loop until the connection list is empty
		closesocket(ConList[0].skt);	//Close the first socket in the list
		ConList.erase(ConList.begin());	//Erase the first socket in the list
	}
	return true;
}
bool CLIENT::Close(SOCKET skt)
{
	for(UINT i = 0;i < ConList.size();i++) {	//Loop thru each connection
		if(ConList[i].skt == skt) {				//If the sockets match
			closesocket(skt);					//Close the socket
			ConList.erase(ConList.begin()+i);	//Remove the socket from the connection list
			return true;						//Get out of the for loop early
		}
	}
	return false;
}
SOCKET CLIENT::Connect(const HWND hwnd,	//Window handle to post winsock messages to
				const char szIP[30],	//IP to connect to
				const UINT iPort)		//Port to connect on
{
	if((ConList.size() == iMaxCon) &&	//If the connection limit was reached
		(iMaxCon > 0)) {				//and iMaxCon isn't 0
		return SOCKET_ERROR;
	}

	if((strlen(szIP) == 0) ||			//ip null?
		(hwnd == NULL) ||				//hwnd null?
		(iPort == 0)) {					//port null?
		return SOCKET_ERROR;			//Leave function
	}

	SOCKADDR_IN saClient;
	ZeroMemory(&saClient,sizeof(SOCKADDR_IN));	//Zero out saClient

	CONNFO NewCon;				//New connection
	NewCon.Port					= iPort;
	strcpy(NewCon.szIP,szIP);						//Set the connection ip
	NewCon.skt = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);	//Create a socket

	if(NewCon.skt == INVALID_SOCKET) {				//Is the socket valid?
		LastError = WSAGetLastError();
		return SOCKET_ERROR;
	}

	saClient.sin_family			= AF_INET;					//Must be AF_INET
	saClient.sin_addr.s_addr	= inet_addr(NewCon.szIP);	//Set ip address and convert from a string
	saClient.sin_port			= htons(NewCon.Port);				//Set port to connect to

	if(saClient.sin_addr.s_addr == INADDR_NONE) {	//Was the ip converted?
		LastError = WSAGetLastError();
		return SOCKET_ERROR;						//No, so leave
	}

	int iRet = WSAConnect(NewCon.skt, (SOCKADDR*)&saClient,
				sizeof(SOCKADDR), NULL, NULL, NULL, NULL);		//Attempt to connect
	if(iRet == SOCKET_ERROR) {
		LastError = WSAGetLastError();
		closesocket(NewCon.skt);
		return SOCKET_ERROR;
	}

	iRet = WSAAsyncSelect( NewCon.skt, hwnd, WM_SOCKET_NOTIFY,
				FD_CONNECT|FD_READ|FD_CLOSE|FD_WRITE );		//Set the socket to asynchronous
	if(iRet == SOCKET_ERROR) {
		LastError = WSAGetLastError();
		closesocket(NewCon.skt);
		return SOCKET_ERROR;
	}


	//NewCon.Status = CONNECTED;		//Connection was okay, so set status to true
	ConList.push_back(NewCon);			//Add the connection to the list
	this->skt = NewCon.skt;
	return NewCon.skt;
}

CLIENT::~CLIENT()
{
	if(ConList.size() > 0) {		//If there are connections in the list
		CloseAll();					//Clear them all out
	}
	WSACleanup();					//Clean up winsock
}
CLIENT::CLIENT()
{
	iMaxCon = 0;		//Set iMaxCon to 0 to accept unlimited connections
}

CLIENT::CLIENT(const int iMax)
{
	iMaxCon = iMax;
}