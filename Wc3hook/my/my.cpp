// my.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "Globals.h"
#include "Handlers.h"
#include "HBP.h"
#include "Game.h"
#include "Unit.h"
#include "Player.h"
#include "Window.h"
void Unhook();
void DestroyConsole();
void Hook(HMODULE hModule);
BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved);
bool HookAPI(char *mod,char* func,PROC whereTo,PROC* Trampoline);
bool IsChannelWardenLoaded(DWORD &WardenBase,DWORD &WardenFuncList);
bool IsIngameWardenLoaded(DWORD &WardenBase,DWORD &WardenFuncList);
void RemoveHook(HOOK hk);
void ProcessCommand(char *buf);
void ConsoleThread(void*);
void InitConsole();
DWORD GetIndex();
BOOL InterceptAPI(DWORD dwAddressToIntercept, DWORD dwReplaced,int nops);
LONG WINAPI MyVectoredHandler1( PEXCEPTION_POINTERS ExceptionInfo );
LONG WINAPI MyVectoredHandler2( PEXCEPTION_POINTERS ExceptionInfo );
void FindOffsets(LPVOID add,SIZE_T bounds);
void ByteSearch(void* hModule);
void UnloadSelf(void);
DWORD GetGameStateValue();
DWORD GetReplayStateValue();
static DWORD index;
HANDLE unload;
char ConfigPath[MAX_PATH];
bool MHEnabled;
DWORD CESP;
DWORD HESP;
PVOID exc2;
DWORD InitialGamestate;
DWORD MainThreadId;
float DefaultCameraDist=1650.0;
float CurrentCameraDist=1650.0;
DWORD CurrentGamestate;
HANDLE hOutConsole;
HANDLE hInConsole;
Unit * basebuild;
Config * kMapConfig;
clock_t LastToggle;
WNDCLASSEX WndClsEx;
PatchManager patchmgr;
WardenManager wardenMgr;
BOOL bShouldDrawManaBars;
DWORD threadId1;
DWORD threadId2;
DWORD threadId3;
HMODULE ths;
HANDLE ToggleEvent;
DWORD BaseOffset;
DWORD CLR;
void WaitForUnload(void*);
bool run;
bool wardenFound;
DWORD wardenJMP;
DWORD TlsValue;
DWORD TlsIndex;
Unit* SafeClicked=NULL;
static DWORD wardenOffset=0x1337;
char patch[] = {0x33,0xc0,0x90,0x33,0xc9,0x90};
char deflt[] = {0x8B,0x41,0x14,0x8B,0x49,0x10};
char sbytes[] = {0x85,0xC0,0x74,0x07,0xBF,0xFF,0x60,0x14,0xFF};
char rbytes[] = {0xE8,0xFA,0x92,0xED,0xFF,0x85,0xC7,0x75,0x07};
char wardenbytes[]={0xC1,0xE9,0x02,0x74,0x02,0xF3,0xA5,0xB1,0x03,0x23,0xCA,0x74,0x02};
DWORD OldValue;
DWORD tmp;
FILE *fout;
INT sock;
typedef int (WSAAPI *SendProc)(SOCKET s,const char FAR * buf,int len,int flags);
typedef int (WSAAPI *RecvProc)(SOCKET s,const char FAR * buf,int len,int flags);
typedef BOOL (WINAPI *ReadFileProc)(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);
list<HOOK> hooks;
list<Line> HeroLines;
list<AutoDance> dancers;
map<Unit*,UnitBar*> ManaBars;
map<DWORD,Cooldown*> Cooldowns;
map<DWORD,Duration*> Durations;
const char *GameStates[7] = {
	"Not in game",
	"Game Lobby",
	"Unknown",
	"Loading...",
	"In game",
	"Unknown",
	"Game paused"
};
DWORD GetIndex()
{
	return *(DWORD*)(_W3XTlsIndex);
}
bool GameStateUpdate(DWORD Value)
{
	if(Value!=CurrentGamestate)
	{
		InitialGamestate=CurrentGamestate;
		CurrentGamestate=Value;
		return true;
	}
	return false;
}
void GetTlsValue()
{
	TlsIndex = GetIndex();
	//printf("INDEX: %X\n",TlsIndex);
	LPVOID tls=(LPVOID)GetW3TlsForIndex(TlsIndex);
	//printf("TLS: %X \n",tls);	
	TlsSetValue(TlsIndex,tls);
	TlsValue=(DWORD)tls;
}
/*char sbytes[] = {0x8A,0x83,0x20,0x01,0x00,0x00};
char rbytes[] = {0xC7,0x45,0xE8,0x00,0x00,0x00,0x00,0xC7,0x45,0xd4,0x3c};*/	
void LogPacket(const unsigned char* data, int pktlen,int direction,int sock)
{
	//if((opcode >=181&&opcode<=247)) return;
	unsigned int line = 1;
	unsigned int countpos = 0;
	unsigned short lenght = pktlen;

	fprintf(fout, "{%s} packet,PacketSize = %u on socket %d\r\n",(direction?"Sending":"Receiving"),lenght,sock);
	fprintf(fout, "|------------------------------------------------|----------------|\r\n");
	fprintf(fout, "|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|\r\n");
	fprintf(fout, "|------------------------------------------------|----------------|\r\n");

	if(lenght > 0)
	{
		fprintf(fout, "|");
		for (unsigned int count = 0 ; count < lenght ; count++)
		{
			if (countpos == 16)
			{
				countpos = 0;

				fprintf(fout,"|");

				for (unsigned int a = count-16; a < count;a++)
				{
					if ((data[a] < 32) || (data[a] > 126))
						fprintf(fout,".");
					else
						fprintf(fout,"%c",data[a]);
				}

				fprintf(fout,"|\r\n");

				line++;
				fprintf(fout,"|");
			}
			fprintf(fout,"%02X ",data[count]);
			//FIX TO PARSE PACKETS WITH LENGHT < OR = TO 16 BYTES.
			if (count+1 == lenght && lenght <= 16)
			{
				for (unsigned int b = countpos+1; b < 16;b++)
					fprintf(fout,"   ");

				fprintf(fout,"|");

				for (unsigned int a = 0; a < lenght;a++)
				{
					if ((data[a] < 32) || (data[a] > 126))
						fprintf(fout,".");
					else
						fprintf(fout,"%c",data[a]);
				}

				for (unsigned int c = count; c < 15;c++)
					fprintf(fout," ");

				fprintf(fout,"|\r\n");
			}
			//FIX TO PARSE THE LAST LINE OF THE PACKETS WHEN THE LENGHT IS > 16 AND ITS IN THE LAST LINE.
			if (count+1 == lenght && lenght > 16)
			{
				for (unsigned int b = countpos+1; b < 16;b++)
					fprintf(fout,"   ");

				fprintf(fout,"|");

				unsigned short print = 0;

				for (unsigned int a = line * 16 - 16; a < lenght;a++)
				{
					if ((data[a] < 32) || (data[a] > 126))
						fprintf(fout,".");
					else
						fprintf(fout,"%c",data[a]);

					print++;
				}

				for (unsigned int c = print; c < 16;c++)
					fprintf(fout," ");

				fprintf(fout,"|\r\n");
			}
			countpos++;
		}
	}
	fprintf(fout, "-------------------------------------------------------------------\r\n\r\n");
}
/*int WSAAPI mysend(SOCKET s,const char FAR * buf,int len,int flags)
{
	sock=s;
	int ret = (*orig)(s,buf,len,flags);
	LogPacket((unsigned char*)buf,len,1,(int)s);
	return ret;
}
BOOL WINAPI myReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	BOOL ret = (*orig2)(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
	LogPacket((unsigned char*)lpBuffer,*lpNumberOfBytesRead,0,(int)hFile);
	return ret;
}*/


void AddHook(PROC* from,PROC to,PROC* Trampoline)
{
	HOOK hk;
	hk.from= from;
	hk.to = to;
	hk.Orig = *hk.from;
	*Trampoline =*from;
	*hk.from = *hk.to;
	hooks.push_back(hk);
}
void RemoveHook(HOOK hk)
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(hk.from,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_READWRITE,&mbi.Protect);
	*hk.from=*hk.Orig;
	DWORD dwOldProtect;
	VirtualProtect(mbi.BaseAddress,mbi.RegionSize,mbi.Protect,&dwOldProtect);
}

bool HookAPI(char *mod,char* func,PROC whereTo,PROC* Trampoline)
{
	HMODULE hInstance = GetModuleHandle("Game.dll");
	HMODULE hKernel = GetModuleHandle(mod);
	PROC pfnNew = (PROC)whereTo, //new address will be here
		pfnHookAPIAddr = GetProcAddress(hKernel,func);
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc =
		(PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
		hInstance,
		TRUE,
		IMAGE_DIRECTORY_ENTRY_IMPORT,
		&ulSize
		);
	while (pImportDesc->Name)
	{
		PSTR pszModName = (PSTR)((PBYTE) hInstance + pImportDesc->Name);
		printf("%s!\n",pszModName);
		if (stricmp(pszModName, mod) == 0)
		{
			printf("Found mod!\n");
			break;
		}
		pImportDesc++;
	}
	PIMAGE_THUNK_DATA pThunk =
		(PIMAGE_THUNK_DATA)((PBYTE) hInstance + pImportDesc->FirstThunk);
	while (pThunk->u1.Function)
	{
		PROC* ppfn = (PROC*) &pThunk->u1.Function;
		BOOL bFound = (*ppfn == pfnHookAPIAddr);
		if (bFound)
		{
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery(ppfn,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
			VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_READWRITE,&mbi.Protect);
			AddHook(ppfn,pfnNew,Trampoline);
			DWORD dwOldProtect;
			VirtualProtect(mbi.BaseAddress,mbi.RegionSize,mbi.Protect,&dwOldProtect);
			break;
		}
		pThunk++;
	}
	return true;
}
void UnloadSelf(void)
{
	MODULEINFO info;
	if(GetModuleInformation(GetCurrentProcess(),ths,&info,sizeof(info)))
	{
		FreeLibraryAndExitThread(ths,0);
	}
	else
	{
		DllMain(ths,DLL_PROCESS_DETACH,NULL);
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery((LPVOID)ths,&mbi,sizeof(mbi));
		__asm
		{
				PUSH MEM_RELEASE
				PUSH 0
				PUSH mbi.AllocationBase
				MOV EAX, ExitThread
				PUSH EAX
				MOV EAX, VirtualFree
				JMP EAX
		}
	}
}
void WaitForUnload(void*)
{
	if(WaitForSingleObject(unload, INFINITE) == WAIT_OBJECT_0)
	{
		ResetEvent(unload);
		CloseHandle(unload);
		UnloadSelf();
	}
	return;
}
void EnableBasicMH(bool enable)
{
	TlsSetValue(TlsIndex,(LPVOID)TlsValue);
	MHEnabled = enable;
	if(enable)
	{
		patchmgr.AddPatch(_MHClickableUnits,"\xEB",1);
		patchmgr.AddPatch(_MHRevealMap+1,"\x15\x70",2);
		patchmgr.AddPatch(_MHMinimapReveal+1,"\x8F",1);
		patchmgr.AddPatch(_MHViewItems,"\xBA\x01\x00\x00\x00",5);
		patchmgr.AddPatch(_MHClickableItems,"\xB8\x01\x00\x00\x00\x90",6);
		patchmgr.AddPatch(_ShowEnemySelection,"\x90\x90",2);
		patchmgr.AddPatch(_ShowEnemyResources,"\xEB",1);
		patchmgr.AddPatch(_ShowEnemyQueue,"\xEB",1);
		patchmgr.AddPatch(_ShowEnemySkillsCds,"\x90\x90",2);
		patchmgr.AddPatch(_BuildUnexplored1,"\x33\xED",2);
		patchmgr.AddPatch(_BuildUnexplored2,"\xEB",1);
		patchmgr.AddPatch(_BuildPlacement,"\x40\x90",2);
		patchmgr.AddPatch(_ShowCommands1,"\x40\x90",2);
		patchmgr.AddPatch(_ShowCommands2,"\x40\x90",2);
		patchmgr.AddPatch(_ShowCommands3,"\x40\x90",2);
		patchmgr.AddPatch(_ShowCommands4,"\x40\x90",2);
		patchmgr.AddPatch(_ShowCommands5,"\x40\x90",2);
		patchmgr.AddPatch(_ShowCommands6,"\x40\x90",2);
		patchmgr.AddPatch(_ShowBlight+1,"\x84",1);
		patchmgr.AddPatch(_ShowProjectiles,"\x90\x90",2);
		patchmgr.AddPatch(_ShowFootprints+2,"\xFF",1);
		patchmgr.AddPatch(_ShowModelGFX1+1,"\xFF",1);
		patchmgr.AddPatch(_ShowModelGFX2+1,"\xFF",1);
		patchmgr.AddPatch(_SafeClickItem,"\xEB",1);
		patchmgr.DetourJMP(_BuffDurationsHk,(DWORD)BuffDurationsHook,0);
		patchmgr.DetourJMP(_BuffDurationsDrawHk,(DWORD)BuffDurationsDrawHook,0);
		patchmgr.DetourJMP(_CooldownsHk,(DWORD)CooldownsHook,1);
		patchmgr.DetourJMP(_CooldownsDrawHk,(DWORD)CooldownsDrawHook,1);
		patchmgr.DetourJMP(_CooldownsEraseHk,(DWORD)CooldownsEraseHook,0);
		patchmgr.DetourJMP(_SafeClickHk1,(DWORD)SafeClickHk1,3);
		patchmgr.DetourJMP(_SafeClickHk2,(DWORD)SafeClickHk2,0);
		patchmgr.DetourJMP(_SafeClickHk3,(DWORD)SafeClickHk3,0);
		patchmgr.DetourJMP(_BuildHk,(DWORD)BuildHook,1);
		patchmgr.DetourJMP(_CameraHk,(DWORD)CameraHook,0);
		if(kMapConfig->getValueAs<bool>("EnableManaBars"))
		{
			patchmgr.DetourJMP(_UnitBarDrawHk,(DWORD)ManaBarDraw,0);
			patchmgr.DetourJMP(_UnitBarPositionHk1,(DWORD)ManaBarPosition,2);
			patchmgr.DetourJMP(_UnitBarHk,(DWORD)ManaBarHook,0);
		}
		patchmgr.DetourJMP(_MHViewUnits,(DWORD)ViewUnitsHook,0);
		patchmgr.DetourJMP(_MinimapDrawHook,(DWORD)DrawLineOnMinimap,0);
		patchmgr.DetourJMP(_UnitCmdHook,(DWORD)UnitCommandHook,0);
		patchmgr.DetourJMP(_MHUnitHpBars,(DWORD)HpBarsHook,0);
		patchmgr.DetourJMP(_ColorSelectionHook,(DWORD)CirclesDrawHook,1);
		//ChangeStartTime(15.0);
		printf("\nMaphack is on!");
	}	
	else
	{
		bShouldDrawManaBars=FALSE;
		Sleep(150);
		DoCleanup();
		patchmgr.RemoveAllPatches();
		printf("\nMaphack is off!");
		//ClearHardwareBPs();
	}
	if(CurrentGamestate==GS_INGAME || CurrentGamestate==GS_PAUSEDGAME)
	{
		
		if(enable)
		{
			if(kMapConfig->getValueAs<bool>("EnableManaBars"))
				RefreshUnitbars();
			RefreshBuffsCooldowns();
			bShouldDrawManaBars=TRUE;
			TextOutWc3("kMap \\TFT 1.24e/ by kolkoo has been turned ON!",CHAT_OBSERVER,15);
			DisplayText("|Cff00ff00kMap v1.24e is ON",5.0);
		}
		else
		{
			TextOutWc3("kMap \\TFT 1.24e/ by kolkoo has been turned OFF!",CHAT_OBSERVER,15);
			DisplayText("|Cffff0000kMap v1.24e is OFF",5.0);
		}
	}
	sprintf(m_TrayData.szTip,"kMap v \\1.24e/ by kolkoo is currently [%s] ",MHEnabled?"ON":"OFF");
	Shell_NotifyIcon(NIM_MODIFY,&m_TrayData);
}

void HandleInitialGameState(DWORD init)
{
	switch(init)
	{
		case GS_INGAME:
		case GS_PAUSEDGAME:
		{
			//GetUnitListOffset();
		}break;
	}
}
void HandleGameStateChange(DWORD newstate)
{
	printf("\nGame state has been changed: %s\n",GameStates[newstate]);
	DWORD ChannelWardenBase=0,IngameWardenBase=0,ChannelFuncPtr=0,IngameFuncPtr=0;
	if(IsChannelWardenLoaded(ChannelWardenBase,ChannelFuncPtr))
	{
		//Unhook();
		//Sleep(10);
		//SetEvent(unload);
		printf("Found channel warden at %X, warden func ptr list : %x\n ",ChannelWardenBase,ChannelFuncPtr);
	}
	if(IsIngameWardenLoaded(IngameWardenBase,IngameFuncPtr))
	{
		printf("Found ingame warden at %X, warden func ptr list : %x\n",IngameWardenBase,IngameFuncPtr);
		wardenMgr.ActivateBypass(IngameWardenBase);
	}
	/*else
		wardenMgr.DeactivateBypass();*/
	switch(newstate)
	{
		case GS_NOTHING:
		{
			basebuild=NULL;
			SafeClicked=NULL;
			bShouldDrawManaBars=FALSE;
			DoCleanup(true);
		}break;
		case GS_LOADING:
		{
			char Text[50];
			sprintf(Text,"L O A D I N G : [ kMap v1.24e ] [%s]",MHEnabled?"ON":"OFF");
			ChangeLoadingBarText(Text);
		}break;
		case GS_INGAME:
		case GS_PAUSEDGAME:
		{
			if(MHEnabled&&newstate==GS_INGAME&&InitialGamestate!=GS_PAUSEDGAME)
			{
				bShouldDrawManaBars=TRUE;
				SafeClicked=NULL;
				CurrentCameraDist=DefaultCameraDist;
				TextOutWc3("kMap \\TFT 1.24e/ Loaded. Have fun.",CHAT_OBSERVER,30);
				if(GetReplayStateValue()==0)
				{
					AutoMine(4);
					AutoQueue();
				}
			}
		}break;
	}
	CurrentGamestate= newstate;
}
void MapThread(void*)
{
	while(run)
	{		
		DWORD p = GetGameStateValue();
		if(p==-1) continue;
		if(GameStateUpdate(p))
			HandleGameStateChange(p);
		DWORD WardenBase,WardenFuncList;
		/*if((LOWORD(GetAsyncKeyState(VK_F5))&1))
		{
			EnableBasicMH(!MHEnabled);
			Sleep(1000);
		}*/
		Sleep(10);
	}
}
DWORD GetReplayStateValue()
{
	if(!run) return -1;
	DWORD rt;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(TlsValue+4*0x0D),(LPVOID)&rt,4,NULL))
		return -1;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(rt+0x10),(LPVOID)&rt,4,NULL))
		return -1;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(rt+0x8),(LPVOID)&rt,4,NULL))
		return -1;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(rt+0x614),(LPVOID)&rt,4,NULL))
		return -1;
	return rt;
}
bool IsChannelWardenLoaded(DWORD &WardenBase,DWORD &WardenFuncList)
{
	DWORD DataHolder = TlsValue+0x102CC;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)DataHolder,&WardenFuncList,sizeof(WardenFuncList),NULL))
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(WardenFuncList+0x18),&WardenBase,sizeof(WardenBase),NULL))
		return false;
	if(!WardenBase)
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(WardenFuncList+0x14),&WardenFuncList,sizeof(WardenFuncList),NULL))
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(WardenFuncList),&WardenFuncList,sizeof(WardenFuncList),NULL))
		return false;
	return true;
}
bool IsIngameWardenLoaded(DWORD &WardenBase,DWORD &WardenFuncList)
{
	DWORD DataHolder = _IngameWarden;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)DataHolder,&WardenFuncList,sizeof(WardenFuncList),NULL))
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)WardenFuncList,&DataHolder,sizeof(WardenFuncList),NULL))
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(WardenFuncList+0x18),&WardenBase,sizeof(WardenBase),NULL))
		return false;
	if(!WardenBase)
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(WardenFuncList+0x14),&WardenFuncList,sizeof(WardenFuncList),NULL))
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(WardenFuncList),&WardenFuncList,sizeof(WardenFuncList),NULL))
		return false;
	return true;
}
DWORD GetGameStateValue()
{
	if(!run) return -1;
	DWORD rt;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(TlsValue+4*0x0D),(LPVOID)&rt,4,NULL))
		return -1;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(rt+0x10),(LPVOID)&rt,4,NULL))
		return -1;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(rt+0x8),(LPVOID)&rt,4,NULL))
		return -1;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)(rt+0x278),(LPVOID)&rt,4,NULL))
		return -1;
	/*__asm
	{
		MOV ESI,0x0D;
		MOV EAX,TlsValue;
		MOV EAX, DWORD PTR DS:[EAX+ESI*4];
		MOV EAX,DWORD PTR DS:[EAX+0x10]
		MOV ECX,DWORD PTR DS:[EAX+0x8]
		MOV EAX, DWORD PTR DS:[ECX+0x278];
		MOV rt,EAX;
	}*/
	return rt;
}
void ProcessCommand(char *buf)
{
	char* cmd="on";
	char* cmd1="off";
	char * cmd2="unload";
	char * cmd3="cu";
	if(strcmp(buf,cmd)==0)
	{
		//HandleInitialGameState(GetGameStateValue());
		EnableBasicMH(true);
		/*BYTE lengths[]={LEN_BYTE};
		DWORD offsets[] = {wardenOffset};
		BYTE types[] = {DBG_COND_EXECUTE};
		SetHardwareBPs(offsets,types,lengths,1);*/
		return;
	}
	if(strcmp(buf,cmd1)==0)
	{
		//HandleInitialGameState(GetGameStateValue());
		EnableBasicMH(false);
		//_beginthread(DrawLineOnMinimap,NULL,0);
		return;
	}
	if(strcmp(buf,cmd2)==0)
	{
		Unhook();
		Sleep(10);
		SetEvent(unload);
		return;
	}
	if(strcmp(buf,cmd3)==0)
	{
		TlsSetValue(TlsIndex,(LPVOID)TlsValue);
		//cout<<IsWardenLoaded()<<"\n";
		return;
		Unit * unt1 = GetSelectedUnit();
		UnitBar * bar = GenerateManaBarForUnit(unt1);
		DWORD drwFunc=0x6F37C530,eraseFunc=0x6F37C510;
		__asm
		{
			MOV ECX,bar;
			CALL drwFunc;
		}
		Sleep(5000);
		__asm
		{
			MOV ECX,bar;
			CALL eraseFunc;
		}
		return;
		Unit * base = GetBaseBuilding();
		LocalSelectUnit(base);
		return;
		int len;
		Unit ** workrs= GetWorkersArray(len,WISP,len);
		SelectMultipleUnits(workrs,len);
		delete [] workrs;
		return;
		Unit * unt = GetSelectedUnit();
		float ASDF = GetUnitFacing(unt);
		Location from = GetUnitLocation(unt);
		printf("from: (%f,%f) \n",from.X,from.Y );
		Location to = calcBack(ASDF,from,800.0);
		printf("from: (%f,%f) \n",to.X,to.Y );
		SendMoveAttackCommand(unt,MOVE,to.X,to.Y,NULL);
		printf("FACING %f\n",ASDF);
	}
	
}
void ConsoleThread(void*)
{
	while(run)
	{
		char var[100];
		memset(var,'\0',100);
		printf("Enter command(on,off,unload): ");
		scanf("%s",var);
		ProcessCommand(var);
	}
}
void DestroyConsole()
{
	FreeConsole();
	hInConsole=hOutConsole=NULL;
}
void Unhook()
{
	if(hhk)
		UnhookWindowsHookEx(hhk);
	PostMessage(hDummyWnd,WM_CLOSE,0,0);
	Shell_NotifyIcon(NIM_DELETE,&m_TrayData);
	run=false;
	bShouldDrawManaBars=FALSE;
	Sleep(150);
	DoCleanup();
	patchmgr.RemoveAllPatches();
	ClearHardwareBPs();
	//printf("HBPS\n");
	//printf("Run = false\n");
	//printf("exc1\n");
	/*list<HOOK>::iterator itr=hooks.begin(),itr2=itr;
	while(itr!=hooks.end())
	{
		itr2=itr++;
		RemoveHook(*itr2);
		hooks.erase(itr2);
	}
	printf("HOOKS\n");
	*/
	DestroyConsole();
}
void Hook(HMODULE hModule)
{
	MHEnabled = false;
	ths=hModule;
	run=true;
	OldValue=-1;
	basebuild=NULL;
	InitConsole();
	GetTlsValue();
	InitialGamestate=CurrentGamestate = GetGameStateValue();
	HandleInitialGameState(InitialGamestate);
	printf("Initial GameState: %s\n",GameStates[InitialGamestate]);
	printf("Game.DLL base: %X\n",GameBase);
	//_beginthread(ByteSearch,0,hModule);
	threadId3=_beginthread(ConsoleThread,NULL,0);
	//fout=fopen("Dump.txt","w");
	/*WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);*/
	//HookAPI("Kernel32.dll","ReadFile",(PROC)&myReadFile,(PROC*)&orig2);
	if(run) // todo check wc3 version ;F
	{
		/////////////////////////////////////////////////////////////////
	 ///								 ///
		//SetHardwareBP(rOffset,1,DBG_COND_EXECUTE);				///
		//SetHardwareBP(pOffset,1,DBG_COND_EXECUTE);			   ///				 
		/////////////////////////////////////////////////////////////
		threadId1 = _beginthread(MapThread,NULL,0);
	}
	else
	{
		printf("Something went wrong :F, type unload to exit.\n");
	}
}
void InitConsole()
{

	hOutConsole=NULL;
	hInConsole=NULL;
	if(AllocConsole())
	{
		/*hOutConsole=::GetStdHandle( STD_OUTPUT_HANDLE );
		if(hOutConsole != INVALID_HANDLE_VALUE)
		{
			if(!SetConsoleMode(hOutConsole, ENABLE_PROCESSED_OUTPUT|ENABLE_WRAP_AT_EOL_OUTPUT ))
				hOutConsole=NULL;
		}*/
		SetConsoleTitle("Warcraft III");
		hConsoleWnd=GetConsoleWindow();
		RemoveMenu(GetSystemMenu(hConsoleWnd, FALSE), SC_CLOSE, MF_BYCOMMAND);
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		unload = CreateEvent(NULL,TRUE,FALSE,NULL);
		threadId2 = _beginthread(WaitForUnload,NULL,0);
		CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)MessageThread,(LPVOID)ths,NULL,NULL);
	}
}

BOOL InterceptAPI(DWORD dwAddressToIntercept, DWORD dwReplaced,int nops)
{
	DWORD dwOldProtect;
	BYTE *pbTargetCode = (BYTE *) dwAddressToIntercept;
	BYTE *pbReplaced = (BYTE *) dwReplaced;
	VirtualProtect((void *) dwAddressToIntercept, 5+nops, PAGE_WRITECOPY, &dwOldProtect);
	*pbTargetCode++ = 0xE9;
	*((unsigned int *)(pbTargetCode)) = pbReplaced - (pbTargetCode +4);
	for(int i=0;i<nops;i++) *(pbTargetCode+4+i)=0x90;
	VirtualProtect((void *) dwAddressToIntercept, 5+nops, PAGE_EXECUTE, &dwOldProtect);
	FlushInstructionCache(GetCurrentProcess(), NULL, NULL);
	return TRUE;
}



void FindOffsets(LPVOID add,SIZE_T bounds)
{
	char *buffer= (char*)add;
	char *p=search(buffer,buffer+bounds,wardenbytes,wardenbytes+sizeof wardenbytes);
	if(p!=(buffer+bounds)) 
	{
		wardenOffset=(p-buffer)+(DWORD)add+3;
		wardenJMP=wardenOffset+0xC;
		wardenFound=true;		
	}
}
bool IsReadable(DWORD Status)
{
	if(Status==PAGE_READWRITE||Status==PAGE_READONLY||Status==PAGE_EXECUTE_READ||Status==PAGE_EXECUTE_READWRITE)
		return true;
	return false;
}
void ByteSearch(void* hMod)
{
	HMODULE hModule=(HMODULE)hMod;
	wardenFound=false;
	LPVOID base,end,base1,end1;
	SYSTEM_INFO	si;
	GetSystemInfo(&si);
	LPVOID off;
	MEMORY_BASIC_INFORMATION mbi;
	MODULEINFO gmi;
	MODULEINFO mi;
	GetModuleInformation(GetCurrentProcess(),(HMODULE)GameBase,&gmi,sizeof(MODULEINFO));
	VirtualQuery((LPVOID)hModule,&mbi,sizeof(mbi));
	base = mbi.BaseAddress;
	end = (LPVOID)((DWORD)base+(DWORD)mbi.RegionSize);
	if(GetModuleInformation(GetCurrentProcess(),hModule,&mi,sizeof(MODULEINFO)))
	{
		base = mi.lpBaseOfDll;
		end = (LPVOID)((DWORD)base+(DWORD)mi.SizeOfImage);
	}
	off= si.lpMinimumApplicationAddress;
	base1 = gmi.lpBaseOfDll;
	end1 = (LPVOID)((DWORD)base1+(DWORD)gmi.SizeOfImage);
	while ((off < si.lpMaximumApplicationAddress)&&!(wardenFound))
	{
		VirtualQuery(off,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
		if(!((mbi.BaseAddress>=base&&mbi.BaseAddress<=end)||(mbi.BaseAddress>=base1&&mbi.BaseAddress<=end1))&&(mbi.State==MEM_COMMIT)&&IsReadable(mbi.Protect))
		{
			FindOffsets(mbi.BaseAddress ,mbi.RegionSize);
		}
		off = (LPVOID)((DWORD)mbi.BaseAddress +(DWORD)mbi.RegionSize);
	}
	if(wardenFound)
	{
		printf("\nWarden active at %X\n",wardenOffset);

	}
	else
	{
		printf("\nWarden not found \n");
	}
}
BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			MainThreadId=GetCurrentThreadId();
			GetModuleFileName(hModule,ConfigPath,MAX_PATH);
			char * p = strstr(ConfigPath,"Kbd.dll");
			*p='\0';
			strcat(ConfigPath,"Config.cfg");
			kMapConfig = &Config::Instance(ConfigPath);
			Hook(hModule);
			break;
		}
		case DLL_PROCESS_DETACH:
		{

			Unhook();
			break;
		}
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

