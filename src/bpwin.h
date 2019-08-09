#pragma once
#include <windows.h>
void ShowBP(HWND hwnd, const UINT iBPNum);
BOOL BPWin_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void BPWin_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
void BPWin_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
BOOL CALLBACK BPWinDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void CloseBPWin(HWND hwnd);