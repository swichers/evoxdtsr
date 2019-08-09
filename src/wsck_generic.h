#pragma once
#pragma comment (lib,"ws2_32.lib")		//Tells linker to include lib file
#include <winsock2.h>					//WinSock header file
#include <windows.h>					//Needed for various defines and macros
										//Can be taken out if those are added
										//manually.

#ifndef WM_SOCKET_NOTIFY
#define WM_SOCKET_NOTIFY	WM_USER+1	//Define a custom message for winsock
#endif

#define CONNECTED		true		//Make some English substitutes
#define DISCONNECTED	false		//for connection status

class GENSOCK
{
private:
protected:
	struct CONNFO {			//Structure for connections
		SOCKET	skt;		//Socket connected on
		UINT	Port;		//Port connected on
		char	szIP[30];	//IP connected on
		//Things to add:
		//Time established (Can compute time connected from this)
		//Numerical IP
	};
	UINT iMaxCon;				//Maximum number of allowed connections
	int LastError;
public:
	GENSOCK::GENSOCK();
	void GENSOCK::SetMaxCon(UINT iMax);
	void GENSOCK::ErrBox(HWND hwnd, const char szMsg[]);
	bool GENSOCK::Startup(const UINT major, const UINT minor);	//Loads winsock
	int GENSOCK::Recv(const SOCKET skt, char szBuf[], const UINT iSize);				//Receive data
	bool GENSOCK::Send(const SOCKET skt, const char szBuf[], const UINT iSize);		//Send specified data
	bool GENSOCK::Send(const SOCKET skt, const char szBuf[]);						//Send plain text
};