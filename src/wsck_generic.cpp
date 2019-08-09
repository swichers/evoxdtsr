#include "wsck_generic.h"
#include <string>
void GENSOCK::ErrBox(HWND hwnd, const char szMsg[])
{
	using namespace std;
	string sMessage = szMsg;
	char szBuf[10] = "";
	if(sMessage.size() > 0)
		sMessage += "\n\n";
	sMessage += "The WinSock function returned: ";
	sMessage += itoa(LastError,szBuf,10);
	MessageBox(hwnd,(char*)sMessage.c_str(),"WinSock Error",MB_OK|MB_ICONHAND);
}
void GENSOCK::SetMaxCon(UINT iMax)
{
	iMaxCon = iMax;
}
bool GENSOCK::Send(const SOCKET skt,		//Socket to send data on
				const char szBuf[])			//Text buffer to send
{
	return Send(skt,szBuf,(int)strlen(szBuf));	//Call other send function w/ strlen
}
bool GENSOCK::Send(const SOCKET skt,		//Socket to send data on
				const char szBuf[],			//Buffer to send
				const UINT iSize)			//Size of the buffer
{
	int iRet = send(skt, szBuf, iSize, 0);	//iRet contains the number of bytes sent
	if(iRet == SOCKET_ERROR) {				//There was a problem sending
		LastError = WSAGetLastError();
		return false;
	}
	return true;
}
int GENSOCK::Recv(SOCKET skt,	//Socket to receive on
			char szBuf[],		//Buffer to read data into
			const UINT iSize)	//Size of the buffer
{
	int iBytes = recv(skt, szBuf, iSize, 0);	//iBytes contains the number of bytes read
	return iBytes;				//Return the number of bytes read
}
bool GENSOCK::Startup(const UINT major,	//Major version of WinSock to use
					const UINT minor)	//Minor version of WinSock to use
{
	WORD wReq = MAKEWORD(major,minor);	//Put major+minor into a word
	WSADATA wsaData;
	ZeroMemory(&wsaData,sizeof(WSADATA));	//Zero out wsaData
	int iRet = WSAStartup(wReq, &wsaData);	//Try to start winsock
	if (iRet != 0 ) {
		LastError = WSAGetLastError();
		return false;
	} else if (wsaData.wVersion != wReq) {	//The version returned wasn't the one requested
		//wrong version, handle error
		LastError = WSAGetLastError();
		return false;
	}
	return true;
}
GENSOCK::GENSOCK()
{
	iMaxCon = 0;
}