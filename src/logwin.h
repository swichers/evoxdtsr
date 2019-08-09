#pragma once
#include <windows.h>
BOOL CALLBACK LogDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void Log_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
BOOL Log_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void CloseLog(HWND hwnd);
void Log_OnSize(HWND hwnd, UINT state, int cx, int cy);