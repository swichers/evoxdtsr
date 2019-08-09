#include <windowsx.h>
#include "dumpwin.h"
#include "resource.h"
#include <sstream>
using namespace std;

//#include "defines.h"
extern HWND hDumpWnd;
extern vector<DUMPDATA> MemDumps;

BOOL CALLBACK DumpWinDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
		HANDLE_MSG(hwnd,WM_SYSCOMMAND,DumpWin_OnSysCommand);
		HANDLE_MSG(hwnd,WM_INITDIALOG,DumpWin_OnInitDialog);
		HANDLE_MSG(hwnd,WM_COMMAND,DumpWin_OnCommand);
        default:
            return FALSE;
    }
    return TRUE;
}
void DumpWin_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
	switch(id)
	{
		case IDC_BUT_DWIN_OKAY: {
			CloseDumpWin(hwnd);
			break; }
		//case IDC_LIST_DWIN_DHIST: {
		//	if(codeNotify == LBN_SELCHANGE) {
		//		int iDumpNum = (int)SendDlgItemMessage(hwnd,IDC_LIST_DWIN_DHIST,LB_GETCURSEL,0,0);
		//		stringstream ss;
		//		for(UINT i = 0;i<MemDumps[iDumpNum].sHEX.size();i+=47) {
		//			ss << MemDumps[iDumpNum].sHEX.substr(i,47) << "\r\n";
		//		}
		//		SetDlgItemText(hwnd,IDC_EDIT_DWIN_HEX,(char*)ss.str().c_str());

		//	}
		//	break; }
	}
}
void DumpWin_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case SC_CLOSE:	{
			CloseDumpWin(hwnd);
			break; }
	}
}
BOOL DumpWin_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	//for(UINT i = 0;i<MemDumps.size();i++) {
	//	stringstream ss;
	//	ss << i+1 << ".) " << MemDumps[i].szStartOffset << uppercase \
	//		<< ": " << MemDumps[i].iBytes << "b";
	//	ListBox_InsertString(GetDlgItem(hwnd,IDC_LIST_DWIN_DHIST),i,(char*)ss.str().c_str());
	//}
	return true;
}
void CloseDumpWin(HWND hwnd)
{
	hDumpWnd = NULL;
	DestroyWindow(hwnd);
}
