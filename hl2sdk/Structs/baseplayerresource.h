#pragma once
#include "baseentity.h"

class CBasePlayerResource : public CBaseEntity
{
public:
	static CBasePlayerResource* Get();

public:
	int GetMaxHealth(int player);
	int GetHealth(int player);
	int GetPing(int player);
	bool IsAlive(int player);
	int GetTeam(int player);
	bool IsConnected(int player);
	bool IsGhost(int player);
	bool IsListenHost(int player);
	
	bool IsFinaleRound();
	bool IsAnySurvivorLeftSafeArea();
	bool IsTeamFrozen();
	int GetRandomSeed();
};

// 不要直接使用这个，用 CBasePlayerResource::Get() 来获取指针
extern CBasePlayerResource* g_pPlayerResource;
