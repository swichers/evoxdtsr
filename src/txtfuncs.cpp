#include <string>
#include "txtfuncs.h"

ULONG StrToHex(const char szNum[])
{
	if(szNum[1] != 'x') {
		char *pTemp = new char[strlen(szNum)+3];
		strcpy(pTemp,"0x");
		strcat(pTemp,szNum);
		ULONG lConv = strtoul(pTemp,NULL,16);
		delete [] pTemp;
		return lConv;
	}
	return strtoul(szNum,NULL,16);				
}
char * HexToStr(ULONG lNum, char szBuf[])
{
	using namespace std;
	string sBuf = ultoa(lNum,szBuf,16);
	for(int i = 0;sBuf.size() < 8;i++) {
		sBuf.insert(0,"0");
	}
	strcpy(szBuf,(char*)sBuf.c_str());
	for(UINT i = 0;i < strlen(szBuf);i++) {
		szBuf[i] = toupper(szBuf[i]);
	}
	return szBuf;
}
bool TextToClip(HWND hwnd,const char szBuf[])
{
	if(!OpenClipboard(hwnd))
		return false;
	HGLOBAL cbuffer;
	char *buffer;
	EmptyClipboard();
	cbuffer = GlobalAlloc(GMEM_DDESHARE,strlen(szBuf)+1);
	buffer = (char*)GlobalLock(cbuffer);
	strcpy(buffer, szBuf);
	GlobalUnlock(cbuffer);
	if(SetClipboardData(CF_TEXT,cbuffer) == NULL) {
		CloseClipboard();
		return false;
	}
	CloseClipboard();
	return true;
}
bool IsHexStr(const char szStr[])
{
	bool bRet = true;
	for(UINT iPos = 0;iPos < strlen(szStr);iPos++) {
		if(((szStr[iPos] >= '0') && (szStr[iPos] <= '9')) ||
			((szStr[iPos] >= 'A') && (szStr[iPos] <= 'F')) ||
			((szStr[iPos] >= 'a') && (szStr[iPos] <= 'f'))) {
				//bRet = true;
				continue;
		} else {
			bRet = false;
			break;
		}
	}
	return bRet;
}