#include "baseweapon.h"
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

	return WeaponIdToAlias(GetWeaponId());
}

int CBaseWeapon::GetWeaponId()
{
	using Fn = int(__thiscall*)(CBaseWeapon*);
	return Utils::GetVTableFunction<Fn>(this, indexes::GetWeaponId)(this);
}
