#include "Globals.h"
#ifndef UNIT_H
#define UNIT_H
float GetUnitFloatStat(Unit * unit,DWORD statNum);
float GetUnitFacing(Unit * unt);
float getMpPercentage(Unit * unit);
float getHpPercentage(Unit * unit);
void SetSelectedUnit(Unit * unt);
void SelectUnit(Unit* unit);
void LocalSelectUnit(Unit * unit);
void SelectMultipleUnits( Unit** units,int count);
void SendMoveAttackCommand(Unit* unit,DWORD cmdId,float X,float Y,Unit* target=NULL);
bool IsFogged();
bool IsUnitFoggedToPlayer(DWORD plr,Unit* unit);
bool IsValidUnitPtr(Unit* ptr);
bool IsValidUnitPtr(DWORD ptr);
bool IsValidItemPtr(DWORD ptr);
bool IsDead(Unit * unt);
bool IsIllusion(Unit * unt);
bool IsUnitOwnedByPlayer(Unit* unit);
bool IsUnitOwnedByPlayer(DWORD plr,Unit* unit);
bool IsHero(Unit* unit);
bool IsDancing(Unit * unit);
bool IsUnitVisibleToPlayer(Unit* unit,DWORD Player);
Unit * GetSelectedUnit();
Location GetUnitLocation(Unit * unit);
UnitBar* GenerateBar(Unit * unit);
void InitBar(UnitBar* &bar,Unit* owner);
float GetUnitScale(Unit * unit);
UnitBar* GenerateManaBar();
UnitBar* GenerateManaBarForUnit(Unit * unit);
UnitBar* GetManaBarForUnit(Unit * unt);
Location GetUnitOnscreenLoc(Unit * unit);
UnitBar* UpdateBar(UnitBar* &bar,Unit * unit);
DWORD GetSelectedUnitsCount();
#endif