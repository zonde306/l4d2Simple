#include "baseplayerresource.h"
#include "../interfaces.h"

#define DECL_NETPROP_GET_2EX(_type,_index,_offset)		(*reinterpret_cast<_type*>(reinterpret_cast<DWORD>(this) + _offset + (_index * 4)))
#define DECL_NETPROP_GET_2(_type,_index)				DECL_NETPROP_GET_2EX(_type,_index,offset)
#define DECL_NETPROP_GET_2L(_type)						DECL_NETPROP_GET_2EX(_type,player,offset)

CBasePlayerResource* g_pPlayerResource = nullptr;

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
	return DECL_NETPROP_GET_2L(WORD);
}

bool CBasePlayerResource::IsAlive(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_bAlive"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(bool);
}

int CBasePlayerResource::GetTeam(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_iTeam"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(BYTE);
}

bool CBasePlayerResource::IsConnected(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_bConnected"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(BYTE);
}

bool CBasePlayerResource::IsGhost(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_isGhost"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(BYTE);
}

bool CBasePlayerResource::IsListenHost(int player)
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayerResource"), XorStr("m_listenServerHost"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET_2L(BYTE);
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
