#ifndef TXTFUNCS_H
#define TXTFUNCS_H
#include <windows.h>

ULONG StrToHex(const char szNum[]);
char * HexToStr(ULONG lNum, char szBuf[]);
bool TextToClip(HWND hwnd,const char szBuf[]);
bool IsHexStr(const char szStr[]);
#endif