#pragma once
#include <windows.h>
BOOL CALLBACK SettingsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void Settings_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
BOOL Settings_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Settings_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
void LoadSettings(HWND hwnd);
void SaveSettings(HWND hwnd);
void CloseSettings(HWND hwnd);
void ShowSettings(HWND hwnd);
