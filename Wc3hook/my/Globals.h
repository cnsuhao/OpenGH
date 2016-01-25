#include <fstream>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include "PatchManager.h"
#include "WardenManager.h"
#include <tlhelp32.h>
#include <algorithm>
#include <Psapi.h>
#include <time.h>
#include <process.h>   
#include <winsock2.h>
#include <DbgHelp.h>
#include <list>
#include "Offsets.h"
#include <shellapi.h>
#include "resource.h"
#include "Config.h"
using namespace std;
using namespace ConfigMan;
#pragma warning(disable:4309)
#pragma warning(disable:4313)
#pragma warning(disable:4996)
#pragma warning(disable:4267)
#pragma warning(disable:4312)
#pragma warning(disable:4244)
#pragma warning(disable:4311)
#pragma warning(disable:4018)
#pragma warning(disable:4405)
#pragma warning(disable:4101)
#pragma warning(disable:4800)
#ifndef GLOBALS_H
#define GLOBALS_H
#define PI 3.14159265
#define Game_Get(OFFSET) (GameBase+OFFSET)
#define W3RGB(R,G,B,A) ((BYTE)A<<24|((BYTE)R<<16|((BYTE)G)<<8|((BYTE)B)))
#define ADDR(X,REG)\
	__asm MOV REG,DWORD PTR DS:[X] \
	__asm MOV REG,DWORD PTR DS:[REG]
#define RACE_HU 1
#define RACE_ORC 2
#define RACE_UD 3
#define RACE_NE 4
#define ACOLYTE 0x7561636F
#define UGM 0x75676F6C
#define WISP 0x65777370
#define GM 0x6E676F6C
#define EGM 0x65676F6C
#define PEON 0x6F70656F
#define PEASANT 0x68706561
#define Necropolis 0x756E706C
#define ToL 0x65746F6C
#define MainHouse 0x6F677265
#define CityHall 0x68746F77
#define FLAG_ILLUSION 0x40000000
#define FLAG_INVIS 0x1000000
#define FLAG_DEAD 0x100
#define STAT_HP 0
#define STAT_MAXHP 1
#define STAT_MANA 2
#define STAT_MAXMANA 3
#define STATE_SHOULDDANCE 0
#define STATE_DANCING 1
#define CHAT_ALL        0
#define CHAT_ALLY       1
#define CHAT_OBSERVER   2
#define CHAT_PRIVATE    4
#define GS_NOTHING 0
#define GS_LOBBY 1
#define GS_LOADING 3
#define GS_INGAME 4
#define GS_PAUSEDGAME 6
struct HOOK;
struct Unit;
struct UnitBar;
struct Group;
struct Location;
struct MmapLoc;
struct Line;
struct AutoDance;
struct CommandPacket;
struct SelectMultiple;
struct UnitListNode;
struct Command;
struct Cooldown;
struct Duration;
enum Cmd;
extern const char * GameStates[7];
extern DWORD threadId1;
extern DWORD threadId2;
extern DWORD threadId3;
extern bool MHEnabled;
extern DWORD InitialGamestate;
extern float DefaultCameraDist;
extern BOOL bShouldDrawManaBars;
extern float CurrentCameraDist;
extern DWORD CurrentGamestate;
extern Unit * basebuild;
extern PatchManager patchmgr;
extern WardenManager wardenMgr;
extern bool run;
extern list<Line> HeroLines;
extern list<AutoDance> dancers;
extern map<DWORD,Cooldown*> Cooldowns;
extern map<DWORD,Duration*> Durations;
extern map<Unit*,UnitBar*> ManaBars;
extern DWORD TlsValue;
extern DWORD TlsIndex;
extern Unit* SafeClicked;
extern char ConfigPath[];
extern Config * kMapConfig;
extern HMODULE ths;
extern HANDLE unload;
extern DWORD MainThreadId;
void EnableBasicMH(bool enable);
void Unhook();
struct Duration
{
	DWORD _0194;
	DWORD _0198;
	long Time;
};
struct Cooldown
{
	DWORD _0194;
	DWORD _0198;
	long Time;
};
struct HOOK
{
	PROC * from;
	PROC to;
	PROC Orig;
};
struct Unit {
	DWORD dwDummy[3];
	DWORD dwID1; 
	DWORD dwID2; 
	BYTE _1[0x1C]; 
	DWORD dwClassId;
	BYTE _2[0x1C];
	DWORD HealthBar;
	DWORD UNK;
	DWORD dwOwnerSlot;
};
struct UnitBar
{
	DWORD _6F952B38;
	DWORD _0;
	Unit* owner;
	DWORD barData;
	long refCount;
};
/*078AEC7C  4C EB 95 6F 02 00 00 00 E4 4D 34 04 C3 39 00 00  LÎïo...‰M4√9..
078AEC8C  2A 72 00 00 10 24 98 6F 01 00 00 00 00 00 00 00  *r..$òo.......
078AEC9C  00 00 00 00 34 EB 95 6F 00 00 00 00 50 01 33 02  ....4Îïo....P3
078AECAC  E8 02 33 02 05 00 00 00 34 01 C1 07 00 00 00 00  Ë3...4¡....
078AECBC  FF FF FF FF 00 00 00 00 FF FF FF FF 00 00 00 00  ˇˇˇˇ....ˇˇˇˇ....
078AECCC  00 00 00 00 30 00 00 00 8A 07 6D 6F 48 EC 8A 07  ....0...ämoHÏä
078AECDC  DC 4A 95 6F 01 00 00 00 00 00 00 00 8D 3B 00 00  ‹Jïo.......ç;..
078AECEC  2B 72 00 00 10 24 98 6F 01 00 00 00 00 00 00 00  +r..$òo.......*/
struct Group
{
	DWORD _No1cares[11];
	DWORD _UnitListPtr1;
	DWORD _UnitListPtr2;
	DWORD _UnitCount; //?
	//rest we dont care about for now
};
struct Location
{
	float X;
	float Y;
	float Z;
};
struct MmapLoc
{
	DWORD X;
	DWORD Y;
};
struct Line
{
	Location to;
	Unit* hero;
	bool active;
	DWORD color;
};
struct AutoDance
{
	Location from;
	Location to;
	Unit * unit;
	DWORD state;
};
struct CommandPacket
{
	DWORD _6F944E08;
	DWORD _00000000;
	DWORD _0A001200;
	DWORD _0000000[2];
	DWORD _00120012;
	DWORD _00120000;
	DWORD _000D0003;
	DWORD _FFFFFFFF[2];
	float X;
	float Y;
	DWORD dwId1;
	DWORD dwId2;
};
struct SelectMultiple
{
	int count1;
	int count2;
	Unit **units;
	int zero;
};
struct UnitListNode
{
	UnitListNode *nPrev;
	UnitListNode *nNext;
	Unit * unit;
};
struct Command
{
	DWORD _6F942254;
	DWORD IDUNCARE[8];
	DWORD CommandId;
	DWORD UNK[8];
	float X;
	DWORD UNK1;
	float Y;
};
enum Cmd
{
	MOVE=0xD0003,
	ATTACK=0xD000F,
	HOLD=0xD0019,
	STOP=0xD0004
};
#endif