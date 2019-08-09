#include "logwin.h"
#include "resource.h"
#include <windowsx.h>

extern HWND hLogWnd;
extern HWND ghWnd;
extern bool show_log;

BOOL CALLBACK LogDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
		HANDLE_MSG(hwnd,WM_SYSCOMMAND,Log_OnSysCommand);
		HANDLE_MSG(hwnd,WM_INITDIALOG,Log_OnInitDialog);
		HANDLE_MSG(hwnd,WM_SIZE,Log_OnSize);
		case WM_ACTIVATE: {
			switch(LOWORD(wParam)) {
				case WA_ACTIVE:
				case WA_CLICKACTIVE: {
					if((HWND)lParam != ghWnd) {
						ShowWindow(ghWnd,SW_SHOWNA);
					}
					return 0; }
				case WA_INACTIVE: {
					break; }
			}
			break; }
		default: {
			return FALSE; }
    }
    return TRUE;
}
void Log_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case SC_CLOSE:	{
			CloseLog(hwnd);
			break; }
	}
}
BOOL Log_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	return true;
}
void CloseLog(HWND hwnd)
{
	//hLogWnd = NULL;
	//DestroyWindow(hwnd);
	show_log = false;
	ShowWindow(hwnd,SW_HIDE);
	HMENU hMenu = GetMenu(ghWnd);
	CheckMenuItem(hMenu,ID_SETTINGS_SHOWLOG,MF_UNCHECKED);
}
void Log_OnSize(HWND hwnd, UINT state, int cx, int cy) {
	switch(state) {
		case SIZE_RESTORED: {
			ShowWindow(ghWnd,SW_RESTORE);
			break; }
	}
#define MOD 15
	const HWND hLog = GetDlgItem(hwnd,IDC_EDIT_LOG);
	RECT rMainWin = {0,0,0,0};
	GetClientRect(hwnd,&rMainWin);
	SetWindowPos(hLog,NULL,MOD,MOD,rMainWin.right-(MOD*2),rMainWin.bottom - (MOD*2),SWP_NOZORDER);
}