#pragma once
#include <commctrl.h>

class LISTVIEW {
	HWND hLV;
	int iColCount;
	int iRowCount;
public:
	HWND hWnd;
	LISTVIEW() { iColCount=iRowCount=0; hLV=hWnd=NULL; }
	void SetHWND(HWND hw) {
		hWnd = hLV = hw;
	}
	HWND GetHWND() {
		return hLV;
	}
	int InsertCol(char szTitle[]) {
		return InsertCol(szTitle,0);
	}
	int InsertCol(char szTitle[], int iColWidth) {
		LVCOLUMN lv;
		ZeroMemory(&lv,sizeof LVCOLUMN);
		lv.mask = LVCF_TEXT;
		lv.pszText = szTitle;
		lv.cchTextMax = (int)strlen(szTitle);

		if(iColWidth > 0) {
			lv.mask = lv.mask | LVCF_WIDTH;
			lv.cx = iColWidth;
		}
		int iRet = ListView_InsertColumn(hLV,iColCount,&lv);
		if(iRet != -1)
			iColCount++;
		return iRet;
	}
	int InsertItem(char szText[]) {
		return InsertItem(szText,0);
	}
	int InsertItem(char szText[],int iColNum) {
		LVITEM lv;
		ZeroMemory(&lv,sizeof LVITEM);
		lv.iItem = iRowCount;
		lv.iSubItem = 0;
		int iRet = ListView_InsertItem(hLV,&lv);
		if(iRet != -1) {
			SetText(iRowCount,iColNum,szText);
			iRowCount++;
		}
		return iRet;
	}
	void SetText(int iRowNum, int iColNum, char szText[]) {
		ListView_SetItemText(hLV, iRowNum, iColNum, szText);
	}
	//char * GetText(int iRowNum, int iColNum, int iSize) {
	//	char * pBuf = new char[iSize];
	//	ListView_GetItemText(hLV,iRowNum, iColNum, pBuf, iSize);
	//	delete [] pBuf;
	//	return pBuf;
	//}
	void SetStyle(long lStyle) {
		long lOldStyle = (long)SendMessage(hLV,LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
		lOldStyle = lOldStyle | lStyle;
		SendMessage(hLV,LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lOldStyle);
	}
	BOOL RemoveItem(int iRow) {
		BOOL bRet = ListView_DeleteItem(hLV,iRow);
		if(bRet)
			iRowCount--;
		return bRet;
	}
	BOOL ClearList() {
		BOOL bRet = ListView_DeleteAllItems(hLV);
		if(bRet)
			iRowCount = 0;
		return bRet;
	}
	BOOL SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
	{
		return ListView_SortItems(hLV, pfnCompare, lParamSort);
	}
};