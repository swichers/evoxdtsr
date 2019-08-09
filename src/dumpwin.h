#pragma once
#include <windows.h>
#include <string>
#include <vector>
using namespace std;

struct DUMPDATA
{
	DUMPDATA() { sASCII="";sHEX="";szStartOffset[0]=iLines=iLinesLeft=iBytes=0; };
	string sASCII;
	string sHEX;
	char szStartOffset[9];
	UINT iLines;
	UINT iLinesLeft;
	UINT iBytes;
};

BOOL CALLBACK DumpWinDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void DumpWin_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
BOOL DumpWin_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void CloseDumpWin(HWND hwnd);
void DumpWin_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );