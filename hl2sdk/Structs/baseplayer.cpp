#include "baseplayer.h"
#include "../indexes.h"

int CBasePlayer::GetHealth()
{
	return GetNetProp<int>(XorStr("DT_BasePlayer"), XorStr("m_iHealth"));
}

bool CBasePlayer::IsAlive()
{
	if (IsDormant())
		return false;
	
	return (GetNetProp<LifeStates_t>(XorStr("DT_BasePlayer"), XorStr("m_lifeState")) == LIFE_ALIVE);
}

MoveType_t CBasePlayer::GetMoveType()
{
	return GetNetProp<MoveType_t>(XorStr("DT_BasePlayer"), XorStr("movetype"));
}

Vector& CBasePlayer::GetPunch()
{
	return *reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(this) + indexes::GetPunch);
}
