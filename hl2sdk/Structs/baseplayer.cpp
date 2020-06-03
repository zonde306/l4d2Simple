#include "baseplayer.h"
#include "../interfaces.h"
#include "../hook.h"
#include "convar.h"
#include "../indexes.h"

#define HITBOX_HEAD_COMMON			15	// 普感
#define HITBOX_HEAD_PLAYER			10	// 生还者/特感
#define HITBOX_HEAD_COMMON_1		14
#define HITBOX_HEAD_COMMON_2		15
#define HITBOX_HEAD_COMMON_3		16
#define HITBOX_HEAD_COMMON_4		17
#define HITBOX_HEAD_JOCKEY			4
#define HITBOX_HEAD_SPITTER			4
#define HITBOX_HEAD_CHARGER			9
#define HITBOX_HEAD_TANK			12
#define HITBOX_HEAD_WITCH			10

#define HITBOX_CHEST_PLAYER			7
#define HITBOX_CHEST_COMMON			8
#define HITBOX_CHEST_JOCKEY			4
#define HITBOX_CHEST_SPITTER		2
#define HITBOX_CHEST_CHARGER		8
#define HITBOX_CHEST_TANK			9
#define HITBOX_CHEST_WITCH			8

Vector CBasePlayer::GetEyePosition()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecViewOffset[0]"));
	Assert_NetProp(offset);
	return (GetAbsOrigin() + DECL_NETPROP_GET(Vector));
}

QAngle CBasePlayer::GetEyeAngles()
{
	static int offset = GetNetPropOffset(XorStr("DT_CSPlayer"), XorStr("m_angEyeAngles[0]"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(QAngle);
}

float CBasePlayer::GetFriction()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_flFriction"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(float);
}

int CBasePlayer::GetTeam()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_iTeamNum"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(byte);
}

Vector CBasePlayer::GetVelocity()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecVelocity[0]"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(Vector);
}

int CBasePlayer::GetHealth()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_iHealth"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

int CBasePlayer::GetTempHealth()
{
	static int bufferOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_healthBuffer"));
	static int timeOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_healthBufferTime"));
	static ConVar* decayRate = g_pInterface->Cvar->FindVar(XorStr("pain_pills_decay_rate"));
	int amount = static_cast<int>(ceil(DECL_NETPROP_GET_EX(bufferOffset, float) -
		((g_pClientPrediction->GetServerTime() - DECL_NETPROP_GET_EX(timeOffset, float)) *
		decayRate->GetFloat())));

	return max(amount, 0);
}

CBaseWeapon * CBasePlayer::GetActiveWeapon()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatCharacter"), XorStr("m_hActiveWeapon"));
	Assert_NetProp(offset);

	CBaseHandle handle = DECL_NETPROP_GET(CBaseHandle);
	if (!handle.IsValid())
		return nullptr;

	return reinterpret_cast<CBaseWeapon*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
}

int CBasePlayer::GetCrosshairID()
{
	return DECL_NETPROP_GET_EX(indexes::GetCrosshairsId, int);
}

bool CBasePlayer::IsGhost()
{
	using FnIsGhost = bool(__thiscall*)(CBasePlayer*);
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_isGhost"));
	FnIsGhost fn = Utils::GetVTableFunction<FnIsGhost>(this, indexes::IsGhost);
	Assert_NetProp(offset);

	if(fn != nullptr)
		return fn(this);

	return (DECL_NETPROP_GET(byte) != 0);
}

int CBasePlayer::GetAmmo(int ammoType)
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_iAmmo"));
	Assert_NetProp(offset);
	return *reinterpret_cast<WORD*>(reinterpret_cast<DWORD>(this) + offset + (ammoType * 4));
}

CBasePlayer * CBasePlayer::GetCurrentAttacker()
{
	static int tongueOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_tongueOwner"));
	static int rideOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_jockeyAttacker"));
	static int pounceOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pounceAttacker"));
	static int pummelOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pummelAttacker"));
	static int carryOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_carryAttacker"));
	Assert_NetProp(tongueOffset);
	Assert_NetProp(rideOffset);
	Assert_NetProp(pounceOffset);
	Assert_NetProp(pummelOffset);
	Assert_NetProp(carryOffset);

	CBaseHandle handle = DECL_NETPROP_GET_EX(tongueOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(rideOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pounceOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pummelOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(carryOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;

	return nullptr;

finish_check_handle:
	return reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
}

CBasePlayer * CBasePlayer::GetCurrentVictim()
{
	static int tongueOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_tongueVictim"));
	static int rideOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_jockeyVictim"));
	static int pounceOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pounceVictim"));
	static int pummelOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pummelVictim"));
	static int carryOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_carryVictim"));
	Assert_NetProp(tongueOffset);
	Assert_NetProp(rideOffset);
	Assert_NetProp(pounceOffset);
	Assert_NetProp(pummelOffset);
	Assert_NetProp(carryOffset);

	CBaseHandle handle = DECL_NETPROP_GET_EX(tongueOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(rideOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pounceOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pummelOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(carryOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;

	return nullptr;

finish_check_handle:
	return reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
}

bool CBasePlayer::IsOnGround()
{
	return (GetGroundEntity() != nullptr && (GetFlags() & FL_ONGROUND));
}

Vector CBasePlayer::GetHeadOrigin()
{
	int classId = GetClassID();

	if (classId == ET_SURVIVORBOT || classId == ET_CTERRORPLAYER || classId == ET_WITCH ||
		classId == ET_SMOKER || classId == ET_BOOMER || classId == ET_HUNTER)
		return GetHitboxOrigin(HITBOX_HEAD_PLAYER);

	if (classId == ET_TANK)
		return GetHitboxOrigin(HITBOX_HEAD_TANK);

	if (classId == ET_JOCKEY || classId == ET_SPITTER)
		return GetHitboxOrigin(HITBOX_HEAD_JOCKEY);

	if (classId == ET_CHARGER)
		return GetHitboxOrigin(HITBOX_HEAD_CHARGER);

	if (classId == ET_INFECTED)
		return GetHitboxOrigin(HITBOX_HEAD_COMMON);

	return GetAbsOrigin();
}

bool CBasePlayer::IsDying()
{
	if(IsIncapacitated())
		return false;

	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_bIsOnThirdStrike"));
	Assert_NetProp(offset);
	return (DECL_NETPROP_GET(byte) != 0);
}

std::string CBasePlayer::GetCharacterName()
{
	int classId = GetClassID();
	std::string buffer;

	switch (classId)
	{
	case ET_SMOKER:
		buffer = XorStr("Smoker");
		break;
	case ET_BOOMER:
		buffer = XorStr("Boomer");
		break;
	case ET_HUNTER:
		buffer = XorStr("Hunter");
		break;
	case ET_SPITTER:
		buffer = XorStr("Spitter");
		break;
	case ET_JOCKEY:
		buffer = XorStr("Jockey");
		break;
	case ET_CHARGER:
		buffer = XorStr("Charger");
		break;
	case ET_WITCH:
		buffer = XorStr("Witch");
		break;
	case ET_TANK:
		buffer = XorStr("Tank");
		break;
	case ET_SURVIVORBOT:
	case ET_CTERRORPLAYER:
		// const char* models = entity->GetNetProp<const char*>(XorStr("DT_BasePlayer"), XorStr("m_ModelName"));
		const model_t* models = GetModel();
		if (models == nullptr || models->name[0] == '\0')
			break;

		if (models->name[0] != 'm' || models->name[7] != 's' || models->name[17] != 's')
			break;

		if (models->name[26] == 'g')			// models/survivors/survivor_gambler.mdl
			buffer = XorStr("Nick");		// 西装
		else if (models->name[26] == 'p')		// models/survivors/survivor_producer.mdl
			buffer = XorStr("Rochelle");	// 黑妹
		else if (models->name[26] == 'c')		// models/survivors/survivor_coach.mdl
			buffer = XorStr("Coach");		// 黑胖
		else if (models->name[26] == 'n')		// models/survivors/survivor_namvet.mdl
			buffer = XorStr("Bill");		// 老头
		else if (models->name[26] == 't')		// models/survivors/survivor_teenangst.mdl
			buffer = XorStr("Zoey");		// 萌妹
		else if (models->name[26] == 'b')		// models/survivors/survivor_biker.mdl
			buffer = XorStr("Francis");		// 背心
		else if (models->name[26] == 'm')
		{
			if (models->name[27] == 'e')		// models/survivors/survivor_mechanic.mdl
				buffer = XorStr("Ellis");	// 帽子
			else if (models->name[27] == 'a')	// models/survivors/survivor_manager.mdl
				buffer = XorStr("Louis");	// 光头
		}

		break;
	}

	// 人类特感的 ClassID 总是被识别为生还者
	if (buffer.empty())
	{
		ZombieClass_t zombie = GetZombieType();
		switch (zombie)
		{
		case ZC_SMOKER:
			buffer = XorStr("Smoker");
			break;
		case ZC_BOOMER:
			buffer = XorStr("Boomer");
			break;
		case ZC_HUNTER:
			buffer = XorStr("Hunter");
			break;
		case ZC_SPITTER:
			buffer = XorStr("Spitter");
			break;
		case ZC_JOCKEY:
			buffer = XorStr("Jockey");
			break;
		case ZC_CHARGER:
			buffer = XorStr("Charger");
			break;
		case ZC_WITCH:
			buffer = XorStr("Witch");
			break;
		case ZC_TANK:
			buffer = XorStr("Tank");
			break;
		}
	}

	return buffer;
}

ZombieClass_t CBasePlayer::GetZombieType()
{
	int classId = GetClassID();
	if (classId == ET_WITCH)
		return ZC_WITCH;
	if (classId == ET_INFECTED)
		return ZC_COMMON;
	if (classId == ET_TankRock)
		return ZC_ROCK;
	
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_zombieClass"));
	Assert_NetProp(offset);
	return static_cast<ZombieClass_t>(DECL_NETPROP_GET(byte));
}

bool CBasePlayer::IsSurvivor()
{
	int classId = GetClassID();
	return (classId == ET_CTERRORPLAYER || classId == ET_SURVIVORBOT);
}

bool CBasePlayer::IsSpecialInfected()
{
	int classId = GetClassID();
	return (classId == ET_BOOMER || classId == ET_SMOKER || classId == ET_HUNTER || classId == ET_SPITTER ||
		classId == ET_JOCKEY || classId == ET_CHARGER || classId == ET_TANK);
}

bool CBasePlayer::IsCommonInfected()
{
	return (GetClassID() == ET_INFECTED);
}

bool CBasePlayer::IsWitch()
{
	return (GetClassID() == ET_WITCH);
}

Vector CBasePlayer::GetChestOrigin()
{
	int classId = GetClassID();

	if (classId == ET_SURVIVORBOT || classId == ET_CTERRORPLAYER ||
		classId == ET_SMOKER || classId == ET_BOOMER || classId == ET_HUNTER)
		return GetHitboxOrigin(HITBOX_CHEST_PLAYER);

	if (classId == ET_TANK)
		return GetHitboxOrigin(HITBOX_CHEST_TANK);

	if (classId == ET_JOCKEY)
		return GetHitboxOrigin(HITBOX_CHEST_JOCKEY);

	if(classId == ET_SPITTER)
		return GetHitboxOrigin(HITBOX_CHEST_SPITTER);

	if (classId == ET_CHARGER)
		return GetHitboxOrigin(HITBOX_CHEST_CHARGER);

	if (classId == ET_INFECTED)
		return GetHitboxOrigin(HITBOX_CHEST_COMMON);

	if (classId == ET_WITCH)
		return GetHitboxOrigin(HITBOX_CHEST_WITCH);

	return GetAbsOrigin();
}

bool CBasePlayer::IsAlive()
{
	if (!IsValid())
		return false;
	
	static int lifeOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_lifeState"));
	static int solidOffset = GetNetPropOffset(XorStr("DT_BaseCombatCharacter"), XorStr("m_usSolidFlags"));
	static int sequenceOffset = GetNetPropOffset(XorStr("DT_BaseAnimating"), XorStr("m_nSequence"));
	static int burnOffset = GetNetPropOffset(XorStr("DT_Infected"), XorStr("m_bIsBurning"));
	Assert_NetProp(lifeOffset);
	Assert_NetProp(solidOffset);
	Assert_NetProp(sequenceOffset);
	Assert_NetProp(burnOffset);

	int entityType = GetClassID();
	if (entityType == ET_INVALID || entityType == ET_WORLD)
		return false;

	// 生还者
	if (entityType == ET_SURVIVORBOT || entityType == ET_CTERRORPLAYER)
	{
		// 生还者即使是 0 血也可以活着的 (因为还有虚血)
		return (DECL_NETPROP_GET_EX(lifeOffset, byte) == LIFE_ALIVE);
	}

	// 特感
	if (entityType == ET_BOOMER || entityType == ET_HUNTER || entityType == ET_SMOKER ||
		entityType == ET_SPITTER || entityType == ET_JOCKEY || entityType == ET_CHARGER ||
		entityType == ET_TANK)
	{
		if ((DECL_NETPROP_GET_EX(lifeOffset, byte) != LIFE_ALIVE) || GetHealth() <= 0 || IsGhost())
			return false;

		if (entityType == ET_TANK)
		{
			// Tank 在死亡前会有个濒死状态 (就是原地动不了但是还没有变尸体)
			if (DECL_NETPROP_GET_EX(sequenceOffset, WORD) > 70 || IsIncapacitated())
				return false;
		}

		return true;
	}

	// 普感
	if (entityType == ET_WITCH || entityType == ET_INFECTED)
	{
		if ((DECL_NETPROP_GET_EX(solidOffset, int) & SF_NOT_SOLID) ||
			DECL_NETPROP_GET_EX(sequenceOffset, WORD) > 305)
			return false;

		if (entityType == ET_INFECTED)
		{
			if (DECL_NETPROP_GET_EX(burnOffset, byte) != 0)
				return false;
		}

		return true;
	}

	if (entityType == ET_TankRock)
		return true;

	return false;
}

/*
int CBasePlayer::GetMoveType()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("movetype"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}
*/

Vector& CBasePlayer::GetPunch()
{
	return DECL_NETPROP_GET_EX(indexes::GetPunch, Vector);
}

int& CBasePlayer::GetTickBase()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_nTickBase"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

int& CBasePlayer::GetFlags()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_fFlags"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

CBaseEntity * CBasePlayer::GetGroundEntity()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_hGroundEntity"));
	Assert_NetProp(offset);

	CBaseHandle handle = DECL_NETPROP_GET(CBaseHandle);
	if (!handle.IsValid())
		return nullptr;

	return reinterpret_cast<CBaseWeapon*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
}

int CBasePlayer::GetWaterLevel()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_nWaterLevel"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(byte);
}

std::string CBasePlayer::GetName()
{
	player_info_t info;
	if (!g_pInterface->Engine->GetPlayerInfo(GetIndex(), &info))
		return "";

	return info.name;
}

bool CBasePlayer::CanShove()
{
	CBaseWeapon* weapon = GetActiveWeapon();
	if (weapon == nullptr)
		return false;

	return (weapon->GetSecondryAttackDelay() <= 0.0f);
}

bool CBasePlayer::CanAttack()
{
	using FnCanAttack = bool(__thiscall*)(CBasePlayer*);
	FnCanAttack fn = Utils::GetVTableFunction<FnCanAttack>(this, indexes::CanAttack);
	return fn(this);
}

bool CBasePlayer::CanBeShove()
{
	using FnCanBeShove = bool(__thiscall*)(CBasePlayer*);
	FnCanBeShove fn = Utils::GetVTableFunction<FnCanBeShove>(this, indexes::CanBeShoved);
	return fn(this);
}

bool CBasePlayer::IsReadyToShove()
{
	using FnIsReadyToShove = bool(__thiscall*)(CBasePlayer*);
	FnIsReadyToShove fn = Utils::GetVTableFunction<FnIsReadyToShove>(this, indexes::IsReadyToShove);
	return fn(this);
}

std::pair<Vector, Vector> CBasePlayer::GetBoundingBox()
{
	// static int collOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_Collision"));
	static int minOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecMins"));
	static int maxOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecMaxs"));
	
	Vector origin = GetAbsOrigin();
	return std::make_pair(origin + DECL_NETPROP_GET_EX(minOffset, Vector),
		origin + DECL_NETPROP_GET_EX(maxOffset, Vector));
}

bool CBasePlayer::IsIncapacitated()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_isIncapacitated"));
	Assert_NetProp(offset);
	return (DECL_NETPROP_GET(byte) != 0);
}

bool CBasePlayer::IsHangingFromLedge()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_isHangingFromLedge"));
	Assert_NetProp(offset);
	return (DECL_NETPROP_GET(byte) != 0);
}
