#include "baseweapon.h"
#include "baseplayer.h"
#include "../interfaces.h"
#include "../hook.h"
#include "../indexes.h"
#include "../../l4d2Simple2/utils.h"

#define SIG_WEAPON_ID_TO_ALIAS		XorStr("55 8B EC 8B 45 08 83 F8 37")
#define SIG_LOOKUP_WEAPON_SLOT		XorStr("55 8B EC 8B 45 08 83 EC 08 85 C0")
#define SIG_GET_INVALID_SLOT		XorStr("B8 ? ? ? ? C3")
#define SIG_GET_WEAPON_INFO			XorStr("55 8B EC 66 8B 45 08 66 3B 05")

float& CBaseWeapon::GetSpread()
{
	return *reinterpret_cast<float*>(reinterpret_cast<DWORD>(this) + indexes::GetSpread);
}

FileWeaponInfo_t * CBaseWeapon::GetWeaponData()
{
	using FnGetSlot = uint16_t(__cdecl*)(const char*);
	using FnGetInvalidSlot = uint16_t(__cdecl*)();
	using FnGetData = FileWeaponInfo_t * (__cdecl*)(uint16_t);

	static FnGetSlot LookupWeaponInfoSlot = nullptr;
	static FnGetInvalidSlot GetInvalidWeaponInfoHandle = nullptr;
	static FnGetData GetFileWeaponInfoFromHandle = nullptr;

	if (LookupWeaponInfoSlot == nullptr)
	{
		LookupWeaponInfoSlot = reinterpret_cast<FnGetSlot>(Utils::FindPattern(XorStr("client.dll"), SIG_LOOKUP_WEAPON_SLOT));
		GetInvalidWeaponInfoHandle = reinterpret_cast<FnGetInvalidSlot>(Utils::FindPattern(XorStr("client.dll"), SIG_GET_INVALID_SLOT));
		GetFileWeaponInfoFromHandle = reinterpret_cast<FnGetData>(Utils::FindPattern(XorStr("client.dll"), SIG_GET_WEAPON_INFO));
	}

	char buffer[255];
	sprintf_s(buffer, "weapon_%s", GetWeaponName());
	uint16_t slot = LookupWeaponInfoSlot(buffer);
	if (slot == GetInvalidWeaponInfoHandle())
		return nullptr;

	return GetFileWeaponInfoFromHandle(slot);
}

const char * CBaseWeapon::GetWeaponName()
{
	using Fn = const char*(__cdecl*)(int);
	static Fn WeaponIdToAlias = nullptr;

	if (WeaponIdToAlias == nullptr)
		WeaponIdToAlias = reinterpret_cast<Fn>(Utils::FindPattern(XorStr("client.dll"), SIG_WEAPON_ID_TO_ALIAS));

	return WeaponIdToAlias(GetWeaponID());
}

int CBaseWeapon::GetWeaponID()
{
	if (GetClassID() == ET_WeaponSpawn)
	{
		static int offset = GetNetPropOffset(XorStr("DT_WeaponSpawn"), XorStr("m_weaponID"));
		Assert_NetProp(offset);
		return DECL_NETPROP_GET(byte);
	}
	
	using Fn = int(__thiscall*)(CBaseWeapon*);
	return Utils::GetVTableFunction<Fn>(this, indexes::GetWeaponId)(this);
}

CBasePlayer * CBaseWeapon::GetOwner()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_hOwnerEntity"));
	// static int offset = GetNetPropOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_hOwner"));
	Assert_NetProp(offset);

	CBaseHandle handle = DECL_NETPROP_GET(CBaseHandle);
	if (!handle.IsValid())
		return nullptr;

	return reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
}

int CBaseWeapon::GetClip()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_iClip1"));
	Assert_NetProp(offset);

	// 弹夹可以是负数(有符号)的
	return DECL_NETPROP_GET(char);
}

bool CBaseWeapon::IsReloading()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_bInReload"));
	Assert_NetProp(offset);
	return (DECL_NETPROP_GET(byte) != 0);
}

int CBaseWeapon::GetAmmoType()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_iPrimaryAmmoType"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(byte);
}

int CBaseWeapon::GetAmmo()
{
	CBasePlayer* player = GetOwner();
	if (player == nullptr)
	{
		static int offset = GetNetPropOffset(XorStr("DT_TerrorWeapon"), XorStr("m_iExtraPrimaryAmmo"));
		Assert_NetProp(offset);
		return DECL_NETPROP_GET(int);
	}

	return player->GetAmmo(GetAmmoType());
}

float CBaseWeapon::GetNextPrimaryAttack()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_flNextPrimaryAttack"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(float);
}

float CBaseWeapon::GetPrimaryAttackDelay()
{
	float interval = GetNextPrimaryAttack() - g_pClientPrediction->GetServerTime();
	return max(interval, 0.0f);
}

float CBaseWeapon::GetNextSecondryAttack()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_flNextSecondaryAttack"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(float);
}

float CBaseWeapon::GetSecondryAttackDelay()
{
	float interval = GetNextSecondryAttack() - g_pClientPrediction->GetServerTime();
	return max(interval, 0.0f);
}

bool CBaseWeapon::CanFire()
{
	// 不需要弹药的武器弹夹永远为 -1
	if (GetClip() == 0)
		return false;

	return (GetPrimaryAttackDelay() <= 0.0f);
}
