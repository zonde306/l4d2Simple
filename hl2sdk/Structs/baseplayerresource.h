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
	const char* GetName(int player);
	int GetScore(int player);
	int GetDeaths(int player);
	int GetZombie(int player);
	bool IsBot(int player);
	
	bool IsFinaleRound();
	bool IsAnySurvivorLeftSafeArea();
	bool IsTeamFrozen();
	int GetRandomSeed();

private:
	LPVOID GetDataPointer();
};

class CBaseGameRulesProxy : public CBaseEntity
{
public:
	static CBaseGameRulesProxy* Get();
};

extern CBasePlayerResource* g_pPlayerResource;
extern CBaseGameRulesProxy* g_pGameRulesProxy;