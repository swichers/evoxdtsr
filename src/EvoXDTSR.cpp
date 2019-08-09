#include "EvoXDTSR.h"
#include "resource.h"
#include "defines.h"
#include "settings.h"
#include "about.h"
#include "bpclass.h"
#include "bpwin.h"
#include "queuewin.h"
#include "logging.h"
#include "evoxcmds.h"
#include "txtfuncs.h"
#include <windowsx.h>
#include "listview.h"
#include <vector>
#include <string>
#include <sstream>
#include "xboxfuncs.h"
#include "wsck_client.h"
#include "dumpwin.h"
#include <commctrl.h>
using namespace std;

vector<DUMPDATA> MemDumps;

void GetLastErrorMsgBox(HWND hWndOwner);
void ShowBreakpoint();
void ShowLog(bool bShow);
int CALLBACK CompareFunc(LPARAM i1, LPARAM i2, LPARAM lParamSort);
bool ToggleMenuCheck(HWND hwnd, int iResID, bool bGetState);

extern BOOL CALLBACK NewBPWinDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
extern BOOL CALLBACK LogDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
CLIENT XBOX;
SOCKET sktXBOX = SOCKET_ERROR;
enum BPCOLS { STATUS, NUMBER, OFFSET, TYPE };
vector<HITBP> PastBPList;

HINSTANCE ghInst	= NULL;
HWND ghWnd			= NULL;
HWND hSetWnd		= NULL;
HWND hAbtWnd		= NULL;
HWND hBPWnd			= NULL;
HWND hDumpWnd		= NULL;
HWND hLogWnd		= NULL;
bool gConnected		= false;
bool log_enabled	= false;
bool autofreeze		= false;
bool show_log		= false;
WNDPROC origEditWndProc = NULL;
string log_path		= "";
DWORD ipaddress		= 0;
bool multibyte_prompt	= true;
bool reversed_byteorder	= true;
LOGCLS Log;

BREAKPOINT bp[4];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ghInst = hInstance;
	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof INITCOMMONCONTROLSEX;
	icce.dwICC = ICC_LISTVIEW_CLASSES | ICC_INTERNET_CLASSES;
	InitCommonControlsEx(&icce);

	HACCEL hAccel = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCEL_MAIN));

	if(NULL == (ghWnd = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_DLG_MAIN),NULL,MainDlgProc))) {
		GetLastErrorMsgBox(NULL);
	}

	MSG msg;
	BOOL bRet = false;

	while((bRet = GetMessage(&msg,NULL,0,0)) != 0) {
		if (bRet == -1)	{
			GetLastErrorMsgBox(ghWnd);
			break;
		}
		else {
			if(!(TranslateAccelerator(ghWnd,hAccel,&msg)) &&
				!(IsDialogMessage(ghWnd,&msg))) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return (int)msg.wParam;
}
BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message) {
		HANDLE_MSG(hwnd,WM_COMMAND,Main_OnCommand);
		HANDLE_MSG(hwnd,WM_NOTIFY,Main_OnNotify);
		case WM_ACTIVATE: {
			switch(LOWORD(wParam)) {
				case WA_ACTIVE:
				case WA_CLICKACTIVE: {
					if((HWND)lParam != hLogWnd) {
						if(IsWindowVisible(hLogWnd)) {
							SetWindowPos(hLogWnd,hwnd,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
							return 0;
						}
					}
				}
				case WA_INACTIVE: {
					break; }
			}
			break; }
		case WM_SIZE: {
			switch(wParam) {
				case SIZE_RESTORED: {
					if(IsWindowVisible(hLogWnd))
						ShowWindow(hLogWnd,SW_RESTORE);
					break; }
				case SIZE_MINIMIZED: {
					if(IsWindowVisible(hLogWnd))
						ShowWindow(hLogWnd,SW_MINIMIZE);
					break; }
			}
			break; }
		case WM_SOCKET_NOTIFY: {
			Main_OnSocketNotify(hwnd, (SOCKET) wParam, HIWORD(lParam), LOWORD(lParam));
			break; }
		case WM_BREAKPOINT: {
			switch(wParam) {	//wParam contains message
				case BP_MODBREAK: {	//lParam has bp number
					//Send breakpoint info
					if(lParam >= MAX_BP_COUNT)
						break;
					if(SetBP((int)lParam,bp[lParam].szOffset,bp[lParam].iType,bp[lParam].iSize)) {
						if(!bp[lParam].bEnable)		//If breakpoint is set to disable
							BPTog((int)lParam,false);	//Be sure to disable it

					} else {
						bp[lParam].iSize = BREAKPOINT::SIZES::SBYTE;
						bp[lParam].iType = BREAKPOINT::TYPES::EXE;
						bp[lParam].bEnable = false;
						bp[lParam].szOffset[0] = 0;
					}
					SendMessage(hwnd,WM_BREAKPOINT,BP_REFRESH,0);
					break; }
				case BP_NEWBREAK: {	//lParam has text offset
					for(BYTE iBPNum=0;iBPNum < MAX_BP_COUNT;iBPNum++) {
						if(strcmp(bp[iBPNum].szOffset,(char*)lParam) == 0) {
							SendMessage(hwnd,WM_BREAKPOINT,BP_MODBREAK,iBPNum);
							break;
						}
					}
					break; }
				case BP_REFRESH: {	//lParam is null
					//Refresh breakpoint list
					LISTVIEW lv;
					lv.SetHWND(GetDlgItem(hwnd,IDC_LIST_BP));
					stringstream ssBuf;	//Used for setting type text
					for(BYTE iSpot = 0;iSpot < MAX_BP_COUNT;iSpot++) {
						//ListView_SetItemState(lv.GetHWND(),iSpot,LVIS_STATEIMAGEMASK,INDEXTOSTATEIMAGEMASK(0));
						if(bp[iSpot].bEnable) {
							ListView_SetItemState(lv.GetHWND(),iSpot,INDEXTOSTATEIMAGEMASK(3), LVIS_STATEIMAGEMASK);
							//lv.SetText(iSpot,0,"E");
						}
						else {
							if(strlen(bp[iSpot].szOffset) == 0) {	//Offset isn't set
                                ListView_SetItemState(lv.GetHWND(),iSpot,INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
							}
							else {
								ListView_SetItemState(lv.GetHWND(),iSpot,INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
							}
							//lv.SetText(iSpot,0,"D");
						}
						lv.SetText(iSpot,2,bp[iSpot].szOffset);
                        switch(bp[iSpot].iSize) {
							case BREAKPOINT::SIZES::SBYTE: {
								ssBuf << "B: "; break; }
							case BREAKPOINT::SIZES::SWORD: {
								ssBuf << "W: "; break; }
							case BREAKPOINT::SIZES::SDWORD: {
								ssBuf << "DW: "; break; }
						}
						switch(bp[iSpot].iType) {
							case BREAKPOINT::TYPES::EXE: {
								ssBuf << "EXE"; break; }
							//case BREAKPOINT::TYPES::READ: {
							//	ss << "READ"; break; }
							case BREAKPOINT::TYPES::READWRITE: {
								ssBuf << "R/W"; break; }
							case BREAKPOINT::TYPES::WRITE: {
								ssBuf << "WRITE"; break; }
						}
						lv.SetText(iSpot,3,(char*)ssBuf.str().c_str());
						ssBuf.clear();
						ssBuf.str(string());
					}

					break; }
			}
			break;
		}

		HANDLE_MSG(hwnd,WM_SYSCOMMAND,Main_OnSysCommand);
		HANDLE_MSG(hwnd,WM_INITDIALOG,Main_OnInitDialog);

        default:
            return FALSE;
    }
    return TRUE;
}
LRESULT Main_OnNotify(HWND hwnd, int id, LPNMHDR pnm)
{
	if(pnm->hwndFrom == GetDlgItem(hwnd,IDC_LIST_MEM))
	{
		NMITEMACTIVATE* pnmia = (NMITEMACTIVATE*) pnm;
		switch(pnm->code)
		{
			case NM_DBLCLK: {
				if(pnmia->iItem == -1)
					break;
				char * pBuf = new char[9];
				ListView_GetItemText(GetDlgItem(hwnd,IDC_LIST_MEM),pnmia->iItem , 0, pBuf, 9);
				SetDlgItemText(hwnd,IDC_EDIT_MEMADDY,pBuf);
				delete [] pBuf;
				return true; }
			case NM_RCLICK: {
				if(pnmia->iItem == -1)
					break;
				POINT pt = pnmia->ptAction;
				ClientToScreen(pnm->hwndFrom,&pt);
				ShowPopUpMenu(hwnd,IDR_SEARCH_POPUP,pt);
				return true; }
			case LVN_COLUMNCLICK: {
				NMLISTVIEW* pnmlv = (NMLISTVIEW*)pnm;
				static bool odd_comp = false;
				odd_comp = !odd_comp;
				//if(odd_comp)
				//	odd_comp=false;
				//else
				//	odd_comp=true;

				ListView_SortItemsEx(GetDlgItem(hwnd,IDC_LIST_MEM), CompareFunc, odd_comp);
				break; }
		}
	}
	else if(pnm->hwndFrom == GetDlgItem(hwnd,IDC_LIST_BP))
	{
		NMITEMACTIVATE* pnmia = (NMITEMACTIVATE*) pnm;
		NMLVDISPINFO*	pnmdi = (NMLVDISPINFO*) pnm;
		switch(pnm->code)
		{
			case LVN_BEGINLABELEDIT: {
				HWND hEdit = ListView_GetEditControl(pnmdi->hdr.hwndFrom);
				if(hEdit == NULL) {
					break;}

				return true; }
			case LVN_ENDLABELEDIT: {
				HWND hEdit = ListView_GetEditControl(pnmdi->hdr.hwndFrom);
				if(hEdit == NULL) {
					break;}

				char szEdit[9] = "";
				GetWindowText(hEdit,szEdit,8);
				ListView_SetItemText(pnmdi->hdr.hwndFrom,
					pnmdi->item.iItem,2,szEdit);
				return true; }
			case NM_DBLCLK: {
				if((pnmia->iItem < 0) || (pnmia->iItem >= MAX_BP_COUNT))
					break;
				if(pnmia->iSubItem == 0) {//Enable/Disable breakpoint
					if(strlen(bp[pnmia->iItem].szOffset) > 0) {
						if(BPTog(pnmia->iItem,!bp[pnmia->iItem].bEnable)) {
							bp[pnmia->iItem].bEnable = !bp[pnmia->iItem].bEnable;
							SendMessage(hwnd,WM_BREAKPOINT,BP_REFRESH,0);
						}
					}
				} else {
					HWND hNewBP = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_NEWBP),hwnd,NewBPWinDlgProc);
					if(hNewBP == NULL)
						break;
					char szBuf[9] = "";
					ListView_GetItemText(GetDlgItem(hwnd,IDC_LIST_BP),pnmia->iItem,2,szBuf,9);
					if(szBuf[0] == 0) {
						SendMessage(hNewBP,WM_BREAKPOINT,BP_NEWBREAK,(LPARAM)"00000000");
					} else {
						SendMessage(hNewBP,WM_BREAKPOINT,BP_MODBREAK,pnmia->iItem);
					}
				}
				return true; }
			case NM_CLICK: {
				switch(pnmia->iSubItem) {
					case 0:	/*Status*/
					case 1: /*Number*/{
						break; }
					case 2: /*Offset*/{
						break;
						HWND hEdit = ListView_EditLabel(pnm->hwndFrom,pnmia->iItem);
						if(hEdit != NULL) {
							SetFocus(hEdit);
							//SetWindowLong(hEdit, GWL_WNDPROC, (LONG)OffsetWndProc);
							SendMessage(hEdit,EM_SETLIMITTEXT,8,0);

							char szBuf[9] = "";
							ListView_GetItemText(pnmdi->hdr.hwndFrom,pnmia->iItem,
								pnmia->iSubItem,szBuf,8);
							SetWindowText(hEdit,szBuf);

							RECT rSub = {0,0,0,0};
							ListView_GetSubItemRect(pnmdi->hdr.hwndFrom,
								pnmia->iItem,pnmia->iSubItem,LVIR_BOUNDS,&rSub);

							SetWindowPos(hEdit,NULL,rSub.left,rSub.top,
									rSub.right-rSub.left,rSub.bottom-rSub.top,
									SWP_NOZORDER);
						}
						break; }
					case 3: /*Type*/{
						break; }
					default: {
						break; }
				}
				return true; }
			case NM_RCLICK: {
				if(pnmia->iItem == -1)
					break;
				POINT pt = pnmia->ptAction;
				ClientToScreen(pnm->hwndFrom,&pt);
				ShowPopUpMenu(hwnd,IDR_BREAK_POPUP,pt);
				return true; }
		}
	}
	return false;
}
BOOL Main_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	ghWnd = hwnd;
	LoadSettings(hwnd);
	hLogWnd = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_LOGWIN),NULL,LogDlgProc);
	Log.SetWindow(GetDlgItem(hLogWnd,IDC_EDIT_LOG));
	ShowLog(show_log);

	LISTVIEW lv,lv2;
	lv.SetHWND(GetDlgItem(hwnd,IDC_LIST_MEM));
	lv.SetStyle(LVS_EX_FULLROWSELECT | LVS_EX_FLATSB | LVS_EX_GRIDLINES);
	RECT rMem = {0,0,0,0};
	GetClientRect(lv.hWnd,&rMem);
	lv.InsertCol("Offset",rMem.right-19);
	//lv.InsertItem("FFFFFFFF",0);
	//lv.InsertItem("DDDDDDDD",0);
	//lv.InsertItem("EEEEEEEE",0);
	//lv.InsertItem("CCCCCCCC",0);
	//lv.InsertItem("12315152",0);
//CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_BPWIN),ghWnd,BPWinDlgProc);
//	enum BPLISTCOL {STATUS,NUMBER,OFFSET,TYPE,HITS};
	lv2.SetHWND(GetDlgItem(hwnd,IDC_LIST_BP));

	HIMAGELIST ilBPList = ImageList_LoadBitmap(ghInst,MAKEINTRESOURCE(IDB_BITMAP_BPLIST),10,0,CLR_DEFAULT);
	SendMessage(lv2.GetHWND(),LVM_SETIMAGELIST,LVSIL_STATE,(LPARAM)ilBPList);

	lv2.SetStyle(LVS_EX_FULLROWSELECT | LVS_EX_FLATSB | LVS_EX_GRIDLINES);
	lv2.InsertCol("S",20);
	lv2.InsertCol("#",20);
	lv2.InsertCol("Offset",68);
	lv2.InsertCol("Break On",70);
	lv2.InsertItem("0",1);
	lv2.InsertItem("1",1);
	lv2.InsertItem("2",1);
	lv2.InsertItem("3",1);

	XBOX.Startup(1,1);

	LogIt(STAT_START);
	origEditWndProc = (WNDPROC)SetWindowLong(GetDlgItem(hwnd,IDC_EDIT_MEMADDY), GWL_WNDPROC, (LONG)OffsetWndProc);
	SendDlgItemMessage(hwnd,IDC_EDIT_MEMADDY,EM_SETLIMITTEXT,8,0);
	SendMessage(hwnd,WM_BREAKPOINT,BP_REFRESH,0);
	ToggleItems(gConnected);

	//HMENU hMenu = GetMenu(hwnd);
	//MENUITEMINFO mii;
	//mii.cbSize = sizeof MENUITEMINFO;
	//mii.fMask = MIIM_FTYPE|MIIM_STRING;
	//mii.fType = MFT_STRING;
	//mii.dwTypeData = "Test";
	////mii.wID = ;
	//
	////AppendMenu(hMenu,MF_ENABLED|MF_STRING,,MF_STRING);
	//InsertMenuItem(hMenu,ID_HELP_MENU,false,&mii);
	//DrawMenuBar(hwnd);
	return true;
}
void Main_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
	switch(id) {
		case ID_APP_BPHISTORYWINDOW: {
			ShowBreakpoint();
			break; }
		case ID_APP_NEWBPWINDOW: {
			HWND hNewBP = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_NEWBP),hwnd,NewBPWinDlgProc);
			if(hNewBP == NULL)
				break;
			SendMessage(hNewBP,WM_BREAKPOINT,BP_NEWBREAK,(LPARAM)"00000000");
			break; }
		case ID_APP_LOGWINDOW: {
			ShowLog(true);
			show_log = true;
			CheckMenuItem(GetMenu(hwnd),ID_SETTINGS_SHOWLOG,MF_CHECKED);
			break; }
		case ID_APP_EXITAPP: { CloseProgram(hwnd); break; }
		case ID_SETTINGS_WINDOW: {
			if(hSetWnd == NULL) {
				hSetWnd = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_SETTINGS),hwnd,SettingsDlgProc);
				if(!hSetWnd) {
					GetLastErrorMsgBox(ghWnd);
					LogIt(ERR_LOADSET);
				}
			}
			break; }
		case ID_FUNCTIONS_CONNECT: {	//Connect menu item selected
			Connect(hwnd);
			break; }
		case ID_FUNCTIONS_DISCONNECT: {
			Disconnect(sktXBOX);
			break; }
		case ID_SETTINGS_AUTOFREEZE: {
			autofreeze = ToggleMenuCheck(hwnd, ID_SETTINGS_AUTOFREEZE, false);
			break; }
		case ID_SETTINGS_FILELOG: {
			log_enabled = ToggleMenuCheck(hwnd, ID_SETTINGS_FILELOG, false);
			Log.file_enabled = log_enabled;
			break; }
		case ID_SETTINGS_SHOWLOG: {
			show_log = ToggleMenuCheck(hwnd, ID_SETTINGS_SHOWLOG, false);
			ShowLog(show_log);
			break; }
		case ID_SETTINGS_ONTOP: {
			bool bRet = ToggleMenuCheck(hwnd,ID_SETTINGS_ONTOP,false);
			if(bRet) {	//Currently on top
				SetWindowPos(hLogWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			}
			else {	//Not top most
				SetWindowPos(hLogWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			}
			break; }
		case ID_FUNCTIONS_FREEZE: {
			FreezeGame(true);
			break; }
		case ID_FUNCTIONS_UNFREEZE: {
			FreezeGame(false);
			break; }
		case ID_FUNCTIONS_QUEUECOMMANDS: {
			CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_QUEUE),hwnd,QueueDlgProc);
			break; }
		case ID_FUNCTIONS_CLEARSCREEN: {
			int iRet = MessageBox(hwnd,MNU_CLEARSCRNLOG,DEF_CAP,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
			if(iRet == IDYES)
				SetDlgItemText(hLogWnd,IDC_EDIT_LOG,"");
			break; }
		case ID_FUNCTIONS_CLEARFILE: {
			int iRet = MessageBox(hwnd,MNU_CLEARFILELOG,DEF_CAP,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
			if(iRet == IDYES)
				Log.ClearFile();
			break; }
		case ID_RESET_HARD: {
			if(SendData(HARDRESET))
				Disconnect(sktXBOX);
			break; }
		case ID_RESET_SOFT: {
			if(SendData(SOFTRESET))
				Disconnect(sktXBOX);
			break; }
		case ID_HELP_QUICKRUNDOWN: {
			MessageBox(hwnd,RUNDOWN_TEXT,HELP_CAP,MB_OK);
			break; }
		case ID_HELP_ABOUT: {
			if(hAbtWnd == NULL)
				hAbtWnd = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_ABOUT),hwnd,AboutDlgProc);
			break; }
		//Memory buttons
		case IDC_BUT_POKE: {
			UINT iOffLen	= GetWindowTextLength(GetDlgItem(hwnd,IDC_EDIT_MEMADDY))+1;
			UINT iParLen	= GetWindowTextLength(GetDlgItem(hwnd,IDC_EDIT_MEMPARAM))+1;
			char *pOffset	= new char[iOffLen];
			char *pParam	= new char[iParLen];
			ZeroMemory(pOffset,iOffLen);
			ZeroMemory(pParam,iParLen);
			GetDlgItemText(hwnd,IDC_EDIT_MEMADDY,pOffset,iOffLen);
			GetDlgItemText(hwnd,IDC_EDIT_MEMPARAM,pParam,iParLen);
			if(strlen(pOffset) == 0) {
				Log.Log(PKT_ERR_MISSADD);	//No offset
				break;
			} else if (strlen(pParam) == 0) {
				Log.Log(MEMPKT_ERR_MISSPARAM);	//No bytes
				break;
			}
			if(IsHexStr(pParam) && IsHexStr(pOffset))
				Poke(pOffset,pParam);
			delete [] pOffset;
			delete [] pParam;
			//GetDlgItemTextLength()
			break; }
		case IDC_BUT_SEARCH: {
			UINT iParLen	= GetWindowTextLength(GetDlgItem(hwnd,IDC_EDIT_MEMPARAM))+1;
			char *pParam	= new char[iParLen];
			ZeroMemory(pParam,iParLen);

			GetDlgItemText(hwnd,IDC_EDIT_MEMPARAM,pParam,iParLen);
			if (strlen(pParam) == 0) {
				Log.Log(MEMPKT_ERR_MISSPARAM);	//No bytes
				break;
			}
			ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_LIST_MEM));
			string sData = SEARCH;
			sData += " ";
			sData += pParam;
			SendData((char*)sData.c_str());
			delete [] pParam;
			break; }
		case IDC_BUT_CLEAR: {
			if(SendData(NEWSEARCH))
				ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_LIST_MEM));
			break; }
		case IDC_BUT_DUMP: {
			UINT iOffLen	= GetWindowTextLength(GetDlgItem(hwnd,IDC_EDIT_MEMADDY))+1;
			UINT iParLen	= GetWindowTextLength(GetDlgItem(hwnd,IDC_EDIT_MEMPARAM))+1;
			char *pOffset	= new char[iOffLen];
			char *pParam	= new char[iParLen];
			ZeroMemory(pOffset,iOffLen);
			ZeroMemory(pParam,iParLen);
			GetDlgItemText(hwnd,IDC_EDIT_MEMADDY,pOffset,iOffLen);
			GetDlgItemText(hwnd,IDC_EDIT_MEMPARAM,pParam,iParLen);
			if(strlen(pOffset) == 0) {
				Log.Log(PKT_ERR_MISSADD);	//No offset
				break;
			} else if (strlen(pParam) == 0) {
				Log.Log(MEMPKT_ERR_MISSPARAM);	//No bytes
				break;
			}
			DumpMem(pOffset,pParam);
			delete [] pOffset;
			delete [] pParam;
			break; }
		//case IDC_BUT_DUMPWIN: {
		//	if(hDumpWnd == NULL)
		//		hDumpWnd = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_DUMPWIN),hwnd,DumpWinDlgProc);
		//	break; }
		//Search results popup
		case ID_SEARCHPOPUP_NEWSEARCH: {
			if(SendData(NEWSEARCH))
				ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_LIST_MEM));
			break; }
		case ID_SRCH_COPY_MEMORYOFFSET: {
			SrcMnuCopyTo(IDC_EDIT_MEMADDY);
			break; }
		case ID_SRCH_CLEAR_LIST: {
			ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_LIST_MEM));
			break; }
		case ID_SRCH_REMOVE_ITEM: {
			int iPos = ListView_GetSelectionMark(GetDlgItem(ghWnd,IDC_LIST_MEM));
			if(iPos >= 0) {
				ListView_DeleteItem(GetDlgItem(hwnd,IDC_LIST_MEM),iPos);
			}
			break; }
		case ID_SRCH_COPY_CLIPBOARD: {
			SrcMnuCopyTo(0);
			break; }
		case ID_SRCH_SETBP: {
			int iPos = ListView_GetSelectionMark(GetDlgItem(hwnd,IDC_LIST_MEM));
			if(iPos == -1)
				break;
			HWND hNewBP = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_NEWBP),hwnd,NewBPWinDlgProc);
			if(hNewBP == NULL)
				break;
			char szBuf[9] = "";
			LVITEM lv;
			lv.mask = LVIF_TEXT;
			lv.iItem = iPos;
			lv.iSubItem = 0;
			lv.pszText = szBuf;
			lv.cchTextMax = 9;

			UINT iLen = (UINT)SendDlgItemMessage(hwnd,IDC_LIST_MEM,LVM_GETITEMTEXT,iPos,(LPARAM)&lv);
			if(iLen <= 0)
				break;
			SendMessage(hNewBP,WM_BREAKPOINT,BP_NEWBREAK,(LPARAM)szBuf);

			break; }
		//Breakpoint popup
		case ID_BPPOPUP_MODIFY: {
			int iPos = ListView_GetSelectionMark(GetDlgItem(hwnd,IDC_LIST_BP));
			if((iPos == -1) || (iPos >= MAX_BP_COUNT))
				break;
			HWND hNewBP = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_NEWBP),hwnd,NewBPWinDlgProc);
			if(hNewBP == NULL)
				break;
			char szBuf[9] = "";
			ListView_GetItemText(GetDlgItem(hwnd,IDC_LIST_BP),iPos,2,szBuf,9);
			if(szBuf[0] == 0) {
				SendMessage(hNewBP,WM_BREAKPOINT,BP_NEWBREAK,(LPARAM)"00000000");
				break; }
			SendMessage(hNewBP,WM_BREAKPOINT,BP_MODBREAK,iPos);
			break; }
		case ID_BPPOPUP_CLEAR: {
			int iPos = ListView_GetSelectionMark(GetDlgItem(hwnd,IDC_LIST_BP));
			if((iPos == -1) || (iPos >= MAX_BP_COUNT))
				break;
			ClearBP(iPos);
			bp[iPos].bEnable = false;
			bp[iPos].iSize = BREAKPOINT::SIZES::SEXECUTE;
			bp[iPos].iType = BREAKPOINT::TYPES::EXE;
			bp[iPos].szOffset[0] = 0;
			SendMessage(hwnd,WM_BREAKPOINT,BP_REFRESH,0);
			break; }
		case ID_BPPOPUP_CLEARALL: {
			//ClearBP(-1);
			for(BYTE iPos=0;iPos<MAX_BP_COUNT;iPos++) {
				ClearBP(iPos);
				bp[iPos].bEnable = false;
				bp[iPos].iSize = BREAKPOINT::SIZES::SEXECUTE;
				bp[iPos].iType = BREAKPOINT::TYPES::EXE;
				bp[iPos].szOffset[0] = 0;
			}
			SendMessage(hwnd,WM_BREAKPOINT,BP_REFRESH,0);
			break; }
		case ID_BPPOPUP_LIST: {
			SendData(BPLIST);
			break; }
		case ID_BPPOPUP_STAT_ENABLE: {
			int iPos = ListView_GetSelectionMark(GetDlgItem(hwnd,IDC_LIST_BP));
			if((iPos == -1) || (iPos >= MAX_BP_COUNT))
				break;
			if(BPTog(iPos,true)) {
				bp[iPos].bEnable = true;
				SendMessage(hwnd,WM_BREAKPOINT,BP_REFRESH,0);
			}
			break; }
		case ID_BPPOPUP_STAT_DISABLE: {
			int iPos = ListView_GetSelectionMark(GetDlgItem(hwnd,IDC_LIST_BP));
			if((iPos == -1) || (iPos >= MAX_BP_COUNT))
				break;
			if(BPTog(iPos,false)) {
				bp[iPos].bEnable = false;
				SendMessage(hwnd,WM_BREAKPOINT,BP_REFRESH,0);
			}
			break; }
	}
}

void Main_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd) {
		case SC_CLOSE: {
			CloseProgram(hwnd);
			break; }
	}
}
void Main_OnSocketNotify(HWND hwnd, SOCKET skt, WORD err, WORD message)
{
	switch(message) {
		case FD_READ: {		//Data ready to be read?
			ReadData(skt);	//Read data from the buffer
			break; }
		case FD_CONNECT:
		case FD_WRITE:
		case FD_OOB:
		case FD_ACCEPT:
			break;
		case FD_CLOSE:
		default: {			//Socket was closed by server
			Disconnect(skt);
			break; }
	}
}
void CloseProgram(HWND hwnd)
{
	if(gConnected)		//Are we connected?
		Disconnect(sktXBOX);	//Disconnects all sockets
	SaveSettings(hwnd);
	DestroyWindow(hwnd);
	PostQuitMessage(0);
}
bool Connect(HWND hwnd) {
	if(!gConnected) {		//Not connected?
		stringstream sStream;
		string sIP = "";
		sStream << FIRST_IPADDRESS(ipaddress) << "." << \
			SECOND_IPADDRESS(ipaddress) << "." << \
			THIRD_IPADDRESS(ipaddress) << "." << \
			FOURTH_IPADDRESS(ipaddress);
		sStream >> sIP;
		sktXBOX = XBOX.Connect(hwnd,(char*)sIP.c_str(),PORT);
		if(sktXBOX != SOCKET_ERROR) {
			gConnected = true;
		}
		if(!gConnected) {	//If the connect call was not successful
			LogIt(SOCK_ERRNOCONNECT);
		} else {
			LogIt(SOCK_CONNECTED);
		}
		//SendDlgItemMessage(hwnd,IDC_LIST_SEARCHRESULTS,LB_RESETCONTENT,0,0);
		ListView_DeleteAllItems(GetDlgItem(hwnd,IDC_LIST_MEM));
	} else {
		LogIt(SOCK_ALREADYCON);
	}
	ToggleItems(gConnected);
	return gConnected;
}
bool Disconnect(SOCKET skt) {
	if(gConnected) {
		SendData(skt,QUIT_MSG);
		XBOX.Close(skt);
		LogIt(SOCK_DISCONNECTED);
		gConnected = false;
	} else {
		LogIt(SOCK_ERR_NOTCON);
	}

	for(BYTE i = 0;i < MAX_BP_COUNT;i++) {
		bp[i].bEnable = false;
		bp[i].szOffset[0] = 0;
		bp[i].iType = BREAKPOINT::TYPES::EXE;
		bp[i].iSize = BREAKPOINT::SIZES::SBYTE;
	}
	SendMessage(ghWnd,WM_BREAKPOINT,BP_REFRESH,0);
	ListView_DeleteAllItems(GetDlgItem(ghWnd,IDC_LIST_MEM));
	for(BYTE iPos=0;iPos<MAX_BP_COUNT;iPos++) {
		bp[iPos].bEnable = false;
		bp[iPos].iSize = BREAKPOINT::SIZES::SBYTE;
		bp[iPos].iType = BREAKPOINT::TYPES::EXE;
		bp[iPos].szOffset[0] = 0;
	}
	SendMessage(ghWnd,WM_BREAKPOINT,BP_REFRESH,0);
	ToggleItems(gConnected);
	return gConnected;
}
void LogIt(char szLogTxt[])
{
	SetDlgItemText(ghWnd,IDC_STATIC_LASTLINE,szLogTxt);
	Log.Log(szLogTxt);
}
void ReadData(SOCKET skt)		//Reads all data currently on the winsock buffer
{
	using namespace std;
	static string PktBuf = "";		//Buffer for packets
	vector<string> Lines;
	static string sBPNum = "", sBPOffset = "", sEAX = "", sEBX = "", sECX = "",
				sEDX = "",  sESI = "", sEDI = "",  sEBP = "";

	while(true) {					//Loop forever
		char szBuf[1024] = "";		//Buffer for winsock function
		int iRet = XBOX.Recv(skt,szBuf,1024);	//Read data from the winsock buffer
		if(iRet <= 0)							//If there was no data, or an error
			break;								//Exit the loop
		PktBuf.append(szBuf,iRet);				//Otherwise add it to the main buffer
	}

	if(PktBuf.empty())
		return;

	//PktBuf.append("\n\r",2);

	while(true) {
		string::size_type iPos = PktBuf.find("\n\r");
		if(iPos == string::npos) {
			//if(!PktBuf.empty()) {
			//	Lines.push_back(PktBuf);
			//}
			break;
		}
		Lines.push_back(PktBuf.substr(0,iPos));
		PktBuf.erase(0,iPos+2);
	}

	for(UINT iCurLine = 0;iCurLine < Lines.size();iCurLine++) {

#ifndef DEBUG
		for(int iCurChar = 0; iCurChar < (int)Lines[iCurLine].size(); iCurChar++) {
			if(Lines[iCurLine][iCurChar] == 0x1B) {
				Lines[iCurLine].erase(0,iCurChar+3);
				iCurChar -= 3;
			} else if((Lines[iCurLine][iCurChar] < ' ') ||
					(Lines[iCurLine][iCurChar] > '~')) {
				Lines[iCurLine].erase(iCurChar,1);
				iCurChar--;
			}
			if(iCurChar < -1)
				iCurChar = -1;
		}
#endif
		if(Lines[iCurLine].size() == 8) {
			if(IsHexStr((char*)Lines[iCurLine].c_str())) {
				LISTVIEW lv;
				lv.SetHWND(GetDlgItem(ghWnd,IDC_LIST_MEM));
				lv.InsertItem((char*)Lines[iCurLine].c_str(),0);
			}
		}
		else if (Lines[iCurLine].size() == 14) {
			if(Lines[iCurLine].substr(0,6) == "EAX : ") {
				sEAX = Lines[iCurLine].substr(6,8);
			}
			else if(Lines[iCurLine].substr(0,6) == "EBX : ") {
				sEBX = Lines[iCurLine].substr(6,8);
			}
			else if(Lines[iCurLine].substr(0,6) == "ECX : ") {
				sECX = Lines[iCurLine].substr(6,8);
			}
			else if(Lines[iCurLine].substr(0,6) == "EDX : ") {
				sEDX = Lines[iCurLine].substr(6,8);
			}
			else if(Lines[iCurLine].substr(0,6) == "ESI : ") {
				sESI = Lines[iCurLine].substr(6,8);
			}
			else if(Lines[iCurLine].substr(0,6) == "EDI : ") {
				sEDI = Lines[iCurLine].substr(6,8);
			}
			else if(Lines[iCurLine].substr(0,6) == "EBP : ") {
				sEBP = Lines[iCurLine].substr(6,8);

				HITBP bpTemp;
				bpTemp.num = atoi((char*)sBPNum.c_str());

				bpTemp.bp.iSize = bp[bpTemp.num].iSize;
				bpTemp.bp.iType = bp[bpTemp.num].iType;
				strcpy(bpTemp.bp.szOffset,bp[bpTemp.num].szOffset);
				bpTemp.hit = StrToHex((char*)sBPOffset.c_str());
				bpTemp.eax = StrToHex((char*)sEAX.c_str());
				bpTemp.ebp = StrToHex((char*)sEBP.c_str());
				bpTemp.ebx = StrToHex((char*)sEBX.c_str());
				bpTemp.ecx = StrToHex((char*)sECX.c_str());
				bpTemp.edi = StrToHex((char*)sEDI.c_str());
				bpTemp.edx = StrToHex((char*)sEDX.c_str());
				bpTemp.esi = StrToHex((char*)sESI.c_str());
				PastBPList.push_back(bpTemp);

				ShowBreakpoint();
			}
		}
		else if (Lines[iCurLine].size() == 15) {
//	<Glom> .Break
//<Glom> BP 0 @ 000c1203
//<Glom> EAX : 00000007
//<Glom> EBX : 00000002
//<Glom> ECX : 00000007
//<Glom> EDX : 0000003f
//<Glom> ESI : 8310c6b4
//<Glom> EDI : 83091fe0
//<Glom> EBP : 83091e48
			if((Lines[iCurLine].substr(0,3) == "BP ") &&
			((Lines[iCurLine][3] >= '0') && (Lines[iCurLine][3] <= '3')) &&
			(Lines[iCurLine].substr(4,3) == " @ ")) {
				sEDX = "";	sEBP = "";	sEBX = "";
				sECX = "";	sEDI = "";	sEDX = "";
				sESI = "";

				sBPNum = Lines[iCurLine].substr(3,1);
				sBPOffset = Lines[iCurLine].substr(7,8);
			}
		}
		else if (Lines[iCurLine].size() == 78) {
			//memory dump line
			//83e58000 : 10 a3 04 00 1f 00 b0 7c 70 1d 04 00 5f df 08 00  | .#....0|p...__..
			//Check if 10th byte is :, check if 61 is |
			if((Lines[iCurLine][9] == ':') && (Lines[iCurLine][60] == '|')) {
				if(MemDumps[0].iLinesLeft > 0) {
					if(MemDumps[0].iLinesLeft == MemDumps[0].iLines)
						strcpy(MemDumps[0].szStartOffset,Lines[iCurLine].substr(0,8).c_str());	//offset
					MemDumps[0].sHEX += Lines[iCurLine].substr(11,47);							//dump data
					MemDumps[0].sASCII += Lines[iCurLine].substr(62);							//text
					MemDumps[0].iLinesLeft--;
				}
			}
		}
//Nr : Adresse  Type       Status
//0  : adeeea0e Execution  Disabled
//1  : Empty
//2  : 003ecac4 Write      Disabled
//3  : 00000000 Read Write Disabled
		LogIt((char*)Lines[iCurLine].c_str());
	}
}
bool SendRawData(char data[])
{
	if(strlen(data) == 0)		//See if there is something to be sent
		return false;				//If not, exit the function
	if(!gConnected) {
		LogIt(SOCK_ERR_NOTCON);
		return false;
	}
	return XBOX.Send(sktXBOX,data);
}
bool SendData(char data[])
{
	return SendData(sktXBOX,data);
}
bool SendData(SOCKET skt, char data[])
{
	if(strlen(data) == 0)		//See if there is something to be sent
		return false;				//If not, exit the function
	if(!gConnected) {
		LogIt(SOCK_ERR_NOTCON);
		return false;
	}
	if(autofreeze) {
		XBOX.Send(skt,FREEZE);
		XBOX.Send(skt,CRLF);
	}
	XBOX.Send(skt,data);
	XBOX.Send(skt,CRLF);
	if(autofreeze) {
		XBOX.Send(skt,UNFREEZE);
		XBOX.Send(skt,CRLF);
	}

	return true;
}
LRESULT CALLBACK OffsetWndProc(HWND hwnd, UINT mMsg, WPARAM wParam, LPARAM lParam)
{
	switch (mMsg)
	{
		case WM_CHAR: {
		   if(((wParam >= '0') && (wParam <= '9')) ||
				((wParam >= 'A') && (wParam <= 'F')) ||
				((wParam >= 'a') && (wParam <= 'f')) ||
				(wParam == VK_BACK) || (wParam < ' ')) {
				return CallWindowProc(origEditWndProc, hwnd, mMsg, wParam, lParam);
			}
			break; }
		case WM_PASTE: {
			if(OpenClipboard(ghWnd)) {
				HANDLE hData = GetClipboardData(CF_TEXT);
				if(hData == NULL)
					break;

				string sBuf = (char*)GlobalLock(hData);
				GlobalUnlock(hData);
				CloseClipboard();

				while(true) {
					string::size_type iPos = sBuf.find_first_not_of("abcdefABCDEF1234567890");
					if(iPos == string::npos) { break; }
					sBuf.erase(iPos,1);
				}
				sBuf = sBuf.substr(0,8);	//Only paste first 8 valid characters
				SetWindowText(hwnd,(char*)sBuf.c_str());
			}
			break; }
		default: {
			return CallWindowProc(origEditWndProc, hwnd, mMsg, wParam, lParam);
		}
	}
	return false;
}
void ToggleItems(bool enable)
{
	EnableWindow(GetDlgItem(ghWnd,IDC_EDIT_MEMADDY),enable);
	EnableWindow(GetDlgItem(ghWnd,IDC_EDIT_MEMPARAM),enable);
	EnableWindow(GetDlgItem(ghWnd,IDC_BUT_POKE),enable);
	EnableWindow(GetDlgItem(ghWnd,IDC_BUT_SEARCH),enable);
	EnableWindow(GetDlgItem(ghWnd,IDC_BUT_CLEAR),enable);
	EnableWindow(GetDlgItem(ghWnd,IDC_BUT_DUMP),enable);
	EnableWindow(GetDlgItem(ghWnd,IDC_LIST_MEM),enable);
	EnableWindow(GetDlgItem(ghWnd,IDC_LIST_BP),enable);
	HMENU hMenu = GetMenu(ghWnd);
	long mnuTog = MF_GRAYED;
	if(enable==true) {
		EnableMenuItem(hMenu,ID_FUNCTIONS_CONNECT,MF_GRAYED);
		EnableMenuItem(hMenu,ID_FUNCTIONS_DISCONNECT,MF_ENABLED);
		mnuTog = MF_ENABLED;
	} else {
		EnableMenuItem(hMenu,ID_FUNCTIONS_CONNECT,MF_ENABLED);
		EnableMenuItem(hMenu,ID_FUNCTIONS_DISCONNECT,MF_GRAYED);
		mnuTog = MF_GRAYED;
	}
	EnableMenuItem(hMenu,ID_FUNCTIONS_FREEZE,mnuTog);
	EnableMenuItem(hMenu,ID_FUNCTIONS_UNFREEZE,mnuTog);
	EnableMenuItem(hMenu,ID_RESET_HARD,mnuTog);
	EnableMenuItem(hMenu,ID_RESET_SOFT,mnuTog);
	EnableMenuItem(hMenu,ID_FUNCTIONS_QUEUECOMMANDS,mnuTog);
}

void APIENTRY ShowPopUpMenu(HWND hwnd, int PopUp, POINT pt)
{
    HMENU hmenu;            // top-level menu

    if ((hmenu = LoadMenu(ghInst, MAKEINTRESOURCE(PopUp))) == NULL)
        return;
    HMENU hmenuTrackPopup = GetSubMenu(hmenu, 0);
    TrackPopupMenu(hmenuTrackPopup,
            TPM_LEFTALIGN | TPM_RIGHTBUTTON,
            pt.x, pt.y, 0, hwnd, NULL);
	DestroyMenu(hmenu);
}

void SrcMnuCopyTo(int CPT)
{
	int iPos = ListView_GetSelectionMark(GetDlgItem(ghWnd,IDC_LIST_MEM));
	if(iPos == -1)
		return;
	char szBuf[9] = "";
	LVITEM lv;
	lv.mask = LVIF_TEXT;
	lv.iItem = iPos;
	lv.iSubItem = 0;
	lv.pszText = szBuf;
	lv.cchTextMax = 9;

	UINT iLen = (UINT)SendDlgItemMessage(ghWnd,IDC_LIST_MEM,LVM_GETITEMTEXT,iPos,(LPARAM)&lv);
	if(iLen <= 0)
		return;

	switch(CPT)
	{
		case IDC_EDIT_MEMADDY: {
			SetDlgItemText(ghWnd,IDC_EDIT_MEMADDY,szBuf);
			break; }
		case 0: {
			TextToClip(ghWnd,szBuf);
			break; }
	}
}
void ShowBreakpoint()
{
	if(!hBPWnd) {
		hBPWnd = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_BPWIN),ghWnd,BPWinDlgProc);
		if(!hBPWnd) {
			LogIt(BPWIN_ERR_NOLOAD);
			return; }
	} else {
		PostMessage(hBPWnd,WM_BREAKPOINT,BP_REFRESH,0);
	}
}
void GetLastErrorMsgBox(HWND hWndOwner)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
	MessageBox(0, (LPCTSTR)lpMsgBuf, NULL, MB_OK|MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
}

void ShowLog(bool bShow)
{
	if(bShow) {
		ShowWindow(hLogWnd,SW_SHOWNA);
	}
	else {
		ShowWindow(hLogWnd,SW_HIDE);
	}
}
int CALLBACK CompareFunc(LPARAM i1, LPARAM i2, LPARAM lParamSort)
{
	char szBuf1[9] = "";
	char szBuf2[9] = "";
	const HWND hwnd = GetDlgItem(ghWnd,IDC_LIST_MEM);
	ListView_GetItemText(hwnd,(int)i1,0,szBuf1,9);
	ListView_GetItemText(hwnd,(int)i2,0,szBuf2,9);
	if(lParamSort) {
		return strcmp(szBuf1,szBuf2);
	} else {
		return strcmp(szBuf2,szBuf1);
	}
}
bool ToggleMenuCheck(HWND hwnd, int iResID, bool bGetState) {
	HMENU hMenu = GetMenu(hwnd);
	MENUITEMINFO mnuinf;
	ZeroMemory(&mnuinf,sizeof(MENUITEMINFO));
	bool bRet = false;
	mnuinf.cbSize = sizeof(MENUITEMINFO);
	mnuinf.fMask = MIIM_STATE;
	GetMenuItemInfo(hMenu,iResID,false,&mnuinf);
	if(mnuinf.fState == MFS_CHECKED) {
		bRet = true;
	} else {
		bRet = false;
	}

	//if(bGetState) {
	//	bRet = !bRet;
	//}
	//else {
		if(bRet) {
			CheckMenuItem(hMenu,iResID,MF_UNCHECKED);
		}
		else {
			CheckMenuItem(hMenu,iResID,MF_CHECKED);
		}
	//}

	return !bRet;
}