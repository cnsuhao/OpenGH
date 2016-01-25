#include "stdafx.h"
#include "Player.h"
#include "Game.h"
#include "Unit.h"
bool IsPlayerEnemy(DWORD pl1,DWORD pl2)
{
	bool ret;
	__asm
	{
		MOV EDI,pl1;
		MOV EAX,pl2;
		MOVZX ESI,DWORD PTR DS:[EAX+0x30];
		MOV ECX,EDI;
		CALL _6F41BF00;
		LEA ECX, DWORD PTR DS:[EAX+0x38];
		CALL _6F473C20;
		MOV ECX,ESI;
		MOV EDX,1;
		SHL EDX,CL;
		AND EAX,EDX;
		NEG EAX;
		SBB EAX,EAX;
		ADD EAX,1;
		MOV ret,AL;
	}
	return ret;
}
DWORD GetPlayerByNumber(DWORD plNum)
{
	DWORD rt=0;
	__asm
	{
		ADDR(_W3XPlayerData,ECX)
		PUSH plNum;
		CALL _GetHPlayerByNumber;
		MOV rt,EAX;
	}
	return rt;
}
DWORD GetPlayer()
{
	return GetPlayerByNumber(GetPlayerNumber());
}
DWORD GetPlayerNumber()
{
	DWORD rt;
	__asm 
	{
		ADDR(_W3XPlayerData,EAX)
		TEST EAX,EAX;
		JE err;
		MOVZX EAX,WORD PTR DS:[EAX+0x28];
		MOV rt,EAX;
	}
	return (DWORD)rt;
err:
	return 0;
}
DWORD GetPlayerRace()
{
	DWORD plr = GetPlayer();
	DWORD rt;
	__asm
	{
		MOV EAX,plr;
		MOV EAX,DWORD PTR DS:[EAX+0x260];
		MOV rt,EAX;
	}
	return rt;
}
DWORD GetPlayerColor(DWORD slot)
{
	__asm
	{
		ADDR(_W3XMinimap,EDI)
		MOV ESI,slot
		LEA EAX,DWORD PTR DS:[EDI+ESI*4+0x23C];
		MOV EAX,DWORD PTR DS:[EAX]
	}
}
DWORD GetPlayerColorByNumber(DWORD num)
{
	__asm
	{
		MOV ECX,num;
		CALL _GetPlayerColorByNumber;
		MOV EAX,DWORD PTR DS:[EAX];
	}
}
DWORD GetPlayerCircleColorBySlot(DWORD dwPlayerSlot,Unit* unit)
{
	DWORD ret,plNum=GetPlayerNumber(),unitOwner=unit->dwOwnerSlot;
	if(plNum!=dwPlayerSlot)
	{
		if(IsPlayerEnemy(GetPlayerByNumber(plNum),GetPlayerByNumber(dwPlayerSlot)))
			return GetPlayerColor(dwPlayerSlot);
		else
			return W3RGB(0,0xFF,0,0xFF);
	}
	__asm
	{
		MOV EDX,unitOwner;
		PUSH dwPlayerSlot;
		LEA ECX, ret;
		CALL _GetPlayerColorBySlot;
	}
	return ret;
}
DWORD GetOwningPlayer(Unit * unit)
{
	DWORD rt;
	DWORD num = unit->dwOwnerSlot;
	return GetPlayerByNumber(num);
}