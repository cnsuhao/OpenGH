#include "stdafx.h"
#include "Game.h"
#include "Player.h"
#include "Unit.h"
DWORD GetBaseClassIdByRace(DWORD race)
{
	return (RACE_HU==race)*CityHall + (RACE_ORC==race)*MainHouse+(RACE_UD==race)*Necropolis+(RACE_NE==race)*ToL;
}
void ConvertMmap(MmapLoc& loc)
{
	DWORD X=loc.X,Y=loc.Y;
	__asm
	{
		MOV EAX,X
		SAR EAX,2
		MOV X,EAX
		MOV EAX,Y
		SAR EAX,2
		MOV Y,EAX
	}
	DWORD Base;
	float xMult,yMult;
	double unk;	
	DWORD xOffset,yOffset;
	DWORD unk2,unk3;
	memcpy(&Base,(void*)_W3XMinimap,sizeof(Base));
	memcpy(&xOffset,(void*)(Base+0x228),sizeof(xOffset));
	memcpy(&yOffset,(void*)(Base+0x22C),sizeof(yOffset));
	memcpy(&xMult,(void*)(Base+0x21C),sizeof(xMult));
	memcpy(&yMult,(void*)(Base+0x220),sizeof(yMult));
	memcpy(&unk,(void*)(_MmapOffset),sizeof(unk));
	memcpy(&unk2,(void*)(Base+0x214),sizeof(unk2));
	memcpy(&unk3,(void*)(Base+0x218),sizeof(unk3));
	X-=xOffset;
	Y-=yOffset;
	Float fX,fY;
	fX.X= (float)(X*xMult+unk);
	fY.X = (float)(Y*yMult+unk);
	__asm
	{
		MOV EAX,fX.dX
		SHR EAX,0xE
		MOVZX EAX,AL
		MOV X,EAX
		MOV EAX,fY.dX
		SHR EAX,0xE
		MOVZX EAX,AL
		MOV Y,EAX
	};
	X+=unk2;
	//X-=6;
	Y=0x100 - Y - unk3 - 2;
	loc.X=X;
	loc.Y=Y;
}
MmapLoc LocationToMinimap(Location loc)
{
	MmapLoc ret;
	ret.X=CoordToMinimap(loc.X,0x6C);
	ret.Y=CoordToMinimap(loc.Y,0x70);
	ConvertMmap(ret);
	return ret;
}
DWORD CoordToMinimap(float Loc,DWORD offst)
{
	//Loc = fabs(Loc);
	float Result;
	DWORD ret;
	__asm
	{
		ADDR(_W3XConversion,EAX)
		//PUSH Loc
		add EAX,offst
		PUSH EAX
		LEA EDX,Loc
		LEA ECX,Result
		CALL _ConvertCoord1
		MOV ECX,DWORD PTR DS:[EAX]
		LEA EAX,DWORD PTR DS:[ECX+0xFD000000]
		XOR EAX,ECX
		SAR EAX,0x1F
		LEA EDX,DWORD PTR DS:[ECX+0xFD800000]
		NOT EAX
		AND EAX,EDX
		LEA ECX,Result
		MOV DWORD PTR DS:[ECX],EAX
		CALL _ConvertCoord2
		MOV ret,EAX
	}
	return ret;
}
float Distance(Location one, Location two)
{
	float A  = (one.X-two.X);
	float B  = (one.Y-two.Y);
	float C  = (one.Z-two.Z);
	return sqrtf(A*A+B*B+C*C);
}
DWORD Distance(MmapLoc one,MmapLoc two)
{
	float A  = ((float)one.X-(float)two.X);
	float B  = ((float)one.Y-(float)two.Y);
	return floor(sqrtf(A*A+B*B));
}
Location calcBack(float facing, Location from,float Distance)
{
	float Yvar = sin(facing*PI/180.0)*Distance;
	float Xvar = cos(facing*PI/180.0)*Distance;
	Location loc=from;
	loc.X-=Xvar;
	loc.Y-=Yvar;
	return loc;
}
Unit* GetBaseBuilding()
{
	if(basebuild)
		return basebuild;

	Unit * unt = FindPlayerBase();
	if(IsValidUnitPtr(unt))
		basebuild=unt;
	//printf("Base BUILDING: %X\n",basebuild);
	return basebuild;
}
Unit* FindWorkerAt(DWORD &ptr,DWORD wId)
{
	Unit* Worker = NULL;
	DWORD pt = *(DWORD*)(ptr);
	//printf("PT :%X,\n",pt);
	if(!IsValidUnitPtr(pt)) 
		return NULL;
	Worker = (Unit*)(pt);
	if(!IsUnitOwnedByPlayer(Worker)) 
		return NULL;
	//printf("WCLSID %X , WID %X\n",Worker->dwClassId,wId);
	if(Worker->dwClassId!=wId) 
		return NULL;
	//printf("Worker offset%d:%X \n",i,Worker);
	return Worker;
}
Unit* FindPlayerBase()
{
	DWORD num  = GetPlayerNumber();
	//DWORD testy = 0x6F03F180;
	DWORD Rt;
	__asm SUB ESP,0x30;
	__asm
	{
		PUSH 0x1F;
		PUSH 3;
		PUSH num;
		PUSH 0x18;
		LEA EAX,DWORD PTR SS:[ESP+0x18];
		PUSH EAX;
		PUSH _BaseConst;
		CALL _BaseGet;
		MOV EAX,DWORD PTR DS:[ESP+0x20];
		MOV Rt,EAX;
		ADD ESP,0x18 //turns out baseget is vararg ( motherfuckers ) 
	}
	return (Unit*)Rt;
}
Unit ** GetWorkersArray(int &len,DWORD wId,int count)
{
	Unit *Worker;DWORD unitcount=0;
	Unit ** workers = new Unit*[count];
	DWORD ptr =(GetUnitArray(unitcount)-4);
	int j=0;
	for(int i=0;i<unitcount&&j<count;i++)
	{
		ptr+=4;
		Worker=FindWorkerAt(ptr,wId);
		if(Worker)
		{
			workers[j]=Worker;
			//SendUnitToMine(Worker,mine);
			j++;
		}
	}
	len=j;
	return workers;
}
void PingMinimap(Location loc, DWORD Color, DWORD PingType,float StayUpTime)
{
	__asm
	{
		PUSH loc.X;
		PUSH loc.Y;
		PUSH loc.Z;
		PUSH Color;
		PUSH PingType;
		LEA EAX,Color;
		PUSH EAX;
		SUB ESP,0x10;
		MOV EAX,StayUpTime;
		MOV DWORD PTR DS:[ESP+0xC],EAX;
		MOV EAX,loc.Y;
		MOV DWORD PTR DS:[ESP],EAX;
		MOV EAX,loc.X;
		MOV DWORD PTR DS:[ESP+4],EAX;
		MOV EAX,loc.Z;
		MOV DWORD PTR DS:[ESP+8],EAX;
		XOR EDX,EDX;
		ADDR(_W3XGlobalClass,ECX)
		MOV ECX,DWORD PTR DS:[ECX+0x3C0];
		CALL _PingMinimapEx;
		add ESP,0x10;
	}
}
void ProcessGameCmd(Unit* unit,Command* CommandData)
{
	switch(CommandData->CommandId)
	{
	case MOVE:
	case ATTACK:
		{
			Location loc;
			loc.X=CommandData->X;
			loc.Y=CommandData->Y;
			if(IsValidUnitPtr(unit)&&!IsIllusion(unit)&&IsHero(unit))
			{
				AddHeroLine(unit,loc);
			}
		}break;
	case STOP:
	case HOLD:
		{
			if(IsValidUnitPtr(unit)&&IsHero(unit))
			{
				RemoveHeroLine(unit);
			}
		}break;
	}
}
void SendUnitToMine(Unit* unit,Unit* mine)
{
	ExecuteCmd(unit,GetUnitLocation(mine),MOVE,mine);
	/*DWORD shit = 0x6F2CDF40;
	Location loc = GetUnitLocation(mine);
	CommandPacket *pkt=new CommandPacket;
	memset(pkt,0,sizeof(CommandPacket));
	pkt->_6F944E08=0x6F944E08;
	pkt->_0A001200=0x000A0012;
	pkt->_00120000=0x00120000;
	BYTE NUM =(BYTE)GetPlayerNumber();
	pkt->_00120012=0x00120012|(NUM<<8);
	memset(pkt->_FFFFFFFF,0xFF,sizeof(pkt->_FFFFFFFF));
	pkt->_000D0003=0x000D0003;
	pkt->dwId1=mine->dwID1;
	pkt->dwId2=mine->dwID2;
	pkt->X=loc.X;
	pkt->Y=loc.Y;
	__asm
	{
	XOR EDX,EDX;
	MOV ECX,pkt;
	CALL shit;
	}*/
}
void ExecuteCmd(Unit* unit,Location target,DWORD CmdId,Unit * tarunit)
{
	DWORD plNum=GetPlayerNumber();
	__asm
	{
		PUSH 0
		PUSH 0
		PUSH 0
		LEA EAX,target.X
		PUSH EAX
		LEA EAX,target.Y
		PUSH EAX
		MOV ECX,CmdId
		MOV EDX,tarunit
		PUSH EDX
		MOV EDX,plNum
		PUSH 0
		CALL _GenerateCmd
		MOV ECX,unit
		PUSH EAX
		PUSH ECX
		MOV CX,AX
		Call _ExecCmd
	}
}
void HarvestCommand()
{
	DWORD ptrr=0x6F37B520;
	__asm 
	{
		PUSH 0;
		PUSH 0;
		PUSH 0x00008004;
		PUSH 0x000D0032;
		PUSH 0x41776861;
		ADDR(_W3XGlobalClass,EAX)
		MOV ECX, DWORD PTR DS:[EAX+0x1B4];
		call ptrr;
	}
}
Unit* GetMine(Unit* base)
{
	Location bLoc = GetUnitLocation(base);
	Location tmp;
	DWORD gId = GetGoldMineClassIDbyBase(base);
	DWORD unitcount;
	DWORD ptr = GetUnitArray(unitcount)-4;
	for(int i=0;i<unitcount;i++)
	{
		ptr+=4;
		DWORD pt = *(DWORD*)(ptr);
		if(!IsValidUnitPtr(pt))continue;
		Unit *mine = (Unit*)(pt);
		if(mine->dwClassId==gId)
		{
			tmp = GetUnitLocation(mine);
			DWORD dst = (DWORD)Distance(bLoc,tmp);
			if(dst<=850)
				return mine;
			else continue;			
		}
	}	
	return NULL;
}
void AutoMine(DWORD num)
{
	TlsSetValue(TlsIndex,(LPVOID)TlsValue);
	Unit *base = GetBaseBuilding();
	if(!base)
		return;
	DWORD wId= GetWorkerClassIDbyBase(base);
	Unit * mine = GetMine(base);
	DWORD nm=num;
	if(wId == ACOLYTE) nm=3;
	//Unit * mine = GetMine(base);
	//if(!mine) return;
	//printf("Mine offset:%X /Slot: %X ?= %X\n",mine,mine->dwOwnerSlot,base->dwOwnerSlot);
	int len=0;
	Unit** Workers=GetWorkersArray(len,wId,nm);
	SelectMultipleUnits(Workers,nm);
	SendMoveAttackCommand(NULL,MOVE,0,0,mine);
	delete [] Workers;
}
void TextOutWc3(char * text,DWORD msgtype,float StayUpTime)
{
	DWORD plNum = GetPlayerNumber();
	__asm
	{
		ADDR(_W3XGlobalClass,EAX)
		PUSH StayUpTime;
		PUSH msgtype;
		PUSH text;
		PUSH plNum;
		MOV ECX,EAX;
		CALL _TextToScreen;
		/*PUSH msgtype;
		PUSH text;
		PUSH plNum;
		ADDR(_W3XGlobalClass,EAX)
		MOV EAX, DWORD PTR SS:[EAX+0x23C];
		MOV EAX, DWORD PTR DS:[EAX+0xC];
		MOV ECX,EAX;
		CALL _TextToChatLog;*/
	}
}
void SetCameraField(float val,float seconds,DWORD field)
{
	__asm
	{
		ADDR(_W3XGlobalClass,EAX)
			MOV ECX,DWORD PTR DS:[EAX+0x254]
		PUSH 1
			PUSH seconds
			PUSH val
			PUSH field
			CALL _SetCameraField
	}
}
void BuildWorker(DWORD Classid)
{
	__asm
	{
		ADDR(_W3XGlobalClass,EAX)
			MOV ECX, DWORD PTR DS:[EAX+0x1B4];
		PUSH 1;
		PUSH 0;
		PUSH 0x86000001;
		PUSH Classid;
		PUSH 0x41717565;
		CALL _QueueUnitByClassId;
	}
}
void AutoQueue()
{
	TlsSetValue(TlsIndex,(LPVOID)TlsValue);
	Unit* base = GetBaseBuilding();
	if(!IsValidUnitPtr(base))
		return;
	SelectUnit(base);
	BuildWorker(GetWorkerClassIDbyBase(base));
	BuildWorker(GetWorkerClassIDbyBase(base));
}
void DrawUnitOnMap(Unit *unit)
{

	__asm
	{
		ADDR(_W3XGlobalClass,EDI);
		MOV EDI,DWORD PTR DS:[EDI+0x3BC];
		MOV EAX, DWORD PTR DS:[EDI+0x140];
		FLD DWORD PTR DS:[EDI+0x238];
		MOV ESI,unit
		PUSH ESI
		PUSH 0
		PUSH EAX
		PUSH -1
		FSTP DWORD PTR SS:[ESP]
		MOV ECX,DWORD PTR DS:[ESI+0x28]
		PUSH ECX
		MOV ECX, DWORD PTR DS:[EDI + 0x16C]
		CALL _DrawUnit
	}

}

void  IllusionInvisColorChange(Unit * unt)
{
	__asm
	{
		MOV ECX,unt;
		TEST DWORD PTR DS:[ECX+0x5C],FLAG_ILLUSION;
		JE off1
		PUSH ECX;
		MOV EAX,ESP;
		MOV DWORD PTR DS:[EAX],0xFF1460FF;
		MOV ECX,DWORD PTR DS:[ECX+0x28];
		XOR EDX,EDX;
		CALL _ColorChange;
		JMP off;
	}
off1:
	__asm
	{
		TEST DWORD PTR DS:[ECX+0x5C],FLAG_INVIS;
		JE off
	}
	DrawUnitOnMap(unt);
	__asm
	{
		MOV ECX,unt;
		PUSH ECX;
		MOV EAX,ESP;
		MOV DWORD PTR DS:[EAX],0xFFFF4500;
		MOV ECX,DWORD PTR DS:[ECX+0x28];
		XOR EDX,EDX;
		CALL _ColorChange;
	}
off:
	return;
}
DWORD GetWorkerClassIDbyBase(Unit * base)
{
	switch(base->dwClassId)
	{
		case Necropolis: return ACOLYTE;
		case ToL: return WISP;
		case MainHouse: return PEON;
		case CityHall: return PEASANT;
	}
	return 0;
}
DWORD GetGoldMineClassIDbyBase(Unit *base)
{
	switch(base->dwClassId)
	{
	case Necropolis: return UGM;
	case ToL: return EGM;
	default: return GM;
	}
	return 0;
}
DWORD GetUnitArray(DWORD &Sz)
{
	__asm
	{
		ADDR(_W3XGlobalClass,EAX)
		MOV EAX,DWORD PTR DS:[EAX+0x3BC]
		PUSH 0 // if 0 here it will just return the pointer if 1 it will update the array
		MOV ECX,EAX
		CALL _GetUnitArrayPtr
		MOV ECX,DWORD PTR DS:[EAX+4]
		MOV EDX,DWORD PTR DS:[Sz]
		MOV DWORD PTR DS:[EDX], ECX
		MOV EAX,DWORD PTR DS:[EAX+8]
	}
}
void DrawPixel(DWORD X,DWORD Y,DWORD Color)
{
	__asm
	{
		ADDR(_W3XMinimap,EDI);
		MOV EDI,DWORD PTR DS:[EDI+0x1D8];
		MOV EAX,Y
		SHL EAX,8
		MOV ECX,X
		ADD EAX,ECX
		MOV ECX,Color
		MOV DWORD PTR DS:[EDI+4*EAX],ECX;
	}
}
void DrawLine(MmapLoc From, MmapLoc To,DWORD Color)
{
	int x2=To.X,y2=To.Y,x1=From.X,y1=From.Y,x,y,xinc1,xinc2,yinc1,yinc2,den,num,numadd,numpixels;
	int deltax = abs(x2 - x1);        // The difference between the x's
	int deltay = abs(y2 - y1);        // The difference between the y's
	x = x1;                       // Start x off at the first pixel
	y = y1;                       // Start y off at the first pixel
	if (x2 >= x1)                 // The x-values are increasing
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else                          // The x-values are decreasing
	{
		xinc1 = -1;
		xinc2 = -1;
	}
	if (y2 >= y1)                 // The y-values are increasing
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else                          // The y-values are decreasing
	{
		yinc1 = -1;
		yinc2 = -1;
	}
	if (deltax >= deltay)         // There is at least one x-value for every y-value
	{
		xinc1 = 0;                  // Don't change the x when numerator >= denominator
		yinc2 = 0;                  // Don't change the y for every iteration
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         // There are more x-values than y-values
	}
	else                          // There is at least one y-value for every x-value
	{
		xinc2 = 0;                  // Don't change the x for every iteration
		yinc1 = 0;                  // Don't change the y when numerator >= denominator
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;         // There are more y-values than x-values
	}

	for (int curpixel = 0; curpixel <= numpixels; curpixel++)
	{
		DrawPixel(x,y,Color);
		num += numadd;              // Increase the numerator by the top of the fraction
		if (num >= den)             // Check if numerator >= denominator
		{
			num -= den;               // Calculate the new numerator value
			x += xinc1;               // Change the x as appropriate
			y += yinc1;               // Change the y as appropriate
		}
		x += xinc2;                 // Change the x as appropriate
		y += yinc2;                 // Change the y as appropriate
	}
}
void DrawLine(Location from,Location to,DWORD Color)
{
	MmapLoc From = LocationToMinimap(from);
	MmapLoc To = LocationToMinimap(to);
	DrawLine(From,To,Color);
	From.X+=1;To.X+=1;
	DrawLine(From,To,Color);
	From.X-=1;To.X-=1;
	From.Y-=1;To.Y-=1;
	DrawLine(From,To,Color);
}
void DrawHeroLines()
{
	list<Line>::iterator iter = HeroLines.begin(),iter2=iter;
	while(iter!=HeroLines.end())
	{
		iter2=iter++;
		if(!IsValidUnitPtr(iter2->hero)||IsDead(iter2->hero))
		{
			HeroLines.erase(iter2);
			continue;
		}
		Location from = GetUnitLocation(iter2->hero);
		if(Distance(from,iter2->to)< 500.0)
		{
			iter2->active=false;
		}
		if(iter2->active)
			DrawLine(from,iter2->to,iter2->color);
		else
			HeroLines.erase(iter2);
	}
}
void RemoveHeroLine(Unit* hero)
{
	list<Line>::iterator iter = HeroLines.begin(),iter2=iter;
	while(iter!=HeroLines.end())
	{
		iter2=iter++;
		if(iter2->hero==hero)
		{
			iter2->active=false;
			HeroLines.erase(iter2);
			break;
		}
	}
}
void AddHeroLine(Unit* Hero,Location to)
{
	RemoveHeroLine(Hero);
	Line ln;
	ln.active=true;
	ln.hero=Hero;
	ln.to=to;
	ln.color=GetPlayerColor((ln.hero)->dwOwnerSlot);
	HeroLines.push_back(ln);
}
void DanceUnits()
{
	DWORD len=0;
	DWORD arr = GetUnitArray(len);
	for(int i=0;i<len;i++)
	{
		Unit* unt = (Unit*)*(DWORD*)(arr + i*4);
		if(!IsValidUnitPtr(unt))
			continue;
		if(IsUnitOwnedByPlayer(unt))
		{
			float perc = getHpPercentage(unt);
			if(perc<0.75&&!IsDancing(unt))
			{
				float facing = GetUnitFacing(unt);
				AutoDance dancer;
				dancer.unit=unt;
				dancer.state = STATE_SHOULDDANCE;
				dancer.from = GetUnitLocation(unt);
				dancer.to = calcBack(facing,dancer.from,800.0);
			}
		}
	}
	list<AutoDance>::iterator  iter = dancers.begin(),iter2=iter;
	while(iter!=dancers.end())
	{
		iter2=iter++;
		if(!IsValidUnitPtr(iter2->unit)||IsDead(iter2->unit))
		{
			dancers.erase(iter2);
		}
		switch(iter2->state)
		{
		case STATE_SHOULDDANCE:
			{
				SendMoveAttackCommand(iter2->unit,MOVE,(iter2->to).X,(iter2->to).Y);
				iter2->state = STATE_DANCING;
			}break;
		case STATE_DANCING:
			{
				Location atm = GetUnitLocation(iter2->unit);
				if(Distance(atm,iter2->to)<50.0)
				{
					SendMoveAttackCommand(iter2->unit,ATTACK,(iter2->from).X,(iter2->from).Y);
				}

			}break;
		}
	}

}
void RedrawPortrait()
{
	__asm
	{
		PUSHAD;
		ADDR(_W3XGlobalClass,ECX)
		MOV ECX,DWORD PTR DS:[ECX+0x3F4];
		CALL _DrawPortrait;
		POPAD;
	}
}
void DrawPortrait(Unit* unt)
{
	DWORD getModel=0x6F32DA70,setPortr=0x6F33B840,Setportr2=0x6F358430;
	if(!unt)
		return;
	__asm
	{
		ADDR(_W3XGlobalClass,EDI)
		MOV EDI,DWORD PTR DS:[EDI+0x3F4];
		MOV ESI,unt
		MOV ECX,DWORD PTR DS:[ESI+0x30]
		CALL getModel;
		PUSH 0;
		PUSH EAX;
		MOV ECX,EDI
		CALL setPortr;
		PUSH ECX;
		MOV EAX,ESP;
		MOV ESI,unt
		MOV DWORD PTR DS:[EAX],ESI;
		ADD DWORD PTR DS:[ESI+4],1;
		MOV ECX,EDI;
		CALL Setportr2;

	}
}
void DrawInfo(Unit * unt)
{
	DWORD drawInfo=0x6F347720;
	if(!unt)
		return;
	__asm
	{
		ADDR(_W3XGlobalClass,EDI)
		MOV EDI,DWORD PTR DS:[EDI+0x3C4];
		MOV ECX,DWORD PTR SS:[EDI+0x130];
		MOV EDX,DWORD PTR SS:[ECX];
		PUSH 1
		MOV EAX,unt
		PUSH EAX
		CALL drawInfo
	}

}
void DrawSkills(Unit *unt)
{
	DWORD drawSkills=0x6F20AD80,shit=0x6F479100;
	if(!unt)
		return;
	__asm
	{
		MOV ESI,unt;
		LEA ECX,DWORD PTR DS:[ESI+0x1D8];
		CALL shit
		MOV EDI,EAX
		MOV ESI,0x000D0142;
		PUSH ESI
		PUSH 0
		MOV ECX,EDI
		CALL drawSkills;
	}
}
void PaintOverlay(void * Overlay)
{
	Duration * ovrl = (Duration*)Overlay;
	DWORD _194 = ovrl->_0194;
	__asm
	{
		MOV ECX,_194;
		MOV EAX,DWORD PTR DS:[ECX];
		MOV EDX,DWORD PTR DS:[EAX+0x68];
		MOV DWORD PTR DS:[ECX+0x90],1;
		CALL EDX;
	}
}
void RefreshBuffsCooldowns()
{
	map<DWORD,Cooldown*>::iterator iter = Cooldowns.begin();
	map<DWORD,Duration*>::iterator iter2 = Durations.begin();
	while(iter!=Cooldowns.end())
	{
		PaintOverlay((void*)iter->second);
		++iter;
	}
	while(iter2!=Durations.end())
	{
		PaintOverlay((void*)iter2->second);
		++iter2;
	}
}
void RefreshUnitbars()
{
	if(kMapConfig->getValueAs<bool>("EnableManaBars"))
	{
		if(!ManaBars.empty())
		{
			map<Unit*,UnitBar*>::iterator iter=ManaBars.begin(),iter2=iter;
			while(iter!=ManaBars.end())
			{
				iter2=iter++;
				UnitBar* bar = iter2->second;
				if(!IsValidUnitPtr(bar->owner)||IsDead(bar->owner))
					continue;
				if(bar==(UnitBar*)-1)
					continue;
				bar = GenerateManaBarForUnit(bar->owner);
				if(GetAsyncKeyState(VK_MENU)&0x8000)
				{
					bar->refCount=0;
					__asm
					{
						PUSH 1;
						MOV ECX,bar;
						MOV ESI,ECX;
						CALL _EraseUnitBar;
					}
					continue;
				}
				__asm
				{
					MOV ECX,bar;
					CALL _DrawUnitBar;
				}
			}
		}
		else
		{
			DWORD len =0;
			DWORD arr =GetUnitArray(len);
			for(int i=0;i<len;i++)
			{
				Unit* unt = (Unit*)*(DWORD*)(arr + i*4);
				if(!IsValidUnitPtr(unt)||IsDead(unt))
					continue;
				UnitBar* bar = GenerateManaBarForUnit(unt);
				if(bar==(UnitBar*)-1)
					continue;
				if(GetAsyncKeyState(VK_MENU)&0x8000)
				{
					bar->refCount=0;
					__asm
					{
						PUSH 1;
						MOV ECX,bar;
						MOV ESI,ECX;
						CALL _EraseUnitBar;
					}
					continue;
				}
				__asm
				{
					MOV ECX,bar;
					CALL _DrawUnitBar;
				}
			}
		}
	}
}
void CleanupUnitbar(UnitBar* bar)
{
	if(bar->refCount<0)
		printf("\nErasing manabar for unit %X reCount: %X bar value %X",bar->owner,bar->refCount,bar->barData);
	while(bar->refCount>0)
	{
		//printf("\nErasing manabar for unit %X reCount: %X bar value %X",bar->owner,bar->refCount,bar->barData);
		__asm
		{
			PUSH 1;
			MOV ECX,bar;
			MOV ESI,ECX;
			CALL _EraseUnitBar;
		}
		bar->refCount--;
	}
	bar->refCount=0;
}
void ManaBarsErase()
{
	map<Unit*,UnitBar*>::iterator iter=ManaBars.begin(),iter2=iter;
	while(iter!=ManaBars.end())
	{
		iter2=iter++;
		UnitBar* bar = iter2->second;
		CleanupUnitbar(bar);
	}
}
void DoCleanup(bool freemem)
{
	if(kMapConfig->getValueAs<bool>("EnableManaBars"))
	{
		map<Unit*,UnitBar*>::iterator iter=ManaBars.begin(),iter2=iter;
		while(iter!=ManaBars.end())
		{
			iter2=iter++;
			UnitBar* bar = iter2->second;
			if(freemem)
			{
				//printf("\nFreeing memory for manabar for unit %X reCount: %X bar value %X",bar->owner,bar->refCount,bar->barData);
				delete bar;
				ManaBars.erase(iter2);
			}
			else
				CleanupUnitbar(bar);
		}
		if(freemem)
			ManaBars.clear();
	}
	DeleteAllCooldowns();
	DeleteAllBuffDurations();
	HeroLines.clear();
}
bool ChangeLoadingBarText(char * newtext)
{
	__asm
	{
looplbl:
		ADDR(_LoadingBarPtr,EAX);
		TEST EAX,EAX;
		JE looplbl;
		MOV ECX,DWORD PTR DS:[EAX+0x190];
		TEST ECX,ECX;
		JE end;
		MOV EAX,newtext;
		PUSH EAX;
		CALL _ChangeLBText;
	}
	return true;
end:
	return false;
}
void DisplayText(char * text,float StayUpTime)
{
	__asm
	{
		PUSH 0xFFFED312;
		PUSH StayUpTime;
		PUSH text;
		ADDR(_W3XGlobalClass,ECX)
		CALL _PrettyPrint;
	}
}
DWORD GenerateButtonOverlay(DWORD Button)
{
	DWORD btnOverlay;
	__asm
	{
		MOV ESI,Button;
		PUSH 0;
		PUSH 0x22E;
		PUSH _GenerateCdConst;
		PUSH 0x124;
		CALL _OverlayGenerate;
		PUSH ESI;
		MOV ECX,EAX;
		CALL _OverlayInit1;
		FLDZ;
		PUSH 1;
		SUB ESP,8;
		FST DWORD PTR SS:[ESP+4];
		FSTP DWORD PTR SS:[ESP];
		MOV ECX,EAX;
		PUSH 8;
		PUSH ESI;
		PUSH 8;
		MOV btnOverlay,EAX;
		CALL _OverlayInit2;
		MOV EAX,_OverlayInitConst;
		FLD DWORD PTR DS:[EAX];
		PUSH ECX;
		MOV ECX,btnOverlay;
		FSTP DWORD PTR SS:[ESP];
		CALL _OverlayInit3;
		MOV EAX,_OverlayInitConst;
		FLD DWORD PTR DS:[EAX];
		PUSH ECX;
		MOV ECX,btnOverlay;
		FSTP DWORD PTR SS:[ESP];
		CALL _OverlayInit4;
	}
	return btnOverlay;
}
DWORD GenerateBuffOverlay(DWORD BuffIcon)
{
	float SizeConst=0.0090000f;

	DWORD btnOverlay;
	__asm
	{
		MOV ESI,BuffIcon;
		PUSH 0;
		PUSH 0x22E;
		PUSH _GenerateCdConst;
		PUSH 0x124;
		CALL _OverlayGenerate;
		PUSH ESI;
		MOV ECX,EAX;
		CALL _OverlayInit1;
		FLDZ;
		PUSH 1;
		SUB ESP,8;
		FST DWORD PTR SS:[ESP+4];
		FSTP DWORD PTR SS:[ESP];
		MOV ECX,EAX;
		PUSH 8;
		PUSH ESI;
		PUSH 8;
		MOV btnOverlay,EAX;
		CALL _OverlayInit2;
		LEA EAX,SizeConst;
		FLD DWORD PTR DS:[EAX];
		PUSH ECX;
		MOV ECX,btnOverlay;
		FSTP DWORD PTR SS:[ESP];
		CALL _OverlayInit3;
		LEA EAX,SizeConst;
		FLD DWORD PTR DS:[EAX];
		PUSH ECX;
		MOV ECX,btnOverlay;
		FSTP DWORD PTR SS:[ESP];
		CALL _OverlayInit4;
	}
	return btnOverlay;
}
float GetSquareSize(DWORD Button)
{
	float res;
	__asm
	{
		MOV ESI,Button;
		MOV ECX,DWORD PTR DS:[ESI+0x58];
		TEST ECX,ECX;
		JE end;
		LEA EAX,res;
		MOV DWORD PTR DS:[EAX],ECX;
	}
	return res;
end:
	return 0.0;
}
Cooldown* GenerateButtonNumberOverlay(DWORD Button)
{
	DWORD buttonoverlay;
	float FontSize=0.029f;
	float yPosOffset=0.0098125f;
	float xPosOffset=-0.0108125f;
	Cooldown *ret=new Cooldown;
	ret->_0194=-1;
	ret->_0198=-1;
	DWORD Colour = 0x00FFFFFF;
	if((*(DWORD*)Button)==0x6F94E7CC)
	{
		FontSize=0.009f;
		buttonoverlay = GenerateBuffOverlay(Button);
		yPosOffset=0.002f;
		xPosOffset=-0.002f;
		Colour = 0xFFFFFFFF;  //o.O weirdly enough it's R G B alpha 
	}
	else 
	{
		buttonoverlay = GenerateButtonOverlay(Button);
		float BtnSize = (GetSquareSize(Button)-0.007f)/2.0;
		FontSize = BtnSize*2.0f + 0.002f;
		if(BtnSize>0.0)
		{
			yPosOffset=(BtnSize/2.0)+(BtnSize/5.0);
			xPosOffset=-((BtnSize/2.0)+(BtnSize/5.0));
		}
	}
	if(!buttonoverlay)
		return ret;
	DWORD numOverlay=0;
	__asm
	{
		MOV ESI,Button;
		PUSH 0;
		PUSH 0x235;
		PUSH _GenerateCdConst;
		PUSH 0x0C4;
		CALL _OverlayGenerate;
		MOV ECX,buttonoverlay;
		PUSH 1;
		PUSH 2;
		PUSH ECX;
		MOV ECX,EAX;
		CALL _FillCdValues;
		MOV numOverlay,EAX;
		MOV EDI,EAX;
		PUSH 0;
		PUSH FontSize;
		XOR EDX,EDX;
		MOV ECX,_FontSizeConst;
		CALL _ChangeFontSize;
		PUSH EAX;
		MOV ECX,EDI;
		CALL _OverlayNumberInit;
		LEA EAX,yPosOffset;
		FLD DWORD PTR DS:[EAX];
		MOV EDX,buttonoverlay;
		PUSH 1;
		SUB ESP,8;
		FSTP DWORD PTR SS:[ESP+4];
		MOV ECX,EDI;
		LEA EAX,xPosOffset;
		FLD DWORD PTR DS:[EAX];
		FSTP DWORD PTR SS:[ESP];
		PUSH 4;
		PUSH EDX;
		PUSH 4;
		CALL _OverlayNumberPosition;
		MOV EAX,Colour;
		MOV ECX,numOverlay;
		MOV DWORD PTR DS:[ECX+0x68],EAX;
	}
	ret->_0194=buttonoverlay;
	ret->_0198=numOverlay;
	ret->Time=-1;
	return ret;
}
void EraseBtnCooldown(Cooldown *cd)
{
	DWORD _0194=cd->_0194;
	__asm
	{
		MOV ECX,_0194;
		MOV EAX,DWORD PTR DS:[ECX];
		MOV EDX,DWORD PTR DS:[EAX+0x64];
		MOV DWORD PTR DS:[ECX+0x90],0;
		CALL EDX;
	}
}
void DrawBtnCooldown(Cooldown *cd)
{
	DWORD _0194=cd->_0194;
	DWORD _0198=cd->_0198;
	DWORD time=cd->Time;
	__asm
	{
		MOV ECX,_0194;
		MOV EAX,DWORD PTR DS:[ECX+0x90];
		TEST EAX,EAX;
		JNE end;
	}
	PaintOverlay((void*)cd);
	__asm
	{
end:
		MOV ECX,_0198;
		MOV EDI,time;
		PUSH EDI;
		CALL _DrawOverlayNumber;
	}
}
void EraseBuffDuration(Duration* cd)
{
	DWORD _0194=cd->_0194;
	__asm
	{
		MOV ECX,_0194;
		MOV EAX,DWORD PTR DS:[ECX];
		MOV EDX,DWORD PTR DS:[EAX+0x64];
		MOV DWORD PTR DS:[ECX+0x90],0;
		CALL EDX;
	}
}
void DrawBuffDuration(Duration* cd)
{
	DWORD _0194=cd->_0194;
	DWORD _0198=cd->_0198;
	DWORD time=cd->Time;
	__asm
	{
		MOV ECX,_0194;
		MOV EAX,DWORD PTR DS:[ECX+0x90];
		TEST EAX,EAX;
		JNE end;
	}
	PaintOverlay((void*)cd);
	__asm
	{
end:
		MOV ECX,_0198;
		MOV EDI,time;
		PUSH EDI;
		CALL _DrawOverlayNumber;
	}
}
bool GetCooldownForButton(DWORD btn,Cooldown* &Res )
{
	map<DWORD,Cooldown*>::iterator iter = Cooldowns.find(btn);
	if(iter!=Cooldowns.end())
	{
		
		Res=iter->second;
		return true;
		//InitBar(manabar,unit);
		//UpdateBar(manabar,unit);
	}
	else
		return false;
}
Cooldown* GenerateCooldown(DWORD button)
{
	map<DWORD,Cooldown*>::iterator iter = Cooldowns.find(button);
	if(iter==Cooldowns.end())
	{
		Cooldown *cd = GenerateButtonNumberOverlay(button);
		if(cd->_0194!=-1&&cd->_0198!=-1)
		{
			Cooldowns[button]=cd;
			PaintOverlay((void*)cd);
			return NULL;
		}
	}
	else
		return iter->second;
}
void RemoveCooldown(DWORD button)
{
	map<DWORD,Cooldown*>::iterator iter = Cooldowns.find(button);
	if(iter!=Cooldowns.end())
	{
		Cooldown *cd = iter->second;
		EraseBtnCooldown(cd);
		delete cd;
		Cooldowns.erase(iter);
	}
}
void DeleteAllBuffDurations()
{
	map<DWORD,Duration*>::iterator iter=Durations.begin(),iter2=iter;
	while(iter!=Durations.end())
	{
		iter2=iter++;
		EraseBuffDuration(iter2->second);
		delete iter2->second;
		Durations.erase(iter2);		
	}
	Durations.clear();
}
void RemoveBuffDuration(DWORD BuffIcon)
{
	map<DWORD,Duration*>::iterator iter = Durations.find(BuffIcon);
	if(iter!=Durations.end())
	{
		Duration *cd = iter->second;
		EraseBuffDuration(cd);
		delete cd;
		Durations.erase(iter);
	}
}
bool GeBuffDurationForBuff(DWORD BuffIcon,Duration* &Res )
{
	map<DWORD,Duration*>::iterator iter = Durations.find(BuffIcon);
	if(iter!=Durations.end())
	{

		Res=iter->second;
		return true;
		//InitBar(manabar,unit);
		//UpdateBar(manabar,unit);
	}
	else
		return false;
}
Duration* GenerateBuffDuration(DWORD BuffIcon)
{
	map<DWORD,Duration*>::iterator iter = Durations.find(BuffIcon);
	if(iter==Durations.end())
	{
		Duration *cd = (Duration*)GenerateButtonNumberOverlay(BuffIcon);
		if(cd->_0194!=-1&&cd->_0198!=-1)
		{
			Durations[BuffIcon]=cd;
			PaintOverlay((void*)cd);
			return NULL;
		}
	}
	else
		return iter->second;
}
float GetTimeLeftForButton(DWORD Button,DWORD &intLeft)
{
	float Result;
	__asm
	{
		MOV ESI,Button;
		MOV EAX,DWORD PTR DS:[ESI+0x190];
		MOV EDI,DWORD PTR DS:[EAX+0x6D4];
		MOV EAX,DWORD PTR DS:[EAX+8];
		MOV EDX,DWORD PTR DS:[EDI];
		MOV EDX,DWORD PTR DS:[EDX+0x2DC];
		PUSH EAX;
		LEA EAX,Result;
		PUSH EAX;
		MOV ECX,EDI;
		CALL EDX;
	}
	intLeft=floor(Result+0.5);
	return Result;
}
float GetDurationLeftForBuff(DWORD BuffIcon,DWORD &intLeft)
{
	float Result;
	__asm
	{
		MOV ESI,BuffIcon;
		LEA EDI,DWORD PTR DS:[ESI+0x978];
		MOV EAX,DWORD PTR DS:[EDI];
		LEA ECX,DWORD PTR DS:[EAX+0x84];
		MOV EAX,DWORD PTR DS:[ECX];
		MOV EAX,DWORD PTR DS:[EAX+0x18];
		LEA EDX,Result;
		PUSH EDX;
		CALL EAX;
	}
	intLeft=floor(Result+0.5);
	return Result;
}
float GetTimeLeftForBar(DWORD Bar)
{
	DWORD rd;
	if(!ReadProcessMemory(GetCurrentProcess(),(LPVOID)Bar,&rd,sizeof(rd),NULL))
		return -1.0;
	float res;
	__asm
	{
		MOV ECX,Bar;
		MOV ESI,DWORD PTR DS:[ECX+0x138];
		LEA ECX,DWORD PTR DS:[ESI+0x7C];
		LEA EAX,res;
		PUSH EAX;
		CALL _GetBarLeft;
	}
	return res;
}
void DeleteAllCooldowns()
{
	map<DWORD,Cooldown*>::iterator iter=Cooldowns.begin(),iter2=iter;
	while(iter!=Cooldowns.end())
	{
		iter2=iter++;
		EraseBtnCooldown(iter2->second);
		delete iter2->second;
		Cooldowns.erase(iter2);		
	}
	Cooldowns.clear();
}

void ChangeStartTime(float NewTime)
{
	double Ct = NewTime;
	patchmgr.AddPatch(_StartTimeConst,(char*)&NewTime,sizeof(NewTime));
	patchmgr.AddPatch(_CountdownConst,(char*)&Ct,sizeof(Ct));
}