#include <windowsx.h>
#include <string>
#include <vector>
#include "resource.h"
#include "bpwin.h"
#include "defines.h"
#include "txtfuncs.h"
#include "bpclass.h"
#include "listview.h"
#include "xboxfuncs.h"
using namespace std;

void BPWin_OnSize(HWND hwnd, UINT state, int cx, int cy);
LRESULT BPWin_OnNotify(HWND hwnd, int id, LPNMHDR pnm);
int CALLBACK CompareFunct(LPARAM i1, LPARAM i2, LPARAM lParamSort);

extern void APIENTRY ShowPopUpMenu(HWND hwnd, int PopUp, POINT pt);
extern bool TextToClip(HWND hwnd,const char szBuf[]);
extern LRESULT CALLBACK OffsetWndProc(HWND hwnd, UINT mMsg, WPARAM wParam, LPARAM lParam);
extern HWND hBPWnd;
extern vector<HITBP> PastBPList;
extern BREAKPOINT bp[4];

int iBPLimit = 0;

BOOL CALLBACK BPWinDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message) {
		HANDLE_MSG(hwnd,WM_COMMAND,BPWin_OnCommand);
		HANDLE_MSG(hwnd,WM_INITDIALOG,BPWin_OnInitDialog);
		HANDLE_MSG(hwnd,WM_SYSCOMMAND,BPWin_OnSysCommand);
		HANDLE_MSG(hwnd,WM_SIZE,BPWin_OnSize);
		HANDLE_MSG(hwnd,WM_NOTIFY,BPWin_OnNotify);
		case WM_BREAKPOINT: {
			if(PastBPList.size() > 0)
				ShowBP(hwnd,(UINT)PastBPList.size()-1);
			if((iBPLimit > 0) && (IsDlgButtonChecked(hwnd,IDC_CHECK_AUTOUNFREEZE) == BST_CHECKED)) {
				iBPLimit--;
				SetDlgItemInt(hwnd,IDC_EDIT_AUTOUNFREEZE,iBPLimit,false);
				FreezeGame(false);
			}
			break; }
        default:
            return FALSE;
    }
    return TRUE;
}
void BPWin_OnSize(HWND hwnd, UINT state, int cx, int cy) {
#define MOD 15
	const HWND hList = GetDlgItem(hwnd,IDC_LIST_REGISTERS);
	const HWND hFrez = GetDlgItem(hwnd,IDC_BUT_BPWIN_FREEZE);
	const HWND hUnfr = GetDlgItem(hwnd,IDC_BUT_BPWIN_UNFREEZE);
	const HWND hOkay = GetDlgItem(hwnd,IDC_BUT_BPWIN_OK);
	const HWND hAuto = GetDlgItem(hwnd,IDC_CHECK_AUTOUNFREEZE);
	const HWND hEdit = GetDlgItem(hwnd,IDC_EDIT_AUTOUNFREEZE);
	const HWND hSta1 = GetDlgItem(hwnd,IDC_STATIC_FOR);
	const HWND hSta2 = GetDlgItem(hwnd,IDC_STATIC_BREAKS);

	RECT rMainWin = {0,0,0,0}, rFrzBut = {0,0,0,0}, rUnfBut = {0,0,0,0}, \
		rOkay = {0,0,0,0}, rAuto = {0,0,0,0}, rEdit = {0,0,0,0}, \
		rSta1 = {0,0,0,0}, rSta2 = {0,0,0,0};
	GetClientRect(hFrez,&rFrzBut);
	GetClientRect(hUnfr,&rUnfBut);
	GetClientRect(hOkay,&rOkay);
	GetClientRect(hwnd,&rMainWin);
	GetClientRect(hAuto,&rAuto);
	GetClientRect(hEdit,&rEdit);
	GetClientRect(hSta1,&rSta1);
	GetClientRect(hSta2,&rSta2);
	
	SetWindowPos(hAuto,NULL,MOD, \
							rMainWin.bottom-MOD-rAuto.bottom,0,0,SWP_NOZORDER|SWP_NOSIZE);

	SetWindowPos(hSta1,NULL,MOD+rAuto.right+5, \
							rMainWin.bottom-MOD-rSta1.bottom-5,0,0,SWP_NOZORDER|SWP_NOSIZE);

	SetWindowPos(hEdit,NULL,rAuto.right+MOD+rSta1.right+10, \
							rMainWin.bottom-MOD-rEdit.bottom-3,0,0,SWP_NOZORDER|SWP_NOSIZE);


	SetWindowPos(hSta2,NULL,MOD+rAuto.right+rSta1.right+rEdit.right+20, \
							rMainWin.bottom-MOD-rSta2.bottom-5,0,0,SWP_NOZORDER|SWP_NOSIZE);


	SetWindowPos(hFrez,NULL,rMainWin.right-MOD*3-rUnfBut.right-rFrzBut.right-rOkay.right, \
							rMainWin.bottom-MOD-rFrzBut.bottom,0,0,SWP_NOZORDER|SWP_NOSIZE);

	SetWindowPos(hUnfr,NULL,rMainWin.right-MOD*2-rOkay.right-rUnfBut.right, \
							rMainWin.bottom-MOD-rUnfBut.bottom,0,0,SWP_NOZORDER|SWP_NOSIZE);

	SetWindowPos(hOkay,NULL,rMainWin.right-MOD-rOkay.right, \
							rMainWin.bottom-MOD-rOkay.bottom,0,0,SWP_NOZORDER|SWP_NOSIZE);

	SetWindowPos(hList,NULL,MOD,MOD,rMainWin.right-(MOD*2),rMainWin.bottom - rUnfBut.bottom -(MOD*3),SWP_NOZORDER);

	RedrawWindow(hwnd,NULL,NULL,RDW_INTERNALPAINT);
}
BOOL BPWin_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{

	LISTVIEW lv;
	lv.SetHWND(GetDlgItem(hwnd,IDC_LIST_REGISTERS));
	lv.SetStyle(LVS_EX_FULLROWSELECT | LVS_EX_FLATSB | LVS_EX_GRIDLINES);
	lv.InsertCol("",20);
	lv.InsertCol("Set On",68);
	lv.InsertCol("Hit On",68);
	lv.InsertCol("EAX",68);
	lv.InsertCol("EBX",68);
	lv.InsertCol("ECX",68);
	lv.InsertCol("EBP",68);
	lv.InsertCol("ESI",68);
	lv.InsertCol("EDX",68);
	lv.InsertCol("EDI",68);
	lv.InsertCol("#",20);
	lv.InsertCol("Size",45);
	lv.InsertCol("Type",45);
	SetDlgItemInt(hwnd,IDC_EDIT_AUTOUNFREEZE,iBPLimit,false);

	const int iNewBP = (int)PastBPList.size()-1;
	if(iNewBP < 0)
		return false;
	for(int iPlace = 0;iPlace <= iNewBP;iPlace++) {
		ShowBP(hwnd,iPlace);
	}

	return true;
}
LRESULT BPWin_OnNotify(HWND hwnd, int id, LPNMHDR pnm)
{
	if(pnm->hwndFrom == GetDlgItem(hwnd,IDC_LIST_REGISTERS))
	{
		NMITEMACTIVATE* pnmia = (NMITEMACTIVATE*) pnm;
		switch(pnm->code)
		{
			case NM_RCLICK: {
				if(pnmia->iItem == -1)
					break;
				POINT pt = pnmia->ptAction;
				ClientToScreen(pnm->hwndFrom,&pt);
				ShowPopUpMenu(hwnd,IDR_MENU_BPHIST,pt);
				return true; }
			case LVN_COLUMNCLICK: {
				NMLISTVIEW* pnmlv = (NMLISTVIEW*)pnm;
				static bool odd_comp = false;
				odd_comp = !odd_comp;
				//if(odd_comp)
				//	odd_comp=false;
				//else
				//	odd_comp=true;

				ListView_SortItemsEx(GetDlgItem(hwnd,IDC_LIST_REGISTERS), CompareFunct, odd_comp);
				break; }
		}
	}
	return false;
}
void BPWin_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
	switch(id)
	{
		case IDC_CHECK_AUTOUNFREEZE: {
			if(IsDlgButtonChecked(hwnd,IDC_CHECK_AUTOUNFREEZE) == BST_CHECKED) {
				iBPLimit = GetDlgItemInt(hwnd,IDC_EDIT_AUTOUNFREEZE,false,false);
				if(iBPLimit < 0)
					iBPLimit = 0;
			}
			else {
				iBPLimit = 0;
			}
			break; }
		case IDC_BUT_BPWIN_FREEZE: {
			FreezeGame(true);
			break; }
		case IDC_BUT_BPWIN_UNFREEZE: {
			FreezeGame(false);
			break; }
		case IDC_BUT_BPWIN_OK: {
			CloseBPWin(hwnd);
			break; }
		case ID_BPHISTPOP_REMOVEALL: {
			const HWND hLV = GetDlgItem(hwnd,IDC_LIST_REGISTERS);
			PastBPList.clear();
			ListView_DeleteAllItems(hLV);
			break; }
		case ID_BPHISTPOP_REMOVE: {
			const HWND hLV = GetDlgItem(hwnd,IDC_LIST_REGISTERS);
			int iItem = ListView_GetSelectionMark(hLV);
			if(iItem >= 0) {
				ListView_DeleteItem(hLV,iItem);
				PastBPList.erase(PastBPList.begin()+iItem);
			}
			break; }
		case ID_BPHISTPOP_COPYROW: {
			const HWND hLV = GetDlgItem(hwnd,IDC_LIST_REGISTERS);
			const int iPos = ListView_GetSelectionMark(hLV);
			if(iPos >= 0) {
				char szBuf[9] = "";
				string sF = "";
				const HWND hHead = ListView_GetHeader(hLV);
				const int iColNum = Header_GetItemCount(hHead);

				for(int iCol = 0;iCol<iColNum;iCol++) {
					ListView_GetItemText(hLV,iPos,iCol,szBuf,9);
					switch(iCol) {
						case 0: {
							sF += "List num"; break; }
						case 1: {
							sF += "Set On"; break; }
						case 2: {
							sF += "Hit On"; break; }
						case 3: {
							sF += "EAX"; break; }
						case 4: {
							sF += "EBX"; break; }
						case 5: {
							sF += "ECX"; break; }
						case 6: {
							sF += "EBP"; break; }
						case 7: {
							sF += "ESI"; break; }
						case 8: {
							sF += "EDX"; break; }
						case 9: {
							sF += "EDI"; break; }
						case 10: {
							sF += "Set num"; break; }
						case 11: {
							sF += "Size"; break; }
						case 12: {
							sF += "Type"; break; }
					}
					sF += ": ";
					sF += szBuf;
					sF += "\r\n";
				}
				TextToClip(hwnd,(char*)sF.c_str());
			}
			break; }
		//case ID_BPHISTPOP_COPYITEM: {
		//	const HWND hLV = GetDlgItem(hwnd,IDC_LIST_REGISTERS);
		//	const int iPos = ListView_GetSelectionMark(hLV);
		//	if(iPos >= 0) {
		//		char szBuf[9] = "";
		//		LVITEM lv;
		//		lv.mask = LVIF_TEXT;
		//		lv.iItem = iPos;
		//		lv.iSubItem = 0;
		//		lv.pszText = szBuf;
		//		lv.cchTextMax = 9;

		//		UINT iLen = (UINT)SendDlgItemMessage(hwnd,IDC_LIST_REGISTERS,LVM_GETITEMTEXT,iPos,(LPARAM)&lv);

		//		if(iLen > 0) {
		//			//Copy to clipboard
		//			TextToClip(hwnd,szBuf);
		//		}
		//	}
		//	break; }
		default:
			break;
	}
}
void ShowBP(HWND hwnd, const UINT iBPNum) {
	if((iBPNum < 0) ||
		(iBPNum >= PastBPList.size()))
		return;

	LISTVIEW lv;
	char szBuf[9] = "";
	lv.SetHWND(GetDlgItem(hwnd,IDC_LIST_REGISTERS));
	LVITEM lvi;//003ECAC6
	ZeroMemory(&lvi,sizeof LVITEM);
	lvi.iItem = iBPNum;

	int iDispNum = iBPNum+1;
	//Need to loop thru and make sure the number isn't taken already
	for(int iPos = 0;iPos < ListView_GetItemCount(lv.GetHWND());iPos++) {
		ListView_GetItemText(lv.GetHWND(),iPos,0,szBuf,9);
		if((atoi(szBuf) - iDispNum) >= 0) {
			iDispNum = atoi(szBuf)+1;
		}
	}
	ListView_InsertItem(lv.GetHWND(),&lvi);
	lv.SetText(iBPNum,0,itoa(iDispNum,szBuf,10));
	lv.SetText(iBPNum,1,PastBPList[iBPNum].bp.szOffset);
	lv.SetText(iBPNum,2,HexToStr(PastBPList[iBPNum].hit,szBuf));
	lv.SetText(iBPNum,3,HexToStr(PastBPList[iBPNum].eax,szBuf));
	lv.SetText(iBPNum,4,HexToStr(PastBPList[iBPNum].ebx,szBuf));
	lv.SetText(iBPNum,5,HexToStr(PastBPList[iBPNum].ecx,szBuf));
	lv.SetText(iBPNum,6,HexToStr(PastBPList[iBPNum].ebp,szBuf));
	lv.SetText(iBPNum,7,HexToStr(PastBPList[iBPNum].esi,szBuf));
	lv.SetText(iBPNum,8,HexToStr(PastBPList[iBPNum].edx,szBuf));
	lv.SetText(iBPNum,9,HexToStr(PastBPList[iBPNum].edi,szBuf));
	lv.SetText(iBPNum,10,itoa(PastBPList[iBPNum].num,szBuf,10));
    switch(PastBPList[iBPNum].bp.iSize) {
		case BREAKPOINT::SIZES::SBYTE: {
			lv.SetText(iBPNum,11,"BYTE"); break; }
		case BREAKPOINT::SIZES::SWORD: {
			lv.SetText(iBPNum,11,"WORD"); break; }
		case BREAKPOINT::SIZES::SDWORD: {
			lv.SetText(iBPNum,11,"DWORD"); break; }
	}
	switch(PastBPList[iBPNum].bp.iType) {
		case BREAKPOINT::TYPES::EXE: {
			lv.SetText(iBPNum,12,"Execute"); break; }
		//case BREAKPOINT::TYPES::READ: {
		//	sTitle += "READ"; break; }
		case BREAKPOINT::TYPES::READWRITE: {
			lv.SetText(iBPNum,12,"R/W"); break; }
		case BREAKPOINT::TYPES::WRITE: {
			lv.SetText(iBPNum,12,"Write"); break; }
	}
}
void BPWin_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case SC_CLOSE:	{
			CloseBPWin(hwnd);
			break; }
	}
}
void CloseBPWin(HWND hwnd)
{
	hBPWnd = NULL;
	DestroyWindow(hwnd);
}
int CALLBACK CompareFunct(LPARAM i1, LPARAM i2, LPARAM lParamSort)
{
	char szBuf1[9] = "";
	char szBuf2[9] = "";
	const HWND hwnd = GetDlgItem(hBPWnd,IDC_LIST_REGISTERS);
	ListView_GetItemText(hwnd,(int)i1,0,szBuf1,9);
	ListView_GetItemText(hwnd,(int)i2,0,szBuf2,9);
	if(lParamSort) {
		return strcmp(szBuf1,szBuf2);
	} else {
		return strcmp(szBuf2,szBuf1);
	}
}