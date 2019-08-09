#include <windowsx.h>
#include <string>
#include <vector>
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "newbpwin.h"
#include "bpclass.h"
using namespace std;

extern HWND ghWnd;
extern LRESULT CALLBACK OffsetWndProc(HWND hwnd, UINT mMsg, WPARAM wParam, LPARAM lParam);
extern BREAKPOINT bp[4];

BOOL CALLBACK NewBPWinDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message) {
		HANDLE_MSG(hwnd,WM_COMMAND,NewBPWin_OnCommand);
		HANDLE_MSG(hwnd,WM_INITDIALOG,NewBPWin_OnInitDialog);
		HANDLE_MSG(hwnd,WM_SYSCOMMAND,NewBPWin_OnSysCommand);
		case WM_BREAKPOINT: {
			switch(wParam)
			{
				case BP_NEWBREAK: {
					SetWindowText(hwnd,"Create a new breakpoint:");
					SetDlgItemText(hwnd,IDC_STATIC_SLOTORDIS,"Slot:");
					ShowWindow(GetDlgItem(hwnd,IDC_SPIN_BPNUM),SW_SHOW);
					ShowWindow(GetDlgItem(hwnd,IDC_EDIT_BPNUM),SW_SHOW);
					ShowWindow(GetDlgItem(hwnd,IDC_CHECK_ENABLED),SW_HIDE);
					CheckDlgButton(hwnd,IDC_RADIO_WRITE,BST_CHECKED);
					CheckDlgButton(hwnd,IDC_RADIO_BYTE,BST_CHECKED);
					if(strlen((char*)lParam) == 8)		//lparam has bp "offset"
						SetDlgItemText(hwnd,IDC_EDIT_NEWBP_OFFSET,(char*)lParam);
					break; }
				case BP_MODBREAK: {//lparam has bp #
					SetWindowText(hwnd,"Modify an existing breakpoint:");
					SetDlgItemText(hwnd,IDC_STATIC_SLOTORDIS,"Status:");
					ShowWindow(GetDlgItem(hwnd,IDC_SPIN_BPNUM),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd,IDC_EDIT_BPNUM),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd,IDC_CHECK_ENABLED),SW_SHOW);
					if((lParam < 0) || (lParam >= MAX_BP_COUNT))
						break;
					if(strlen(bp[lParam].szOffset) == 0) {
						SetDlgItemText(hwnd,IDC_EDIT_NEWBP_OFFSET,"00000000");
					}
					else {
						SetDlgItemText(hwnd,IDC_EDIT_NEWBP_OFFSET,bp[lParam].szOffset);
					}
					SetDlgItemInt(hwnd,IDC_EDIT_BPNUM,lParam,false);
					switch(bp[lParam].iType) {
						case BREAKPOINT::TYPES::EXE: {
							CheckDlgButton(hwnd,IDC_RADIO_EXECUTE,BST_CHECKED);
							CheckDlgButton(hwnd,IDC_RADIO_BYTE,BST_CHECKED);
							break;}
						//case BREAKPOINT::TYPES::READ: {
						//	CheckDlgButton(hwnd,IDC_RADIO_READ,BST_CHECKED);
						//	break;}
						case BREAKPOINT::TYPES::READWRITE: {
							CheckDlgButton(hwnd,IDC_RADIO_READWRITE,BST_CHECKED);
							break;}
						case BREAKPOINT::TYPES::WRITE: {
							CheckDlgButton(hwnd,IDC_RADIO_WRITE,BST_CHECKED);
							break;}
					}
					switch(bp[lParam].iSize) {
						case BREAKPOINT::SIZES::SBYTE: {
							CheckDlgButton(hwnd,IDC_RADIO_BYTE,BST_CHECKED);
							break; }
						case BREAKPOINT::SIZES::SWORD: {
							CheckDlgButton(hwnd,IDC_RADIO_WORD,BST_CHECKED);
							break; }
						case BREAKPOINT::SIZES::SDWORD: {
							CheckDlgButton(hwnd,IDC_RADIO_DWORD,BST_CHECKED);
							break; }
						case BREAKPOINT::SIZES::SEXECUTE: {
							CheckDlgButton(hwnd,IDC_RADIO_BYTE,BST_CHECKED);
							break; }
					}
					if(bp[lParam].bEnable)
						CheckDlgButton(hwnd,IDC_CHECK_ENABLED,BST_CHECKED);
					else
						CheckDlgButton(hwnd,IDC_CHECK_ENABLED,BST_UNCHECKED);
					break; }
			}

			break; }
        default:
            return FALSE;
    }
    return TRUE;
}

BOOL NewBPWin_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetWindowLong(GetDlgItem(hwnd,IDC_EDIT_NEWBP_OFFSET), GWL_WNDPROC, (LONG)OffsetWndProc);
	SendDlgItemMessage(hwnd,IDC_EDIT_NEWBP_OFFSET,EM_SETLIMITTEXT,8,0);
	SendDlgItemMessage(hwnd,IDC_SPIN_BPNUM,UDM_SETRANGE,0,MAKELONG(3,0));
	//CheckDlgButton(hwnd,IDC_RADIO_WRITE,BST_CHECKED);
	//CheckDlgButton(hwnd,IDC_RADIO_BYTE,BST_CHECKED);
	CheckDlgButton(hwnd,IDC_CHECK_ENABLED,BST_CHECKED);
	return true;
}
 void NewBPWin_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
	switch(id)
	{
		case IDC_BUT_NEWBP_OK: {
			int iBP = 0;
			char szOffBuff[9] = "";
			GetDlgItemText(hwnd,IDC_EDIT_NEWBP_OFFSET,szOffBuff,9);
			//if(IsWindowVisible(GetDlgItem(hwnd,IDC_EDIT_BPNUM))) {
				iBP = GetDlgItemInt(hwnd,IDC_EDIT_BPNUM,NULL,false);
			//}
			//else {
			//	for(;iBP < MAX_BP_COUNT;iBP++) {
			//		if(strcmp(bp[iBP].szOffset,szOffBuff) == 0) {
			//			break;
			//		}
			//	}
			//}
			if((iBP < 0) || (iBP >= MAX_BP_COUNT))
				break;

			if(strcmp(bp[iBP].szOffset,szOffBuff) != 0) {	//Offsets weren't the same
				strcpy(bp[iBP].szOffset,szOffBuff);			//Copy the new offset into the array
			}

			if(IsDlgButtonChecked(hwnd,IDC_RADIO_BYTE) == BST_CHECKED) {
				bp[iBP].iSize = BREAKPOINT::SIZES::SBYTE;
			}
			else if(IsDlgButtonChecked(hwnd,IDC_RADIO_WORD) == BST_CHECKED) {
				bp[iBP].iSize = BREAKPOINT::SIZES::SWORD;
			}
			else if(IsDlgButtonChecked(hwnd,IDC_RADIO_DWORD) == BST_CHECKED) {
				bp[iBP].iSize = BREAKPOINT::SIZES::SDWORD;
			} else {
				bp[iBP].iSize = BREAKPOINT::SIZES::SEXECUTE;
			}
			if(IsDlgButtonChecked(hwnd,IDC_RADIO_WRITE) == BST_CHECKED) {
				bp[iBP].iType = BREAKPOINT::TYPES::WRITE;
			}
			else if (IsDlgButtonChecked(hwnd,IDC_RADIO_READWRITE) == BST_CHECKED) {
				bp[iBP].iType = BREAKPOINT::TYPES::READWRITE;
			}
			else if (IsDlgButtonChecked(hwnd,IDC_RADIO_EXECUTE) == BST_CHECKED) {
				bp[iBP].iType = BREAKPOINT::TYPES::EXE;
				bp[iBP].iSize = BREAKPOINT::SIZES::SEXECUTE;
			}
			if(IsDlgButtonChecked(hwnd,IDC_CHECK_ENABLED) == BST_CHECKED) {
				bp[iBP].bEnable = true;
			}
			else {
				bp[iBP].bEnable = false;
			}
			SendMessage(ghWnd,WM_BREAKPOINT,BP_MODBREAK,iBP);
		}
		case IDC_BUT_NEWBP_CANCEL: {
			CloseNewBPWin(hwnd);
			break; }
	}
}
void NewBPWin_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case SC_CLOSE:	{
			CloseNewBPWin(hwnd);
			break; }
	}
}
void CloseNewBPWin(HWND hwnd)
{
	DestroyWindow(hwnd);
}