#pragma once
#include <windows.h>
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void About_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
BOOL About_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void CloseAbout(HWND hwnd);
