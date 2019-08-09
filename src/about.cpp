#include <windowsx.h>
#include "about.h"
#include "resource.h"
#include "defines.h"
#define RELEASE		"02.06.04"
extern HWND hAbtWnd;

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
		HANDLE_MSG(hwnd,WM_SYSCOMMAND,About_OnSysCommand);
		HANDLE_MSG(hwnd,WM_INITDIALOG,About_OnInitDialog);
		case WM_LBUTTONDOWN: {
			ReleaseCapture();
			SendMessage(hwnd,WM_NCLBUTTONDOWN,HTCAPTION,0);
			break; }
        default:
            return FALSE;
    }
    return TRUE;
}
void About_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case SC_CLOSE:	{
			CloseAbout(hwnd);
			break; }
	}
}
BOOL About_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
//	SetWindowText(GetDlgItem(hwnd,IDC_STATIC_RELEASE),RELEASE);
	SetWindowText(GetDlgItem(hwnd,IDC_STATIC_RELEASE),__DATE__);
	return true;
}
void CloseAbout(HWND hwnd)
{
	hAbtWnd = NULL;
	DestroyWindow(hwnd);
}
