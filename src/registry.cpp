#include <windows.h>
#include "registry.h"

REG::REG()
{
	RegCreateKeyEx(HKEY_CURRENT_USER, SUBKEY, 0,
		NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, 
		NULL, &hKey, NULL);
}
int REG::ReadInt(char valname[])
{
	DWORD iVal = 0;
	DWORD iSize = sizeof DWORD;
	DWORD type = 0;
	if(hKey)
		RegQueryValueEx(hKey,valname,0,&type,(LPBYTE)&iVal,&iSize);
	return iVal;
}
DWORD REG::ReadDword(char valname[])
{
	DWORD iVal = 0;
	DWORD iSize = sizeof DWORD;
	DWORD type = 0;
	if(hKey)
		RegQueryValueEx(hKey,valname,0,&type,(LPBYTE)&iVal,&iSize);
	return iVal;
}
void REG::ReadStr(char valname[],char buf[],DWORD bufsize)
{
	DWORD type = 0;

	if(hKey)
		RegQueryValueEx(hKey,valname,0,&type,(LPBYTE)buf,&bufsize);
}
REG::~REG()
{
	if(hKey)
		RegCloseKey(hKey);
}
void REG::WriteStr(char valname[],char buf[])
{
	if(hKey)
		RegSetValueEx(hKey,valname,0,REG_SZ,(BYTE*)buf,(DWORD)strlen(buf)+1);
}
void REG::WriteInt(char valname[],int i)
{
	if(hKey)
		RegSetValueEx(hKey,valname,0,REG_DWORD,(BYTE*)&i,sizeof i);
}
void REG::WriteInt(char valname[],DWORD i)
{
	if(hKey)
		RegSetValueEx(hKey,valname,0,REG_DWORD,(BYTE*)&i,sizeof i);
}
