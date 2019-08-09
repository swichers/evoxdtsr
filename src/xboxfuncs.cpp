#include "xboxfuncs.h"
#include "defines.h"
#include <string>
#include <sstream>
#include <windows.h>
#include <vector>
#include <algorithm>
#include "resource.h"
#include "dumpwin.h"
#include "evoxcmds.h"
#include "bpclass.h"
using namespace std;

#define EVOX_DUMP_BYTECOUNT	16

extern HWND ghWnd;
extern HINSTANCE ghInst;
extern bool multibyte_prompt;
extern bool reversed_byteorder;
extern bool SendData(char data[]);
extern bool SendRawData(char data[]);
extern vector<DUMPDATA> MemDumps;
extern BOOL CALLBACK QueueDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
bool ClearBP(int iBPNum) {
	stringstream ss;
	ss << BPCLEAR << " ";
	if(iBPNum < 0) {
		ss << "*";
	} else {
		ss << iBPNum;
	}
	return SendData((char*)ss.str().c_str());
}
bool SetBP(int iBPNum, char szOffset[9], BREAKPOINT::TYPES iBPType, BREAKPOINT::SIZES iBPSize) {
	stringstream ssBuf;
	ssBuf << BPCLEAR << " " << iBPNum << CRLF;

    switch(iBPSize) {
		case BREAKPOINT::SIZES::SBYTE: {
			ssBuf << BPBYTE; break; }
		case BREAKPOINT::SIZES::SWORD: {
			ssBuf << BPWORD; break; }
		case BREAKPOINT::SIZES::SDWORD: {
			ssBuf << BPDWORD; break; }
		case BREAKPOINT::SIZES::SEXECUTE: {
			ssBuf << BPEXECUTE; break; }
	}

	ssBuf << " " << iBPNum << " " << szOffset;
	switch(iBPType) {
		case BREAKPOINT::TYPES::EXE: {
				break; }
		case BREAKPOINT::TYPES::READ: {
			ssBuf << " " << BPREAD; break; }
		case BREAKPOINT::TYPES::READWRITE: {
			ssBuf << " " << BPRW; break; }
		case BREAKPOINT::TYPES::WRITE: {
			ssBuf << " " << BPWRITE; break; }
	}
	return SendData((char*)ssBuf.str().c_str());
}
bool BPTog(int iBPNum, bool bState) {
	stringstream ss;
	if(bState)
		ss << BPENABLE;
	else
		ss << BPDISABLE;
	ss << " " << iBPNum;
    return SendData((char*)ss.str().c_str());    
}
bool FreezeGame(bool freeze)
{
	if(freeze) {
		if(SendRawData(FREEZE)) {
            SendRawData(CRLF);
			return true;
		}
	} else {
		if(SendRawData(UNFREEZE)) {
			SendRawData(CRLF);
			return false;
		}
	}
	return false;
}
bool DumpMem(const char offset[9], const char bytes[])
{
	string sBytes = bytes;
	string::size_type iPos = sBytes.find_first_not_of("0123456789");
	if(iPos != string::npos) {	//Number of bytes wasn't a number
		return false;			//Exit function early
	}
	string sPacket = DUMP_MEM;
	sPacket += " ";
	sPacket += offset;
	sPacket += " ";
	sPacket += bytes;

	DUMPDATA dd;
	dd.iBytes = atoi(bytes);

	//Save how many bytes were set to dump
	if (dd.iBytes <= EVOX_DUMP_BYTECOUNT) {				//Not many bytes were selected to dump
		dd.iLines = 1;									//So set number of lines to one
	} else {											//More than EDBC was selected to be dumped
		dd.iLines = dd.iBytes / EVOX_DUMP_BYTECOUNT;	//How many times does EDBC go into the bytes we want
			if((dd.iBytes % EVOX_DUMP_BYTECOUNT) > 0)	//Was there some left over?
				dd.iLines++;							//Yes, so increase lines by one
	}
	dd.iLinesLeft = dd.iLines;
	MemDumps.insert(MemDumps.begin(),dd);
	return SendData((char*)sPacket.c_str());
}
bool ResetXbox(bool hard)
{
	if(hard)
		return SendData(HARDRESET);
	else
		return SendData(SOFTRESET);
}
void Poke(const char szOffset[9],const char szParam[])
{
	const int iOffPad = 8 - (int)strlen(szOffset);		//Number of zeros to prefix to offset
	stringstream ssStream;						//Used for converting between types
	ULONG lOffset = 0;
	string sData = "";
	string sOffset = szOffset;
	string sParam = szParam;

	if(((UINT)(strlen(szParam)%2)) == 1) {
		sParam.insert(0,"0");
	}
	const UINT iNumBytes = (UINT)(sParam.size() / 2);	//Gets byte count ("FF"=1,"FFFF"=2)

	//Convert input to "good" hex, and convert to useful data types.
	ssStream << hex << sOffset;
	ssStream >> hex >> lOffset;
	ssStream.clear();
	ssStream.str(string());
	ssStream << hex << sParam;
	ssStream >> hex >> sData;
	ssStream.clear();
	ssStream.str(string());
	if(iNumBytes <= 1) {
		//Send single byte w/o prompting
		sData = POKE;
		sData += " ";
		sData += sOffset;
		sData += " ";
		sData += sParam;
		sData += CRLF;
		SendData((char*)sData.c_str());
		return;
	}
	char szBuf[9] = "";

	for(UINT i = 0;i<iNumBytes;i++) {		//Offsets shown in order
		ssStream << POKE << " ";
		ultoa(lOffset+i,szBuf,16);

		for(BYTE iPadding = 0;iPadding<(8-strlen(szBuf));iPadding++) {
			ssStream << "0";
		}
		ssStream << hex << szBuf;		//Normal order offsets
		ssStream << " ";
		if(reversed_byteorder)
			ssStream << hex << sData.substr(((iNumBytes-i)*2)-2,2);	//Reverse byte order
		else
			ssStream << hex << sData.substr(i*2,2);					//Normal byte order
		ssStream << CRLF;
	}
	string sTrans = ssStream.str();
	transform(sTrans.begin(), sTrans.end(), sTrans.begin(), towupper);
	ssStream.clear();
	ssStream.str(string());
	ssStream << sTrans;

	if(multibyte_prompt) {
		HWND hQDlg = CreateDialog(ghInst,MAKEINTRESOURCE(IDD_DLG_QUEUE),ghWnd,QueueDlgProc);
		if(hQDlg) {
			SetDlgItemText(hQDlg,IDC_EDIT_QUEUE_CMDS,(char*)ssStream.str().c_str());
		} else {
			SendData((char*)ssStream.str().c_str());	//Send all bytes w/o prompting
		}
	} else {
		SendData((char*)ssStream.str().c_str());
	}
}