#pragma once
#ifndef WARDENMGR_H
#define WARDENMGR_H
enum WardenTypes
{
	WARDEN_CHAT=0,
	WARDEN_INGAME
};

extern FILE * fdump;
void __fastcall CheckBytes(int count,int where);
void WardenBypass();
LONG WINAPI MyWardenHandler( PEXCEPTION_POINTERS ExceptionInfo );
void InitDump();
void Log(const char* format,...);
static DWORD WardenHook;
static DWORD WardenHookJMP;
class WardenManager
{
public:
	WardenManager():bypassActive(false),wardenBase(0)
	{
		exc1=AddVectoredExceptionHandler(0x1337,MyWardenHandler);
	}
	WardenManager(DWORD aWardenBase):bypassActive(false),wardenBase(aWardenBase)
	{
		exc1=AddVectoredExceptionHandler(0x1337,MyWardenHandler);
	}
	~WardenManager();
	DWORD FindWardenHook(DWORD aWardenBase);
	void SetWardenBase(DWORD aWardenBase)
	{
		wardenBase=aWardenBase;
	}
	DWORD GetWardenBase() const
	{
		return wardenBase;
	}
	DWORD GetWardenHook() const
	{
		return WardenHook;
	}
	bool IsActive() const
	{
		return bypassActive;
	}
	void ActivateBypass();
	void ActivateBypass(DWORD aWardenBase);
	void DeactivateBypass();
private:
	bool bypassActive;
	DWORD wardenBase;
	PVOID exc1;
};



#endif