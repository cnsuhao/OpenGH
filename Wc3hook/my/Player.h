#include "Globals.h"
#ifndef PLAYER_H
#define PLAYER_H
bool IsPlayerEnemy(DWORD one,DWORD two);
DWORD GetPlayerByNumber(DWORD plNum);
DWORD GetPlayerNumber();
DWORD GetPlayerRace();
DWORD GetPlayer();
DWORD GetPlayerColor(DWORD slot);
DWORD GetPlayerColorByNumber(DWORD num);
DWORD GetPlayerCircleColorBySlot(DWORD dwPlayerSlot,Unit* unit);
DWORD GetOwningPlayer(Unit * unit);
#endif