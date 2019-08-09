#pragma once
#include <windows.h>

#define MAX_BP_COUNT	4
#define WM_BREAKPOINT	WM_APP+2
#define BP_NEWBREAK 0	//lParam contains text offset
#define BP_MODBREAK 1	//lParam contains offset id
#define BP_REFRESH	2	//lParam is NULL, just tells window there was a new breakpoint or adjustment

struct BREAKPOINT {
	enum TYPES { EXE,READ,READWRITE,WRITE };
	enum SIZES { SBYTE,SWORD,SDWORD,SEXECUTE };
	BREAKPOINT::BREAKPOINT() {
		iType=BREAKPOINT::EXE;
		iSize=BREAKPOINT::SEXECUTE;
		bEnable=false;
		szOffset[0]=0;
	};
	//BYTE iNum;
	TYPES iType;
	SIZES iSize;
	bool bEnable;
	//long lOffset;
	char szOffset[9];
};

struct HITBP
{
	HITBP::HITBP();
	BREAKPOINT bp;
	//ULONG offset;
	ULONG eax;
	ULONG ebx;
	ULONG ecx;
	ULONG edx;
	ULONG esi;
	ULONG edi;
	ULONG ebp;
	ULONG hit;
	BYTE num;
	//BYTE type;
	//enum Flags {EXE, READ, WRITE, READWRITE};
	//enum Types {exe, bw, brw, ww, wrw, dw, drw, size};
	//bool enabled;
	//Flags Flag;
};

//struct BREAKPOINT {
//	enum Flags {EXE, READ, WRITE, READWRITE};
//	enum Types {exe, bw, brw, ww, wrw, dw, drw, size};
//	Flags Flag;
//	char offset[9];
//	BYTE type;
//	bool enabled;
//};