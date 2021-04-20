#include "baseplayerresource.h"
#include "../interfaces.h"

#define DECL_NETPROP_GET_2EX(_type,_index,_offset)		(*reinterpret_cast<_type*>(reinterpret_cast<DWORD>(this) + _offset + (_index * 4)))
#define DECL_NETPROP_GET_2(_type,_index)				DECL_NETPROP_GET_2EX(_type,_index,offset)
#define DECL_NETPROP_GET_2L(_type)						DECL_NETPROP_GET_2EX(_type,player,offset)

#define SIG_RESOURCE_ISGHOST		XorStr("55 8B EC 8B 45 08 8A 84 08 ? ? ? ? 5D C2 04 00")
#define SIG_RESOURCE_ZOMBIETYPE		XorStr("55 8B EC 8B 45 08 8B 84 81 ? ? ? ? 5D C2 04 00")

CBasePlayerResource* g_pPlayerResource = nullptr;
CBaseGameRulesProxy* g_pGameRulesProxy = nullptr;

CBasePlayerResource * CBasePlayerResource::Get()
{
	const static auto findPointer = []() -> CBasePlayerResource*
	{
		int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
		for (int i = g_pInterface->Engine->GetMaxClients() + 1; i <= maxEntity; ++i)
		{
			CBasePlayerResource* entity = reinterpret_cast<CBasePlayerResource*>(g_pInterface->EntList->GetClientEntity(i));
			if (entity == nullptr || entity->IsDormant() || entity->GetClassID() != ET_TerrorPlayerResource)
				continue;

			return entity;
		}

		return nullptr;
	};

	try
	{
		if (g_pPlayerResource == nullptr || g_pPlayerResource->IsDormant() ||
			g_pPlayerResource->GetClassID() != ET_TerrorPlayerResource)
			g_pPlayerResource = findPointer();
	}
	catch (...)
	{
		g_pPlayerResource = findPointer();
	}

	return g_pPlayerResource;
}

int CBasePlayerResource::GetMaxHealth(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_maxHealth"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(WORD);
}

int CBasePlayerResource::GetHealth(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_iHealth"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(WORD);
}

int CBasePlayerResource::GetPing(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_iPing"));
	Assert_NetProp(offset);

	/*
	LPVOID ptr = GetDataPointer();
	using FnGetPing = int(__thiscall*)(LPVOID, int);
	FnGetPing fn = Utils::GetVTableFunction<FnGetPing>(ptr, 10);
	if (fn != nullptr)
		return fn(ptr, player);
	*/

	return DECL_NETPROP_GET_2L(WORD);
}

bool CBasePlayerResource::IsAlive(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_bAlive"));
	Assert_NetProp(offset);

	/*
	LPVOID ptr = GetDataPointer();
	using FnIsAlive = int(__thiscall*)(LPVOID, int);
	FnIsAlive fn = Utils::GetVTableFunction<FnIsAlive>(ptr, 5);
	if (fn != nullptr)
		return fn(ptr, player);
	*/

	return DECL_NETPROP_GET_2L(bool);
}

int CBasePlayerResource::GetTeam(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_iTeam"));
	Assert_NetProp(offset);

	/*
	LPVOID ptr = GetDataPointer();
	using FnGetTeam = int(__thiscall*)(LPVOID, int);
	FnGetTeam fn = Utils::GetVTableFunction<FnGetTeam>(ptr, 13);
	if (fn != nullptr)
		return fn(ptr, player);
	*/

	return DECL_NETPROP_GET_2L(BYTE);
}

bool CBasePlayerResource::IsConnected(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_bConnected"));
	Assert_NetProp(offset);

	/*
	LPVOID ptr = GetDataPointer();
	using FnIsConnected = bool(__thiscall*)(LPVOID, int);
	FnIsConnected fn = Utils::GetVTableFunction<FnIsConnected>(ptr, 4);
	if (fn != nullptr)
		return fn(ptr, player);
	*/

	return DECL_NETPROP_GET_2L(BYTE);
}

bool CBasePlayerResource::IsGhost(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_isGhost"));
	Assert_NetProp(offset);

	/*
	using FnIsGhost = bool(__thiscall*)(CBasePlayerResource*, int);
	static FnIsGhost fn = reinterpret_cast<FnIsGhost>(Utils::FindPattern(XorStr("client.dll"), SIG_RESOURCE_ISGHOST));
	if (fn != nullptr)
		return fn(this, player);
	*/

	return DECL_NETPROP_GET_2L(BYTE);
}

bool CBasePlayerResource::IsListenHost(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_listenServerHost"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(BYTE);
}

const char * CBasePlayerResource::GetName(int player)
{
	/*
	LPVOID ptr = GetDataPointer();
	using FnGetName = const char*(__thiscall*)(LPVOID, int);
	FnGetName fn = Utils::GetVTableFunction<FnGetName>(ptr, 8);
	if (fn != nullptr)
		return fn(ptr, player);
	*/

	return nullptr;
}

int CBasePlayerResource::GetScore(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_iScore"));
	Assert_NetProp(offset);
	
	/*
	LPVOID ptr = GetDataPointer();
	using FnGetScore = int(__thiscall*)(LPVOID, int);
	FnGetScore fn = Utils::GetVTableFunction<FnGetScore>(ptr, 8);
	if (fn != nullptr)
		return fn(ptr, player);
	*/
	
	return DECL_NETPROP_GET(WORD);
}

int CBasePlayerResource::GetDeaths(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_iDeaths"));
	Assert_NetProp(offset);

	/*
	LPVOID ptr = GetDataPointer();
	using FnGetDeaths = int(__thiscall*)(LPVOID, int);
	FnGetDeaths fn = Utils::GetVTableFunction<FnGetDeaths>(ptr, 9);
	if (fn != nullptr)
		return fn(ptr, player);
	*/

	return DECL_NETPROP_GET(WORD);
}

int CBasePlayerResource::GetZombie(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_zombieClass"));
	Assert_NetProp(offset);

	/*
	using FnGetZombie = int(__thiscall*)(CBasePlayerResource*, int);
	static FnGetZombie fn = reinterpret_cast<FnGetZombie>(Utils::FindPattern(XorStr("client.dll"), SIG_RESOURCE_ZOMBIETYPE));
	if (fn != nullptr)
		return fn(this, player);
	*/

	return DECL_NETPROP_GET(BYTE);
}

bool CBasePlayerResource::IsBot(int player)
{
	LPVOID ptr = GetDataPointer();
	using FnIsBot = bool(__thiscall*)(LPVOID, int);
	FnIsBot fn = Utils::GetVTableFunction<FnIsBot>(ptr, 6);
	if (fn != nullptr)
		return fn(ptr, player);
	
	return false;
}

bool CBasePlayerResource::IsFinaleRound()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_isFinale"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(BYTE);
}

bool CBasePlayerResource::IsAnySurvivorLeftSafeArea()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_hasAnySurvivorLeftSafeArea"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(BYTE);
}

bool CBasePlayerResource::IsTeamFrozen()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_isTeamFrozen"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(BYTE);
}

int CBasePlayerResource::GetRandomSeed()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_sharedRandomSeed"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

LPVOID CBasePlayerResource::GetDataPointer()
{
	return *reinterpret_cast<LPVOID*>(reinterpret_cast<DWORD>(this) + 1592);
}

CBaseGameRulesProxy* CBaseGameRulesProxy::Get()
{
	const static auto findPointer = []() -> CBaseGameRulesProxy*
	{
		int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
		for (int i = g_pInterface->Engine->GetMaxClients() + 1; i <= maxEntity; ++i)
		{
			CBaseGameRulesProxy* entity = reinterpret_cast<CBaseGameRulesProxy*>(g_pInterface->EntList->GetClientEntity(i));
			if (entity == nullptr || entity->IsDormant() || entity->GetClassID() != ET_TerrorGameRulesProxy)
				continue;

			return entity;
		}

		return nullptr;
	};

	try
	{
		if (g_pGameRulesProxy == nullptr || g_pGameRulesProxy->IsDormant() ||
			g_pGameRulesProxy->GetClassID() != ET_TerrorPlayerResource)
			g_pGameRulesProxy = findPointer();
	}
	catch (...)
	{
		g_pGameRulesProxy = findPointer();
	}

	return g_pGameRulesProxy;
}
