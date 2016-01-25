#include "stdafx.h"
#include "HBP.h"
void SetBreakpointBits(DWORD &Reg,BYTE Type,BYTE Length,BYTE Position)
{
	DWORD Shift = (Position<<1);
	Reg |=((1<<Shift)|(Type<<((Shift<<1)+16))|(Length<<((Shift<<1)+18)));
}
void SetAccessBP(DWORD Addr)
{
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);
	Thread32First(thSnap, &te);
	do
	{

		if(thSnap==INVALID_HANDLE_VALUE) continue;
		if(te.th32OwnerProcessID != GetCurrentProcessId()) continue;
		if(te.th32ThreadID==GetCurrentThreadId()||te.th32ThreadID==threadId1||te.th32ThreadID==threadId2||te.th32ThreadID==threadId3) continue;
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
		CONTEXT ct;
		ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		GetThreadContext(hThread, &ct);
		ct.Dr3 = Addr;
		ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		DWORD err=SuspendThread(hThread);
		SetThreadContext(hThread, &ct);
		err=ResumeThread(hThread);
		if(err!=1) printf("ResumeThread failed with return value %d\n",err);
		CloseHandle(hThread);
	} while(Thread32Next(thSnap, &te));
}
DWORD GetW3TlsForIndex(DWORD index)
{
	DWORD pid = GetCurrentProcessId();
	THREADENTRY32 te32;
	HANDLE hSnap=CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, pid );
	te32.dwSize = sizeof(THREADENTRY32);
	if ( Thread32First( hSnap, &te32 ) )
	{
		do 
		{
			if ( te32.th32OwnerProcessID == pid )
			{
				HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, false, te32.th32ThreadID );
				CONTEXT ctx = { CONTEXT_SEGMENTS };
				LDT_ENTRY ldt;
				GetThreadContext( hThread, &ctx );
				GetThreadSelectorEntry( hThread, ctx.SegFs, &ldt );
				DWORD dwThreadBase = ldt.BaseLow|(ldt.HighWord.Bytes.BaseMid<<16)|(ldt.HighWord.Bytes.BaseHi<<24);
				CloseHandle( hThread );
				if ( dwThreadBase == NULL )
					continue;
				DWORD *dwTLS = *(DWORD**)(dwThreadBase+0xE10+4*index);
				printf("Thread: %X , TLS for index %X : %X\n",te32.th32ThreadID,index,(DWORD)dwTLS);
				if ( dwTLS == NULL )
					continue;
				return (DWORD)dwTLS;

			}
		} while( Thread32Next( hSnap, &te32 ) );
	}
	return NULL;
}
BOOL SetHardwareBPs(DWORD *  Offsets,BYTE * Types,BYTE * Lengths,int Count)
{
	printf("SETBPS %X\n",*Offsets);
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);
	Thread32First(thSnap, &te);
	do
	{
		if(thSnap==INVALID_HANDLE_VALUE) continue;
		if(te.th32ThreadID==GetCurrentThreadId()
			||te.th32ThreadID==threadId1
			||te.th32ThreadID==threadId2
			||te.th32ThreadID==threadId3) continue;
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
		CONTEXT ct;
		ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if(!GetThreadContext(hThread, &ct))
		{
			CloseHandle(hThread);
			continue;
		}
		ct.Dr7=DR7_DEFAULT;
		for(int i=0;i<Count;i++)
		{
			if(Offsets[i]==0)
				continue;
			SetBreakpointBits(ct.Dr7,Types[i],Lengths[i],i);
			ct.Dr0 = (i==0)*Offsets[i]+(i!=0)*ct.Dr0;
			ct.Dr1 = (i==1)*Offsets[i]+(i!=1)*ct.Dr1;
			ct.Dr2 = (i==2)*Offsets[i]+(i!=2)*ct.Dr2;
			ct.Dr3 = (i==3)*Offsets[i]+(i!=3)*ct.Dr3;
		}
		ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		DWORD err=SuspendThread(hThread);
		if(!SetThreadContext(hThread, &ct))
		{
			ResumeThread(hThread);
			CloseHandle(hThread);
			continue;
		}
		err=ResumeThread(hThread);
		CloseHandle(hThread);		
	} while(Thread32Next(thSnap, &te));
	return -1;
}

void ClearHardwareBPs()
{
	printf("CLEARBPS\n");
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);
	Thread32First(thSnap, &te);
	do
	{
		if(thSnap==INVALID_HANDLE_VALUE) continue;
		if(te.th32ThreadID==GetCurrentThreadId()
			||te.th32ThreadID==threadId1
			||te.th32ThreadID==threadId2
			||te.th32ThreadID==threadId3) continue;
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
		CONTEXT ct;
		ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		// get contents of every debug register
		if(!GetThreadContext(hThread, &ct))
		{
			CloseHandle(hThread);
			continue;
		}
		if(!ct.Dr0&&!ct.Dr1&&!ct.Dr2&&!ct.Dr3)
			continue;
		ct.Dr7=DR7_DEFAULT;
		ct.Dr0=0;
		ct.Dr1=0;
		ct.Dr2=0;
		ct.Dr3=0;
		ct.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		DWORD err=SuspendThread(hThread);
		if(!SetThreadContext(hThread, &ct))
		{
			err=ResumeThread(hThread);
			CloseHandle(hThread);
			continue;
		}
		err=ResumeThread(hThread);
		CloseHandle(hThread);
	} while(Thread32Next(thSnap, &te));
}
void SuspendAllThreads()
{
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);
	Thread32First(thSnap, &te);
	do
	{
		if(thSnap==INVALID_HANDLE_VALUE) continue;
		if(te.th32ThreadID==GetCurrentThreadId()
			||te.th32ThreadID==threadId1
			||te.th32ThreadID==threadId2
			||te.th32ThreadID==threadId3) continue;
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
		SuspendThread(hThread);
		CloseHandle(hThread);
	}
	while(Thread32Next(thSnap, &te));

}
void ResumeAllThreads()
{
	HANDLE thSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);
	Thread32First(thSnap, &te);
	do
	{
		if(thSnap==INVALID_HANDLE_VALUE) continue;
		if(te.th32ThreadID==GetCurrentThreadId()
			||te.th32ThreadID==threadId1
			||te.th32ThreadID==threadId2
			||te.th32ThreadID==threadId3) continue;
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
		ResumeThread(hThread);
		CloseHandle(hThread);
	}
	while(Thread32Next(thSnap, &te));
}