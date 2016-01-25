#pragma once
#ifndef HWBP_H
#define HWBP_H
#include "Globals.h"
#define DBG_COND_EXECUTE	0
#define DBG_COND_WRITE		1
#define DBG_COND_IORW		2
#define DBG_COND_READWRITE  3
#define DR7_DEFAULT 1024
//
//	Address   - address to break on.
//	Length    - must be 0, 1, 2 or 4
//  Condition - must be 0, 1 or 3
//
#define LEN_BYTE 0
#define LEN_WORD 1
#define LEN_DWORD 3

DWORD GetW3TlsForIndex(DWORD index);
void ClearHardwareBPs();
void SetBreakpointBits(DWORD &Reg,BYTE Type,BYTE Length,BYTE Position);
void SetAccessBP(DWORD Addr);
BOOL SetHardwareBPs(DWORD *  Offsets,BYTE * Types,BYTE * Lengths,int Count);
void SuspendAllThreads();
void ResumeAllThreads();

#endif