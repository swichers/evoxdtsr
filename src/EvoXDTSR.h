#ifndef EVOXDTSR_H
#define EVOXDTSR_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "wsock.h"

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
void Main_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
BOOL Main_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Main_OnSocketNotify(HWND hwnd, SOCKET skt, WORD error, WORD message);
void Main_OnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify );
LRESULT Main_OnNotify(HWND hwnd, int id, LPNMHDR pnm);
LRESULT CALLBACK OffsetWndProc(HWND hwnd, UINT mMsg, WPARAM wParam, LPARAM lParam);
void CloseProgram(HWND hwnd);
bool Connect(HWND hwnd);
bool Disconnect(SOCKET skt);
void LogIt(char szLogTxt[]);
void ReadData(SOCKET skt);
bool SendData(SOCKET skt, char data[]);
bool SendRawData(char data[]);
bool SendData(char data[]);
bool BPPacket(const char Type[]);
VOID APIENTRY ShowPopUpMenu(HWND hwnd, int PopUp, POINT pt);
void SrcMnuCopyTo(int CPT);
void ToggleItems(bool enable);
#endif
