#ifndef LOGGING_H
#define LOGGING_H
#include <windows.h>
#include <string>
#include <fstream>
#include <richedit.h>
#include "defines.h"
using namespace std;

class LOGCLS
{
	bool Open();
	string LogPath;
	void File(string Data);
	void Screen(string Data);
	HWND hEdit;
	int iHardLimit;
public:
	ofstream file;
	int iSoftLimit;
	bool file_enabled;
	~LOGCLS();
	LOGCLS();
	void SetPath(string Path);
	void SetWindow(HWND hWndEdit);
	void Log(string Data);
	void Log(char szData[]);
	bool ClearFile();
};
#endif