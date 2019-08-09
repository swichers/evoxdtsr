#include <windows.h>
#include <richedit.h>
#include "logging.h"
using namespace std;

LOGCLS::LOGCLS()
{
	iSoftLimit = DEF_CAPACITY;
	iHardLimit = DEF_CAPACITY;
	file_enabled = false;
	hEdit = NULL;
	LogPath = DEF_LOG;
}
LOGCLS::~LOGCLS()
{
	if(file.is_open())
		file.close();
}
bool LOGCLS::Open()
{
	file.open((char*)LogPath.c_str(), ios::out | ios::trunc);
	if(file.is_open()) {	//Log file couldn't be opened
		return true;
	}
	return false;
}
void LOGCLS::Log(char szData[])
{
	string Data = szData;
	Screen(Data);
	if(file_enabled)
		File(Data);
}
void LOGCLS::Log(string Data)
{
	Screen(Data);
	if(file_enabled)
		File(Data);
}
void LOGCLS::SetWindow(HWND hWndEdit)
{
	hEdit = hWndEdit;
	iHardLimit = (int)SendMessage(hWndEdit,EM_GETLIMITTEXT,0,0);
	//iSoftLimit = iHardLimit;
}
void LOGCLS::SetPath(string Path)
{
	if(file.is_open()) {
		file.close();
	}
	LogPath = Path;
}
void LOGCLS::Screen(string Data)
{
	Data += CRLF;
	SendMessage(hEdit,EM_SETSEL,0,-1);
	SendMessage(hEdit,EM_SETSEL,-1,-1);
	SendMessage(hEdit,EM_REPLACESEL,0,(LPARAM)(char*)Data.c_str());
	int iRet = GetWindowTextLength(hEdit);
	if((iRet >= iHardLimit) || (iRet >= iSoftLimit))
		SetWindowText(hEdit,"");
}
void LOGCLS::File(string Data)
{
	if(!file.is_open())		//If the file isn't open
		if(!Open())			//Try to open it
			return;			//Couldn't open, so leave
	file << Data << endl;	//Log the data
}
bool LOGCLS::ClearFile()
{
	if(file.is_open())
		file.close();
	remove((char*)LogPath.c_str());
	return Open();
}