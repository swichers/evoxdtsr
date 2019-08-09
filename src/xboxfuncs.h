#pragma once
#include "bpclass.h"
bool FreezeGame(bool freeze);
bool DumpMem(const char offset[9], const char bytes[]);
bool ResetXbox(bool hard);
void Poke(const char szOffset[9], const char szParam[]);
bool BPTog(int iBPNum, bool bState);
bool SetBP(int iBPNum, char szOffset[9], BREAKPOINT::TYPES iBPType, BREAKPOINT::SIZES iBPSize);
bool ClearBP(int iBPNum);
