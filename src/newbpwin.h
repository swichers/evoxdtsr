#pragma once
#include <windows.h>

BOOL NewBPWin_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void NewBPWin_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
void NewBPWin_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
BOOL CALLBACK NewBPWinDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void CloseNewBPWin(HWND hwnd);
