#include "Globals.h"
#ifndef GAME_H
#define GAME_H
union Float
{
	float X;
	DWORD dX;
};
DWORD GetBaseClassIdByRace(DWORD race);
DWORD CoordToMinimap(float Loc,DWORD offset);
DWORD Distance(MmapLoc one,MmapLoc two);
DWORD GetGoldMineClassIDbyBase(Unit *base);
DWORD GetUnitArray(DWORD &Sz);
DWORD GetWorkerClassIDbyBase(Unit * base);
float Distance(Location one, Location two);
Location calcBack(float facing, Location from,float Distance);
Unit* GetMine(Unit* base);
Unit* GetBaseBuilding();
Unit* FindWorkerAt(DWORD &ptr,DWORD wId);
Unit* FindPlayerBase();
Unit** GetWorkersArray(int &len,DWORD wId,int count);
void PingMinimap(Location loc, DWORD Color, DWORD PingType=0,float StayUpTime=2.0f);
void ProcessGameCmd(Unit*,Command* CommandData);
void SendUnitToMine(Unit* unit,Unit* mine);
void HarvestCommand();
void AutoMine(DWORD num);
void ConvertMmap(MmapLoc& loc);
void TextOutWc3(char * text,DWORD msgtype,float StayUpTime=10.0);
void DisplayText(char * text,float StayUpTime=10.0);
void ExecuteCmd(Unit* unit,Location target,DWORD CmdId,Unit * tarunit);
void SetCameraField(float val,float seconds,DWORD field);
void BuildWorker(DWORD Classid);
void AutoQueue();
void DrawUnitOnMap(Unit *unit);
void IllusionInvisColorChange(Unit * unt);
void DrawPixel(DWORD X,DWORD Y,DWORD Color);
void DrawLine(MmapLoc From, MmapLoc To,DWORD Color);
void DrawLine(Location from,Location to,DWORD Color);
void DrawHeroLines();
void RemoveHeroLine(Unit* hero);
void AddHeroLine(Unit* Hero,Location to);
void DanceUnits();
void RedrawPortrait();
void DrawPortrait(Unit* unt);
void DrawInfo(Unit * unt);
void DrawSkills(Unit *unt);
void RefreshUnitbars();
void DoCleanup(bool freemem=false);
void ManaBarsErase();
void CleanupUnitbar(UnitBar*);
bool ChangeLoadingBarText(char * newtext);
MmapLoc LocationToMinimap(Location loc);
Cooldown* GenerateButtonNumberOverlay(DWORD btn);
float GetTimeLeftForButton(DWORD Button,DWORD &intLeft);
float GetDurationLeftForBuff(DWORD BuffIcon,DWORD &intLeft);
float GetTimeLeftForBar(DWORD Bar);
bool GetCooldownForButton(DWORD btn,Cooldown*&);
void DrawBtnCooldown(Cooldown *cd);
void EraseBtnCooldown(Cooldown *cd);
void DeleteAllCooldowns();
void RemoveCooldown(DWORD button);
Cooldown* GenerateCooldown(DWORD button);
//
void PaintOverlay(void * Overlay);
void RefreshBuffsCooldowns();
float GetSquareSize(DWORD Button);
//
DWORD GenerateBuffOverlay(DWORD BuffIcon);
bool GeBuffDurationForBuff(DWORD BuffIcon,Duration*&);
void DrawBuffDuration(Duration* cd);
void EraseBuffDuration(Duration* cd);
void DeleteAllBuffDurations();
void RemoveBuffDuration(DWORD BuffIcon);
Duration* GenerateBuffDuration(DWORD BuffIcon);

void ChangeStartTime(float NewTime);

#endif