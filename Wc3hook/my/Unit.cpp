#include "stdafx.h"
#include "Unit.h"
#include "Player.h"
#include "Game.h"
float GetUnitFloatStat(Unit * unit,DWORD statNum)
{
	float result=0;
	__asm
	{
		PUSH statNum;
		LEA EAX,result
		PUSH EAX
		MOV ECX,unit
		CALL _GetFloatStat
	}
	return result;
}

float GetUnitFacing(Unit * unt)
{
	float ret1,ret2;
	__asm
	{
		MOV ESI,unt;
		MOV EDX,DWORD PTR DS:[ESI];
		MOV EAX,DWORD PTR DS:[EDX+0xB8]
		MOV ECX,ESI
		CALL EAX
		MOV EDX,DWORD PTR DS:[EAX]
		MOV EDX,DWORD PTR DS:[EDX+0x1C]
		LEA ECX,ret1
		PUSH ECX
		MOV ECX,EAX
		CALL EDX
		PUSH _FacingConst
		MOV EDX,EAX
		LEA ECX,ret2
		CALL _6F6EF520
	}
	return ret2;
}
float getMpPercentage(Unit * unit)
{
	float Cur = GetUnitFloatStat(unit,STAT_MANA);
	float Max = GetUnitFloatStat(unit,STAT_MAXMANA);
	if(Max <= 0.0)
		return -1.0;
	return Cur/Max;
}
float getHpPercentage(Unit * unit)
{
	float ret;
	__asm
	{
		LEA EAX,ret
		PUSH EAX
		MOV ECX,unit
		CALL _GetHpPer
	}
	return ret;
}
void SetSelectedUnit(Unit * unt)
{
	DWORD plr = GetPlayer();
	DWORD rt;
	__asm
	{
		MOV EAX,plr;
		MOV ECX,DWORD PTR DS:[EAX+0x34];
		MOV EAX,unt
		MOV DWORD PTR DS:[ECX+0x1E0],EAX;
	}
}
void SelectUnit(Unit* unit)
{
	__asm
	{
		ADDR(_W3XGlobalClass,EAX)
		MOV ESI, DWORD PTR DS:[EAX+0x24C];
		PUSH 0;
		PUSH 0;
		PUSH 0;
		PUSH unit;
		MOV ECX,ESI;
		CALL _UnitSelect;
	}
}
void LocalSelectUnit(Unit * unit)
{
	DWORD test = 0x6F28E7A0;
	DWORD test2 = 0x6F422110;
	DWORD plr = GetPlayer();
	DWORD addr=0;
	__asm
	{
		MOV EAX,plr;
		MOV ECX,DWORD PTR DS:[EAX+0x34];
		LEA EAX,DWORD PTR DS:[ECX+0x1E8];
		MOV addr,EAX;
		PUSH 0
		PUSH 1
		PUSH 1
		PUSH 1
		PUSH 1
		MOV ECX,unit
		CALL test
		PUSH 1
		PUSH 0
		MOV ESI,unit
		PUSH ESI
		MOV EDX,test2
		MOV ECX,addr;
		CALL EDX;
	}
}
void SelectMultipleUnits( Unit** units,int count)
{
	SelectMultiple select;
	select.count1=select.count2=count;
	select.units=units;
	select.zero = 0;
	__asm
	{
		ADDR(_W3XGlobalClass,EAX)
		MOV ESI, DWORD PTR DS:[EAX+0x24C];
		PUSH 0;
		PUSH 0;
		PUSH 0;
		PUSH 1;
		PUSH 0;
		LEA EAX,select
		PUSH EAX
		MOV ECX,ESI;
		CALL _SelectMultipleUnits;
	}
}
bool _declspec(naked) IsFogged()
{
	__asm
	{
		SUB ESP,8
		PUSH ESI
		MOV ESI,ECX
		LEA ECX,DWORD PTR SS:[ESP+0x10]
		CALL _6F469F40
		LEA EAX,DWORD PTR SS:[ESP+4]
		PUSH EAX                               
		LEA ECX,DWORD PTR SS:[ESP+0xC]             
		PUSH ECX                                 
		LEA EDX,DWORD PTR SS:[ESP+0x18]           
		PUSH EDX                                 
		MOV ECX,ESI                             
		CALL _6F26DB70                     
		MOV EAX,DWORD PTR SS:[ESP+0x1C]
		MOV ECX,DWORD PTR SS:[ESP+4]
		MOV EDX,DWORD PTR SS:[ESP+8]
		PUSH EAX                                
		PUSH ECX                                 
		PUSH EDX                                
		MOV ECX,ESI                             
		CALL _6F00F4E0                      
		XOR ECX,ECX
		CMP EAX,4
		SETNE CL
		POP ESI
		MOV EAX,ECX
		ADD ESP,8
		RETN
	}
}
bool IsUnitFoggedToPlayer(DWORD plr,Unit* unit)
{
	if(!unit) return false;
	Location loc=GetUnitLocation(unit);
	DWORD rt;
	//DWORD IsFogged =0x6F276D00;
	__asm
	{
		MOV EBX,plr
		ADDR(_W3XPlayerData,EDI)
		MOV EDI,DWORD PTR DS:[EDI+0x34];
		MOV ESI,unit
		MOVZX ECX,BYTE PTR DS:[EBX+0x30];
		MOV EDX,1;
		SHL EDX,CL;
		MOV ECX,loc.X;
		PUSH EDX;
		MOV EDX,loc.Y;
		SUB ESP,0x0C;
		MOV EAX,ESP;
		MOV DWORD PTR DS:[EAX],ECX;
		MOV ECX,loc.Z;
		MOV DWORD PTR DS:[EAX+4],EDX;
		MOV DWORD PTR DS:[EAX+8],ECX;
		MOV ECX,EDI;
		CALL IsFogged;
		MOV rt,EAX;
	}
	return rt;
}
bool IsUnitOwnedByPlayer(DWORD plr,Unit* unit)
{
	if(!unit) return false;
	DWORD rt;
	__asm
	{
		MOV ESI,unit;
		MOV EAX,plr;
		PUSH -1;
		MOVZX EDI,BYTE PTR DS:[EAX+0x30];
		MOV EAX,DWORD PTR DS:[ESI];
		MOV EDX,DWORD PTR DS:[EAX+0xEC];
		MOV ECX,ESI;
		CALL EDX;
		XOR ECX,ECX;
		CMP EDI,EAX;
		SETE CL;
		MOV EAX,ECX;
		MOV rt,EAX;
	}
	return rt;
}
bool IsUnitOwnedByPlayer(Unit* unit)
{
	DWORD plr=GetPlayer();
	return IsUnitOwnedByPlayer(plr,unit);
}
bool IsValidUnitPtr(Unit* ptr)
{
	return IsValidUnitPtr((DWORD)ptr);
}
bool IsValidUnitPtr(DWORD ptr)
{
	DWORD test;
	if(!ptr) 
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)ptr,&test,sizeof(test),NULL))
		return false;
	if(test != _UnitClass) 
		return false;
	return true;
}
bool IsValidItemPtr(DWORD ptr)
{
	DWORD test;
	if(!ptr) 
		return false;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)ptr,&test,sizeof(test),NULL))
		return false;
	if(test != _UnitClass) 
		return false;
	return true;
}
bool IsDead(Unit * unt)
{
	__asm
	{
		MOV ECX,unt;
		TEST DWORD PTR DS:[ECX+0x5C],FLAG_DEAD;
		JE end;
	}
	return true;
end:
	return false;
}
bool IsIllusion(Unit * unt)
{
	__asm
	{
		MOV ECX,unt;
		TEST DWORD PTR DS:[ECX+0x5C],FLAG_ILLUSION;
		JE end;
	}
	return true;
end:
	return false;
}
bool IsHero(Unit *unit)
{
	__asm
	{
		MOV EAX,unit
		CALL _IsUnitHero
	}
}
Unit * GetSelectedUnit()
{
	DWORD plr = GetPlayer();
	DWORD rt;
	__asm
	{
		MOV EAX,plr;
		MOV ECX,DWORD PTR DS:[EAX+0x34];
		MOV EAX,DWORD PTR DS:[ECX+0x1E0];
		MOV rt,EAX;
	}
	return (Unit*)rt;
}
DWORD GetSelectedUnitsCount()
{
	DWORD plr = GetPlayer();
	DWORD rt;
	__asm
	{
		MOV EAX,plr;
		MOV ECX,DWORD PTR DS:[EAX+0x34];
		MOV EAX,DWORD PTR DS:[ECX+0x1D4];
		MOV rt,EAX;
	}
	return rt;
}
Location GetUnitLocation(Unit * unit)
{
	Location loc={0,0,0};
	if(!unit) return loc;
	__asm
	{
		PUSHAD;
		MOV EAX,unit;
		MOV EDX, DWORD PTR DS:[EAX];
		LEA ECX,loc;
		PUSH ECX;
		MOV ECX,EAX;
		MOV EAX,DWORD PTR DS:[EDX+0xB8];
		CALL EAX;
		MOV ECX,EAX;
		CALL _GetLocation;
		POPAD;
	}
	return loc;
}
void SendMoveAttackCommand(Unit* unit,DWORD cmdId,float X,float Y,Unit* target)
{
	Unit * unt = GetSelectedUnit();
	if(unit)
		SelectUnit(unit);
	if(!target)
	{
		__asm
		{
			PUSH 0
			PUSH 6
			PUSH 0
			ADDR(_W3XGlobalClass,ECX)
			MOV ECX, DWORD PTR DS:[ECX+0x1B4];
			PUSH Y
			PUSH X
			PUSH 0
			PUSH cmdId
			CALL _MoveAttackCmd
		}
	}
	else
	{
		__asm
		{
			PUSH 0
			PUSH 4
			PUSH target
			PUSH 0
			PUSH cmdId
			ADDR(_W3XGlobalClass,ECX)
			MOV ECX, DWORD PTR DS:[ECX+0x1B4];
			CALL _MoveTargetCmd
		}
	}
	if(unit)
		SelectUnit(unt);
}
bool IsDancing(Unit * unit)
{
	return true;
}
bool IsUnitVisibleToPlayer(Unit* unit,DWORD Player)
{
	if(!unit)
		return false;
	__asm
	{
		MOV ESI,unit;
		MOV EAX,Player;
		MOVZX EAX,BYTE PTR DS:[EAX+0x30];
		MOV EDX,DWORD PTR DS:[ESI];
		PUSH 4;
		PUSH 0;
		PUSH EAX;
		MOV EAX,DWORD PTR DS:[EDX+0xFC];
		MOV ECX,ESI;
		CALL EAX;
	}
}
UnitBar* GenerateManaBar()
{
	UnitBar* manabar=new UnitBar;
	ZeroMemory(manabar,sizeof(UnitBar));
	manabar->_6F952B38=_UnitBarClass;
	manabar->refCount=0;
	//DWORD test=0x6F37A370;
	__asm
	{
		//CALL _MakeUnitbar;
		/*PUSH 0;
		PUSH 0x6A;
		PUSH _BarDataConst;
		PUSH 0x10;
		CALL _MakeBarData;
		MOV ECX,EAX;
		CALL test;
		MOV manabar,EAX;*/
		MOV ECX,manabar;
		CMP DWORD PTR DS:[ECX+0xC],0;
		JNE end;
		PUSH 0;
		PUSH 0x0DD;
		PUSH _BarDataConst;
		PUSH 0x158;
		CALL _MakeBarData;
		MOV ECX,EAX;
		PUSH 0;
		PUSH 0;
		PUSH 0;
		CALL _GenerateBarData;
		MOV ECX,manabar;
		MOV DWORD PTR DS:[ECX+0xC],EAX;
	}
end:
	return manabar;
}
float GetUnitScale(Unit * unit)
{
	float UnitScale;
	__asm
	{
		MOV ECX,unit;
		MOV ECX,DWORD PTR DS:[ECX+0x30];
		CALL _GetScale;
		LEA EAX,UnitScale;
		FSTP DWORD PTR SS:[EAX];
	}
	return UnitScale;
}
void InitBar(UnitBar* &bar,Unit* owner)
{
	DWORD CLR=W3RGB(0,0xFF,0xFF,0xFF);
	float UnitScale=GetUnitScale(owner);
	float SelectionScale;
	DWORD barData=bar->barData;
	if(!barData)
	{
		UnitBar* temp = GenerateManaBar();
		delete bar;
		bar=temp;
		barData=bar->barData;
	}
	__asm
	{
		PUSH 0;
		MOV EDX,_ScaleFactor;
		MOV ECX,_SelectionCircle;
		CALL _GetObjectScale;
		LEA EAX,SelectionScale;
		FSTP DWORD PTR SS:[EAX];
		FLD DWORD PTR SS:[EAX];
		PUSH ECX;
		FMUL UnitScale;
		MOV ECX,barData;
		MOV EAX,_ScaleMultConst;
		FMUL QWORD PTR SS:[EAX];
		FSTP DWORD PTR SS:[ESP];
		CALL _SetBarWidth;
		MOV EAX,_BarHeightConst;
		FLD DWORD PTR DS:[EAX];
		PUSH ECX;
		MOV ECX,barData;
		FSTP DWORD PTR SS:[ESP];
		CALL _SetBarHeight;
		MOV EAX,_BarConst;
		FLD DWORD PTR DS:[EAX];
		MOV ECX,barData;
		PUSH 1;
		SUB ESP,8;
		FST DWORD PTR SS:[ESP+4];
		FSTP DWORD PTR SS:[ESP];
		PUSH 1;
		CALL _SetBarPosition;
		/*MOV ECX,barData;
		MOV EAX,DWORD PTR DS:[ECX+0x84];
		ADD EAX,1;
		PUSH EAX;
		CALL _UnkBarInit;
		MOV ECX,barData;
		CALL _UnkBarInit2;*/ 
		MOV ECX,barData;
		MOV ECX,DWORD PTR DS:[ECX+0x134];
		LEA EDX,CLR;
		PUSH EDX;
		CALL _ColorBars;
	}
	bar->owner=owner;
}
UnitBar* GenerateBar(Unit * unt)
{
	float mpPerc = getMpPercentage(unt);
	if(mpPerc<0.0)
		return (UnitBar*)-1;
	UnitBar* manabar = GenerateManaBar();
	InitBar(manabar,unt);
	manabar->owner=unt;
	return manabar;
}
UnitBar* GetManaBarForUnit(Unit * unit)
{
	UnitBar* manabar;
	map<Unit*,UnitBar*>::iterator iter = ManaBars.find(unit);
	if(iter!=ManaBars.end())
	{
		//UpdateBar(iter->second,unit);
		return iter->second;
	}
	else 
		return (UnitBar*)-1;
}
UnitBar* UpdateBar(UnitBar* &bar,Unit * unit)
{
	float UnitScale=GetUnitScale(unit);
	float SelectionScale;
	DWORD barData=bar->barData;
	__asm
	{
		PUSH 0;
		MOV EDX,_ScaleFactor;
		MOV ECX,_SelectionCircle;
		CALL _GetObjectScale;
		LEA EAX,SelectionScale;
		FSTP DWORD PTR SS:[EAX];
		FLD DWORD PTR SS:[EAX];
		PUSH ECX;
		FMUL UnitScale;
		MOV ECX,barData;
		MOV EAX,_ScaleMultConst;
		FMUL QWORD PTR SS:[EAX];
		FSTP DWORD PTR SS:[ESP];
		CALL _SetBarWidth;
		/*MOV ECX,barData;
		MOV EDX,DWORD PTR DS:[ECX];
		MOV EAX,unit;
		MOV EDX,DWORD PTR DS:[EDX+0x74]
		PUSH EAX;
		CALL EDX;
		/*MOV ECX,barData;
		CALL _UnkUpdateBar;*/
	}
	bar->owner=unit;
}
UnitBar* GenerateManaBarForUnit(Unit * unit)
{
	UnitBar* manabar;
	map<Unit*,UnitBar*>::iterator iter = ManaBars.find(unit);
	if(iter!=ManaBars.end())
	{
		manabar=iter->second;
		//InitBar(manabar,unit);
		UpdateBar(manabar,unit);
	}
	else
	{
		manabar = GenerateBar(unit);
		if(manabar!=(UnitBar*)-1)
		{
			ManaBars[unit]=manabar;
		}
	}
	if(manabar!=(UnitBar*)-1&&(manabar->owner))
	{
		manabar->refCount++;
	}
	/*if((manabar!=(UnitBar*)-1)&&!manabar->owner)
	{
		manabar->owner=unit;
	}/*/
	//if(!FillManaValues(manabar,unit))
	//	return -1;
	return manabar;
}
Location GetUnitOnscreenLoc(Unit * unit)
{
	float UNK;
	Location ret;
	DWORD fn=0x6F334C00;
	__asm
	{
beg:
		LEA ECX,UNK;
		PUSH ECX;
		LEA EDX,ret;
		MOV ECX,unit;
		CALL fn;
		TEST EAX,EAX
		JE beg;
	}
	return ret;
}
