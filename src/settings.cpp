#include <windowsx.h>
#include "settings.h"
#include "resource.h"
#include "registry.h"
#include "defines.h"
#include "logging.h"
#include <commctrl.h>
#include <string>
using namespace std;

#define SET_AUTOFREEZE	"autofreeze"
#define SET_FILELOG		"log_to_file"
#define SET_IP			"xboxip"
#define SET_LOGPATH		"log_path"
#define SET_PROMPTMULTI "prompt_multibyte"
#define SET_SCREENCAP	"screen_capacity"
#define SET_BYTEORDER	"reversed_order"
#define SET_SHOWSCREENLOG "show_screen"

#define DEF_LOGSOFTLIMIT 20000

extern HWND hSetWnd;
extern HWND ghWnd;
extern bool log_enabled;
extern bool autofreeze;
extern bool show_log;
extern DWORD ipaddress;
extern string log_path;
extern LOGCLS Log;
extern bool multibyte_prompt;
extern bool reversed_byteorder;
extern void ShowLog(bool show);
char tmplogpath[1024] = "";

BOOL CALLBACK SettingsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
		HANDLE_MSG(hwnd,WM_SYSCOMMAND,Settings_OnSysCommand);
		HANDLE_MSG(hwnd,WM_INITDIALOG,Settings_OnInitDialog);
		HANDLE_MSG(hwnd,WM_COMMAND,Settings_OnCommand);
        default:
            return FALSE;
    }
    return TRUE;
}

void Settings_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case SC_CLOSE:	{
			CloseSettings(hwnd);
			break; }
	}
}
BOOL Settings_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	ShowSettings(hwnd);
	return true;
}
void Settings_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
		case IDC_CHECK_LOG: {
			if(IsDlgButtonChecked(hwnd,IDC_CHECK_LOG) == BST_CHECKED) {
				EnableWindow(GetDlgItem(hwnd,IDC_EDIT_LOGPATH),true);
			} else {
				EnableWindow(GetDlgItem(hwnd,IDC_EDIT_LOGPATH),false);
			}
			break; }
		case IDC_BUT_LOGPATH: {
			OPENFILENAME ofn;
			ZeroMemory( &ofn, sizeof(OPENFILENAME) );

			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0Log Files (*.log)\0*.log\0All Files (*.*)\0*.*\0";
			ofn.nMaxFile = 1024;
			ofn.lpstrFile = tmplogpath;
			ofn.lpstrInitialDir = tmplogpath;
			ofn.lpstrTitle = "Save logfile";
			ofn.lpstrDefExt = "txt";
			ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
			if(GetSaveFileName(&ofn)==TRUE) {
				SetDlgItemText(hwnd,IDC_EDIT_LOGPATH,ofn.lpstrFile);
				strcpy(tmplogpath,ofn.lpstrFile);
			}
			break; }
		case IDOK: {
			DWORD ip = 0;
			SendDlgItemMessage(hwnd,IDC_IPADDRESS,IPM_GETADDRESS,0,(LPARAM)&ip);
			ipaddress = ip;
			SaveSettings(hwnd); }		//Saves settings and falls below
		case IDCANCEL: { CloseSettings(hwnd); break; }	//Closes the dialog
		default:
			break;
	}
}
void CloseSettings(HWND hwnd)
{
	hSetWnd = NULL;
	Log.file_enabled = log_enabled;
	DestroyWindow(hwnd);
}
void SaveSettings(HWND hwnd)
{
	REG Save;
	HMENU hMenu = GetMenu(ghWnd);

	if(hSetWnd != NULL) {
		if(strlen(tmplogpath) != 0) {
			log_path = tmplogpath;
			Log.SetPath(log_path);
		}
		Log.iSoftLimit = GetDlgItemInt(hwnd,IDC_EDIT_LOGCAP,false,false);
		if(IsDlgButtonChecked(hwnd,IDC_CHECK_LOG) == BST_CHECKED) {
			log_enabled = true;
		} else {
			log_enabled = false;
		}
		if(IsDlgButtonChecked(hwnd,IDC_CHECK_MULTIPROMPT) == BST_CHECKED) {
			multibyte_prompt = true;
		} else {
			multibyte_prompt = false;
		}
		if(IsDlgButtonChecked(hwnd,IDC_RADIO_BOREVERSED) == BST_CHECKED) {
			reversed_byteorder = true;
		} else {
			reversed_byteorder = false;
		}
		if(IsDlgButtonChecked(hwnd,IDC_CHECK_SHOWSCREENLOG) == BST_CHECKED) {
			if(show_log == false)
				ShowLog(true);
			show_log = true;
		} else {
			if(show_log == true)
				ShowLog(false);
			show_log = false;
		}
	}

	Save.WriteStr(SET_LOGPATH,(char*)log_path.c_str());

	int iCheckStat = MF_UNCHECKED;
	if(log_enabled)
		iCheckStat = MF_CHECKED;
	else
		iCheckStat = MF_UNCHECKED;

	Save.WriteInt(SET_FILELOG,iCheckStat);
	CheckMenuItem(hMenu,ID_SETTINGS_FILELOG,iCheckStat);

	if(autofreeze)
		iCheckStat = MF_CHECKED;
	else
		iCheckStat = MF_UNCHECKED;

	Save.WriteInt(SET_AUTOFREEZE,iCheckStat);
	CheckMenuItem(hMenu,ID_SETTINGS_AUTOFREEZE,iCheckStat);

	if(show_log)
		iCheckStat = MF_CHECKED;
	else
		iCheckStat = MF_UNCHECKED;

	Save.WriteInt(SET_SHOWSCREENLOG,iCheckStat);
	CheckMenuItem(hMenu,ID_SETTINGS_SHOWLOG,iCheckStat);

	if(multibyte_prompt)
		Save.WriteInt(SET_PROMPTMULTI,MF_CHECKED);
	else
		Save.WriteInt(SET_PROMPTMULTI,MF_UNCHECKED);

	Save.WriteInt(SET_IP,ipaddress);
	Save.WriteInt(SET_SCREENCAP,Log.iSoftLimit);
	Save.WriteInt(SET_BYTEORDER,reversed_byteorder);
}
void LoadSettings(HWND hwnd)
{
	REG Load;

	char szLogPath[1024] = "";
	Load.ReadStr(SET_LOGPATH,szLogPath,1024);
	if(strlen(szLogPath) == 0)
		log_path = DEF_LOG;
	else
		log_path = szLogPath;
	Log.SetPath(log_path);

	ipaddress = Load.ReadDword(SET_IP);

	if(ipaddress == 0)
		ipaddress = DEF_IP;

	HMENU hMenu = GetMenu(ghWnd);
	if(Load.ReadInt(SET_FILELOG) == MF_CHECKED) {
		log_enabled = true;
		CheckMenuItem(hMenu,ID_SETTINGS_FILELOG,MF_CHECKED);
	} else {
		log_enabled = false;
		CheckMenuItem(hMenu,ID_SETTINGS_FILELOG,MF_UNCHECKED);
	}
	Log.file_enabled = log_enabled;

	if(Load.ReadInt(SET_AUTOFREEZE) == MF_CHECKED) {
		autofreeze = true;
		CheckMenuItem(hMenu,ID_SETTINGS_AUTOFREEZE,MF_CHECKED);
	} else {
		autofreeze = false;
		CheckMenuItem(hMenu,ID_SETTINGS_AUTOFREEZE,MF_UNCHECKED);
	}

	if(Load.ReadInt(SET_SHOWSCREENLOG) == MF_CHECKED) {
		show_log = true;
		CheckMenuItem(hMenu,ID_SETTINGS_SHOWLOG,MF_CHECKED);
	} else {
		show_log = false;
		CheckMenuItem(hMenu,ID_SETTINGS_SHOWLOG,MF_UNCHECKED);
	}

	if(Load.ReadInt(SET_PROMPTMULTI) == MF_CHECKED) {
		multibyte_prompt = true;
	} else {
		multibyte_prompt = false;
	}
	reversed_byteorder = Load.ReadInt(SET_BYTEORDER);
	Log.iSoftLimit = Load.ReadInt(SET_SCREENCAP);
	if(Log.iSoftLimit <= 0)
		Log.iSoftLimit = DEF_LOGSOFTLIMIT;
}
void ShowSettings(HWND hwnd)
{
	if(log_enabled)
		CheckDlgButton(hwnd, IDC_CHECK_LOG,BST_CHECKED);
	else
		CheckDlgButton(hwnd, IDC_CHECK_LOG,BST_UNCHECKED);

	if(multibyte_prompt)
		CheckDlgButton(hwnd, IDC_CHECK_MULTIPROMPT,BST_CHECKED);
	else
		CheckDlgButton(hwnd, IDC_CHECK_MULTIPROMPT,BST_UNCHECKED);

	if(reversed_byteorder)
		CheckDlgButton(hwnd,IDC_RADIO_BOREVERSED,BST_CHECKED);
	else
		CheckDlgButton(hwnd,IDC_RADIO_BONORMAL,BST_CHECKED);

	if(show_log)
		CheckDlgButton(hwnd,IDC_CHECK_SHOWSCREENLOG,BST_CHECKED);
	else
		CheckDlgButton(hwnd,IDC_CHECK_SHOWSCREENLOG,BST_UNCHECKED);

	SetDlgItemText(hwnd,IDC_EDIT_LOGPATH,(char*)log_path.c_str());
	SetDlgItemInt(hwnd,IDC_EDIT_LOGCAP,Log.iSoftLimit,false);
	EnableWindow(GetDlgItem(hwnd,IDC_EDIT_LOGPATH),log_enabled);
	SendDlgItemMessage(hwnd,IDC_IPADDRESS,IPM_SETADDRESS,0,ipaddress);
}