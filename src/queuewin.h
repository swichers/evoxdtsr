#ifndef QUEUEWIN_H
#define QUEUEWIN_H
#include <windows.h>
BOOL CALLBACK QueueDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void Queue_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
void Queue_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
void CloseQueue(HWND hwnd);
#endif