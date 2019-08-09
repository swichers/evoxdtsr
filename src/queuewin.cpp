#include <windowsx.h>
#include "queuewin.h"
#include "resource.h"
#include "defines.h"
#include <fstream>
#include <string>
using namespace std;

extern HWND ghWnd;
extern bool SendData(char data[]);
extern void APIENTRY ShowPopUpMenu(HWND hwnd, int PopUp, POINT pt);

BOOL CALLBACK QueueDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
		HANDLE_MSG(hwnd,WM_SYSCOMMAND,Queue_OnSysCommand);
		HANDLE_MSG(hwnd,WM_COMMAND,Queue_OnCommand);
        default:
            return FALSE;
    }
    return TRUE;
}

void Queue_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case SC_CLOSE:	{
			CloseQueue(hwnd);
			break; }
	}
}
void Queue_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
		case ID_QUEUEPOPUP_SAVE: {
			int iSize = GetWindowTextLength(GetDlgItem(hwnd,IDC_EDIT_QUEUE_CMDS));
			if(iSize <= 0)
				break;
			OPENFILENAME ofn;
			char szLogPath[1024] = "";
			ZeroMemory( &ofn, sizeof(OPENFILENAME) );

			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Saved Queue (*.que)\0*.que\0Text Files (*.txt)\0*.txt\0Log Files (*.log)\0*.log\0All Files (*.*)\0*.*\0";
			ofn.nMaxFile = 1024;
			ofn.lpstrFile = szLogPath;
			ofn.lpstrInitialDir = szLogPath;
			ofn.lpstrTitle = "Save queued commands";
			ofn.lpstrDefExt = "que";
			ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
			if(GetSaveFileName(&ofn)==TRUE) {
				ofstream file(ofn.lpstrFile,ios::out | ios::trunc);
				if(file.is_open()) {
					char *pBuf = new char[iSize+1];
					GetWindowText(GetDlgItem(hwnd,IDC_EDIT_QUEUE_CMDS),pBuf,iSize+1);
					file << pBuf;
					file.close();
					delete [] pBuf;
				}
			}
			break; }
		case ID_QUEUEPOPUP_LOAD: {
			OPENFILENAME ofn;
			char szLogPath[1024] = "";
			ZeroMemory( &ofn, sizeof(OPENFILENAME) );

			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Open Queue (*.que)\0*.que\0Text Files (*.txt)\0*.txt\0Log Files (*.log)\0*.log\0All Files (*.*)\0*.*\0";
			ofn.nMaxFile = 1024;
			ofn.lpstrFile = szLogPath;
			ofn.lpstrInitialDir = szLogPath;
			ofn.lpstrTitle = "Open queued commands file";
			ofn.lpstrDefExt = "que";
			ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
			if(GetOpenFileName(&ofn)==TRUE) {
				ifstream file(ofn.lpstrFile,ios::in);
				if(file.is_open()) {
					string sFileBuf = "";
					char cBuf = 0;
					while(file.good()) {
						file.get(cBuf);
						if(cBuf != '\n') {
							sFileBuf += cBuf;
						} else {
							if(sFileBuf[sFileBuf.size()-1] != '\r')
								sFileBuf += "\r\n";
						}
					}
					file.close();
					SetDlgItemText(hwnd,IDC_EDIT_QUEUE_CMDS,(char*)sFileBuf.c_str());
				}
			}
			break; }
		case IDC_BUT_QUEUE_MENU: {
			RECT rBut;
			if(!GetWindowRect(GetDlgItem(hwnd,IDC_BUT_QUEUE_MENU),&rBut))
				break;
			POINT pt = {rBut.right-(rBut.right-rBut.left),rBut.bottom};
			ShowPopUpMenu(hwnd,IDR_QUEUE_BUTMENU,pt);
			break; }
		case IDC_BUT_QUEUE_CLEAR: {
			SetDlgItemText(hwnd,IDC_EDIT_QUEUE_CMDS,"");
			break; }
		case IDC_BUT_CMDS_EXECUTE: {
			int iSize = GetWindowTextLength(GetDlgItem(hwnd,IDC_EDIT_QUEUE_CMDS));
			if(iSize <= 0)
				break;
			char *pBuf = new char[iSize+1];
			GetWindowText(GetDlgItem(hwnd,IDC_EDIT_QUEUE_CMDS),pBuf,iSize+1);
			SendData(pBuf);
			delete [] pBuf;
			break; }
		case IDC_BUT_CMDS_CANCEL: { CloseQueue(hwnd); break; }
		default:
			break;
	}
}
void CloseQueue(HWND hwnd)
{
	DestroyWindow(hwnd);
}