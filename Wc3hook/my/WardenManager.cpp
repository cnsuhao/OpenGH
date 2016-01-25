#include "stdafx.h"
#include "WardenManager.h"
#include "HBP.h"
#include "Globals.h"
char wardenArray[32];
FILE *fdump = NULL;
int GetIntersect(DWORD Start1,DWORD End1,DWORD Start2,DWORD End2,DWORD &Start,DWORD &End)
{
	Start= MAX(Start1,Start2);
	End= MIN(End1,End2);
	return (End-Start);
}
void Log(const char* format,...)
{
	if(!fdump)
		return;
	va_list args;
	int     len;
	char    *buffer;
	va_start( args, format );
	len = _vscprintf(format,args)+1;
	buffer = new char[len];
	vsprintf( buffer, format, args );
	va_end( args );
	fprintf(fdump,buffer);
	fprintf(fdump,"\n");
	delete [] buffer;
}
void __fastcall CheckBytes(int count,int where)
{
	count*=sizeof(DWORD);
	ZeroMemory(wardenArray,sizeof(wardenArray));
	Log("Warden read : %d bytes at location %X",count,where);
	printf("Warden read : %d bytes at location %X",count,where);
	Patch info;
	if(patchmgr.GetPatchInRange(where,where+count,info))
	{
		DWORD offset = info.GetOffset();
		DWORD Start,End;
		int c = GetIntersect(where,where+count,offset,offset+info.GetLength(),Start,End);
		int displ=0;
		if(where>offset)
		{
			displ = where-offset;
			memcpy(&wardenArray[0],(void*)(info.GetOriginal()+displ),c);
			if(c<count)
				memcpy(&wardenArray[c],(void*)(where+c),count-c);
		}
		else
		{
			displ = offset-where;
			memcpy(&wardenArray[0],(void*)(where),displ);
			memcpy(&wardenArray[displ],(void*)(info.GetOriginal()),c);
			int cnt = displ+c;
			if(cnt<count)
				memcpy(&wardenArray[cnt],(void*)(where+cnt),count - cnt);
		}

	}
	else
	{
		memcpy(wardenArray,(void*)(where),count);
	}
}

void _declspec(naked) WardenBypass()
{
	__asm 
	{
		JE jmp1;
		PUSHAD;
		MOV EDX,ESI;
		call CheckBytes;
		POPAD;
		PUSHAD;
		LEA ESI,wardenArray;
		REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI];
		POPAD;
jmp1:
		MOV CL,3;
		AND ECX,EDX;
		JE JMP2;
		PUSHAD;
		MOV EDX,ESI;
		call CheckBytes;
		POPAD;
		PUSHAD;
		LEA ESI,wardenArray;
		REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI];
		POPAD;
JMP2:
		JMP  WardenHookJMP;
	}
}

LONG WINAPI MyWardenHandler( PEXCEPTION_POINTERS ExceptionInfo )
{
	if(ExceptionInfo->ExceptionRecord->ExceptionCode!=EXCEPTION_SINGLE_STEP)
	{	
		return EXCEPTION_CONTINUE_SEARCH;
	}
	if(ExceptionInfo->ExceptionRecord->ExceptionAddress==(PVOID)wardenMgr.GetWardenHook()) 
	{
		ExceptionInfo->ContextRecord->Eip=(DWORD)WardenBypass;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

void InitDump()
{
	if(fdump)
		fclose(fdump);
	char name[20];char time[9];
	memset(name,'\0',20);
	memset(time,'\0',9);
	_strtime_s(time,9);
	char *p=strstr(time,":");
	while(p)
	{
		*p='_';
		p=strstr(p,":");
	}
	sprintf(name,"WardenLog_%s.txt",time);
	fdump = fopen(name,"w");
}
void WardenManager::ActivateBypass()
{
	DWORD Base = GetWardenBase();
	if(Base)
	{	
		WardenHook=FindWardenHook(Base);
		if(WardenHook)
		{
			printf("Found warden hook at %X\n",WardenHook);
			InitDump();
			bypassActive=true;
			BYTE lengths[]={LEN_BYTE};
			DWORD offsets[] = {WardenHook};
			BYTE types[] = {DBG_COND_EXECUTE};
			SetHardwareBPs(offsets,types,lengths,1);
		}
		else
		{
			printf("Couldn't find warden hook pattern\n");
		}
	}
}
DWORD WardenManager::FindWardenHook(DWORD aWardenBase)
{
	char *pattern ="\xC1\xE9\x02\x74\x02\xF3\xA5\xB1\x03\x23\xCA\x74\x02\xF3\xA4";
	int numBytes = 15;
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPVOID)aWardenBase,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
	char *buffer= (char*)aWardenBase;
	char * end = buffer + mbi.RegionSize;
	char *p=search(buffer,buffer + mbi.RegionSize,pattern,pattern + numBytes );
	while(p == end)
	{
		printf("%X %X RegionData AllocBase %X,BaseAddr:%X, Region Size %X\n",p,end,mbi.AllocationBase,mbi.BaseAddress,mbi.RegionSize);
		VirtualQuery((LPVOID)(end + 1),&mbi,sizeof(MEMORY_BASIC_INFORMATION));
		end = (char*)mbi.BaseAddress+mbi.RegionSize;
		p = search(p,p + mbi.RegionSize,pattern,pattern + numBytes);

	}
	WardenHook=(p-buffer)+(DWORD)aWardenBase+3;
	WardenHookJMP=WardenHook+0xC;	
	return WardenHook;
}
void WardenManager::ActivateBypass(DWORD aWardenBase)
{
	if(!IsActive())
	{
		SetWardenBase(aWardenBase);
		ActivateBypass();
	}
}
void WardenManager::DeactivateBypass()
{
	if(IsActive())
	{
		if(fdump)
			fclose(fdump);
		fdump=NULL;
		ClearHardwareBPs();
		SetWardenBase(0);
		bypassActive=false;
	}
}
WardenManager::~WardenManager()
{
	if(IsActive())
		DeactivateBypass();
}