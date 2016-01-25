#include "stdafx.h"
#include "Handlers.h"
#include "Game.h"
#include "Player.h"
#include "Unit.h"
void _declspec(naked) BuildHook()
{
	__asm
	{
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	Location loc;
	Unit * builder;
	loc.Z=0.0;
	__asm 
	{
		LEA EAX,loc.X;
		MOV ECX,DWORD PTR DS:[EBP+0x18];
		MOV DWORD PTR DS:[EAX],ECX;
		LEA EAX,loc.Y;
		MOV ECX,DWORD PTR DS:[EBP+0x1C];
		MOV DWORD PTR DS:[EAX],ECX;
		MOV builder,ESI;
	}
	if(IsPlayerEnemy(GetPlayer(),GetPlayerByNumber(builder->dwOwnerSlot)))
	{
		DWORD Color;
		Color = GetPlayerColor(builder->dwOwnerSlot);
		PingMinimap(loc,Color);
	}
	__asm
	{
		POPAD;
		MOV ESP,EBP
		POP EBP;
		MOV EAX,DWORD PTR DS:[EBX];
		JMP _BuildHkJMP;
	}
}

void _declspec(naked) CameraHook()
{
	__asm
	{
		PUSH EBP
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
		TEST EAX,EAX
		JG decreasing
	}
	CurrentCameraDist+=100.00;
	SetCameraField(CurrentCameraDist,0.0,0);
	goto end1;
decreasing:
	if(DefaultCameraDist < CurrentCameraDist)
	{
		CurrentCameraDist-=100.00;
		SetCameraField(CurrentCameraDist,0.0,0);
end1:
		__asm
		{
			POPAD;
			MOV ESP,EBP
			POP EBP
			JMP _CameraHkJMP2
		}
	}
	__asm
	{
		POPAD;
		MOV ESP,EBP
		POP EBP
		MOV EAX,DWORD PTR DS:[ESI+0x20]
		JMP _CameraHkJMP1
	}
}
void _declspec(naked) ViewUnitsHook()
{
	__asm
	{
		PUSH EBP
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	Unit *unt;
	__asm 
	{
		MOV EAX,DWORD PTR DS:[EBP+8]
		MOV unt,EAX
	}
	if(IsValidUnitPtr((DWORD)unt))
	{
		if(!IsUnitOwnedByPlayer(unt))
			IllusionInvisColorChange(unt);
	}
	__asm
	{
		POPAD;
		MOV ESP,EBP
		POP EBP
		MOV EDX,1
		JMP _MHViewUnitsJmp
	}
}

void _declspec(naked) HpBarsHook()
{
	__asm
	{
		PUSH EBP
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	Unit * unt;
	__asm MOV unt,ESI;
	bool fogged;
	bool visible;
	visible = false;
	fogged = false;
	DWORD player;
	player = GetPlayer();
	fogged=IsUnitFoggedToPlayer(player,unt);
	visible = IsUnitVisibleToPlayer(unt,player);
	if(!visible || fogged)
	{
		__asm 
		{
			POPAD;
			MOV ESP,EBP
			POP EBP
			MOV DWORD PTR DS:[EDX],0xFFFFFFFF;
			CALL _ColorBars;
			JMP _MHUnitHpBarsJmp;
		}
	}
	else
	{
		__asm 
		{
			POPAD;
			MOV ESP,EBP;
			POP EBP;
			CALL _ColorBars;
			JMP _MHUnitHpBarsJmp;
		}
	}
}

void _declspec(naked) DrawLineOnMinimap()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	DrawHeroLines();
	//DanceUnits();
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		MOV ECX,DWORD PTR SS:[ESP+0x7C];
		POP EDI;
		JMP _MinimapDrawJMP;
	}
}
void _declspec(naked) UnitCommandHook()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	Unit* unt;
	Command* CommandData;
	__asm 
	{
		MOV EAX,DWORD PTR DS:[EBP+0xC];
		mov unt,EAX;
		MOV EAX,DWORD PTR DS:[EBP+0x10];
		mov CommandData,EAX;
	}
	ProcessGameCmd(unt,CommandData);
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		PUSH _CmdHookConst;
		JMP _UnitCmdHookJMP;
	}
}
void _declspec(naked) CirclesDrawHook()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	DWORD plSlot;
	DWORD addr;
	Unit * unit;
	__asm MOV plSlot,EBX;
	__asm MOV addr,EAX;
	__asm MOV unit,ESI;
	DWORD CLR;
	CLR = GetPlayerCircleColorBySlot(plSlot,unit);
	memcpy((void*)addr,&CLR,sizeof(CLR));
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		MOV ECX,DWORD PTR DS:[EAX];
		MOV DWORD PTR SS:[ESP+0x60],ECX;
		TEST BYTE PTR DS:[ESI+0x60],1;
		JMP _ColorSelectionJMP;
	}

}
void _declspec(naked) SafeClickHk1()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	Unit * toBeSelected;
	__asm 
	{
		TEST EAX,EAX;
		JE itemcheck;
		MOV EAX,DWORD PTR DS:[EAX];
		MOV toBeSelected,EAX;
		JMP unitcheck;
	}
itemcheck:
	__asm MOV toBeSelected,EDI; 
	if(!IsValidItemPtr((DWORD)toBeSelected))
		goto end2;
unitcheck:
	DWORD plr;
	plr = GetPlayer();
	if(IsUnitFoggedToPlayer(plr,toBeSelected)||!IsUnitVisibleToPlayer(toBeSelected,plr))
		goto end1;
	else
		goto end2;
end1:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		XOR EDI,EDI;
		MOV DWORD PTR SS:[ESP+0x60],EDI;
		MOV DWORD PTR SS:[ESP+0x1C],EDI;
		JMP _SafeClickHk1JMP;
	}
end2:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		MOV EDI,DWORD PTR SS:[ESP+0x60];
		MOV DWORD PTR SS:[ESP+0x1C],ECX;
		JMP _SafeClickHk1JMP;
	}

}
void _declspec(naked) SafeClickHk2()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	Unit * toBeSelected;
	__asm 
	{
		MOV toBeSelected,EDI;
		TEST EDI,EDI
		JE end2;
	}
	DWORD plr;
	plr = GetPlayer();
	if(IsUnitFoggedToPlayer(plr,toBeSelected)||!IsUnitVisibleToPlayer(toBeSelected,plr))
		goto end1;
	else goto end2;
end1:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		JMP _SafeClickHk2JMP1;
	}
end2:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		CALL _SafeClickHk2CALL;
		JMP _SafeClickHk2JMP2;
	}

}
void _declspec(naked) SafeClickHk3()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	Unit * toBeSelected;
	BOOL Select;
	__asm 
	{
		MOV EAX,DWORD PTR DS:[EBP+0x54];
		MOV Select,EAX;
	}
	if(Select)
	{
		__asm MOV toBeSelected,ESI;

	}
	else
	{
		__asm 
		{
			MOV EAX,DWORD PTR DS:[EBP+0x84];
			MOV toBeSelected,EAX;
		}
		goto end2;
	}
	if(!IsValidUnitPtr(toBeSelected))
		goto end2;
	DWORD plr;
	plr = GetPlayer();
	if(IsUnitFoggedToPlayer(plr,toBeSelected)||!IsUnitVisibleToPlayer(toBeSelected,plr))
		goto end1;
	else goto end2;
end1:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		JMP _SafeClickHk3JMP1;
	}
end2:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		CMP DWORD PTR SS:[ESP+0x50],0
		JMP _SafeClickHk3JMP2;
	}

}
map<Unit*,UnitBar*>::iterator iter;
void _declspec(naked) ManaBarDraw()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x8;
		PUSHAD;
	}
	Unit * unit;
	UnitBar* bar;
	__asm MOV unit,ECX;
	if(!bShouldDrawManaBars||!IsValidUnitPtr(unit))
		goto end;
	bar=GetManaBarForUnit(unit);
	if(bar==(UnitBar*)-1||!(bar->owner))
		goto end;
	if(bar->refCount>0)
	{
		__asm
		{
			MOV ECX,bar;
			CALL _DrawUnitBar;
		}
	}
end:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		MOV ECX,DWORD PTR DS:[ECX+0x50];
		TEST ECX,ECX;
		JMP _UnitBarDrawHkJMP;
	}

}
void _declspec(naked) ManaBarHook()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0xC;
		PUSHAD;
	}
	BOOL Draw;
	Unit * unit;
	UnitBar* bar;
	__asm 
	{
		MOV EAX,DWORD PTR SS:[EBP+8];
		MOV Draw,EAX;
		MOV unit,ECX;
	}		
	if(!IsValidUnitPtr(unit))
		goto end;
	if(Draw)
	{	
		bar=GenerateManaBarForUnit(unit);
		goto end;
	}
	else
	{
		bar=GetManaBarForUnit(unit);
		if(bar==(UnitBar*)-1||!(bar->owner))
			goto end;
		if(bar->refCount>0)
		{
			__asm
			{
				PUSH 1;
				MOV ECX,bar;
				MOV ESI,ECX;
				CALL _EraseUnitBar;
			}
			bar->refCount--;
		}
	}
end:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		CMP DWORD PTR SS:[ESP+4],0;
		JMP _UnitBarHkJMP;
	}
}

void _declspec(naked) ManaBarPosition()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x14;
		PUSHAD;
		TEST EAX,EAX
		JE end;
	}
	UnitBar* BarPtr;
	Unit *unit;
	float Y;
	__asm 
	{
		MOV BarPtr,ESI;
		MOV unit,EBX;
		MOV EAX,DWORD PTR DS:[EBP+0x34];
		MOV Y,EAX;
	}
	UnitBar* barval;
	barval=GetManaBarForUnit(unit);
	if(barval==(UnitBar*)-1)
		goto end;
	if(barval==BarPtr)
	{
		Y-=0.006f;
		__asm 
		{
			LEA EAX,Y;
			FLD DWORD PTR DS:[EAX];
			FSTP DWORD PTR DS:[EBP+0x34];
		}
	}
end:
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		FLD DWORD PTR SS:[ESP+0x30];
		MOV ECX,DWORD PTR DS:[ESI+0xC];
		JMP _UnitBarPositionHkJMP;
	}
}
void _declspec(naked) CooldownsDrawHook()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	DWORD CommandButton;
	float Cooldownability;
	DWORD  cd;
	Cooldown *numOverlay;
	__asm 
	{
		MOV CommandButton,ESI;
	}
	Cooldownability=GetTimeLeftForButton(CommandButton,cd);
	if(Cooldownability>0.0)
	{	
		if(GetCooldownForButton(CommandButton,numOverlay))
		{
			if(numOverlay->Time==-1.0)
			{
				numOverlay->Time=cd;
				DrawBtnCooldown(numOverlay);
			}
			else
			{
				DWORD diff;
				diff = numOverlay->Time-cd;
				if(diff>=1.0)
				{
					numOverlay->Time=cd;
					DrawBtnCooldown(numOverlay);
				}
			}
		}
	}
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		FLD DWORD PTR SS:[ESP+8];
		FST DWORD PTR DS:[EDI];
		JMP _CooldownsDrawHkJMP;
	}
}
void _declspec(naked) CooldownsHook()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	float CdTime;
	DWORD CommandButton;
	__asm
	{
		MOV CommandButton,ESI;
		MOV EDX,DWORD PTR DS:[EAX];
		LEA EAX,CdTime;
		MOV DWORD PTR DS:[EAX],EDX;
	}
	if(CdTime==0.0)
	{
		RemoveCooldown(CommandButton);
	}
	else if (CdTime>0.0)
	{
		GenerateCooldown(CommandButton);
	}
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		FLD DWORD PTR DS:[0x6FAC52CC];
		JMP _CooldownsHkJMP;
	}
}
void _declspec(naked) CooldownsEraseHook()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	DWORD CommandButton;
	__asm MOV CommandButton,ESI;
	RemoveCooldown(CommandButton);
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		CALL _CooldownCallFunc;
		JMP _CooldownsEraseHkJMP;
	}
}

void _declspec(naked) BuffDurationsHook()
{
	__asm  
	{	
		CALL _BuffDurCallFunc;
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	DWORD BuffIcon;
	float Duration;
	DWORD draw;
	__asm
	{
		MOV BuffIcon,ESI;
		MOV draw,EAX;
	}
	if(draw&&(GetSelectedUnitsCount()==1))
	{
		DWORD cd;
		Duration=GetDurationLeftForBuff(BuffIcon,cd);
		if(Duration==0.0)
		{
			RemoveBuffDuration(BuffIcon);
		}
		else if (Duration>0.0)
		{
			GenerateBuffDuration(BuffIcon);
		}
	}
	else
		RemoveBuffDuration(BuffIcon);
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		JMP _BuffDurationsHkJMP;
	}
}

void _declspec(naked) BuffDurationsDrawHook()
{
	__asm  
	{	
		PUSH EBP;
		MOV EBP,ESP;
		SUB ESP,0x30;
		PUSHAD;
	}
	DWORD BuffIcon;
	float DurationLeft;
	DWORD  dur;
	Duration *numOverlay;
	__asm 
	{
		MOV BuffIcon,ECX;
	}
	if(GeBuffDurationForBuff(BuffIcon,numOverlay)&&(GetSelectedUnitsCount()==1))
	{
		DurationLeft=GetDurationLeftForBuff(BuffIcon,dur);
		if(DurationLeft>0.0)
		{	
			if(numOverlay->Time==-1.0)
			{
				numOverlay->Time=dur;
				DrawBuffDuration(numOverlay);
			}
			else
			{
				DWORD diff;
				diff = numOverlay->Time-dur;
				if(diff>=1.0)
				{
					numOverlay->Time=dur;
					DrawBuffDuration(numOverlay);
				}
			}
		}
	}
	__asm
	{
		POPAD;
		MOV ESP,EBP;
		POP EBP;
		MOV EDX,DWORD PTR DS:[ECX];
		MOV EAX,DWORD PTR DS:[EDX+0x34];
		JMP _BuffDurationsDrawHkJMP;
	}
}