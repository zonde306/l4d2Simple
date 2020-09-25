#include "baseweapon.h"
#include "baseplayer.h"
#include "../interfaces.h"
#include "../hook.h"
#include "../indexes.h"
#include "../../l4d2Simple2/utils.h"

#define SIG_WEAPON_ID_TO_ALIAS		XorStr("55 8B EC 8B 45 08 83 F8 37")
#define SIG_LOOKUP_WEAPON_SLOT		XorStr("55 8B EC 8B 45 08 83 EC 08 85 C0")
#define SIG_GET_INVALID_SLOT		XorStr("E8 ? ? ? ? 66 3B F0")
#define SIG_GET_WEAPON_INFO			XorStr("55 8B EC 66 8B 45 08 66 3B 05")
#define SIG_GET_WEAPON_DATA			XorStr("0F B7 ? ? ? ? ? 50 E8 ? ? ? ? 83 C4 ? C3")
#define SIG_UPDATE_WEAPON_SPREAD	XorStr("53 8B DC 83 EC ? 83 E4 ? 83 C4 ? 55 8B 6B ? 89 6C ? ? 8B EC 83 EC ? 56 57 8B F9 E8")

#define IsSubMachinegun(_id)		(_id == WeaponId_SubMachinegun || _id == WeaponId_Silenced || _id == WeaponId_MP5)
#define IsShotgun(_id)				(_id == WeaponId_PumpShotgun || _id == WeaponId_Chrome || _id == WeaponId_AutoShotgun || _id == WeaponId_SPAS)
#define IsAssaultRifle(_id)			(_id == WeaponId_AssaultRifle || _id == WeaponId_AK47 || _id == WeaponId_Desert || _id == WeaponId_SG552 || _id == WeaponId_M60)
#define IsSniper(_id)				(_id == WeaponId_SniperRifle || _id == WeaponId_Military || _id == WeaponId_Scout || _id == WeaponId_AWP)
#define IsPistol(_id)				(_id == WeaponId_Pistol || _id == WeaponId_MagnumPistol)
#define IsMedical(_id)				(_id == WeaponId_FirstAidKit || _id == WeaponId_ItemDefibrillator || _id == WeaponId_PainPills || _id == WeaponId_Adrenaline)
#define IsAmmoPack(_id)				(_id == WeaponId_ItemAmmoPack || _id == WeaponId_ItemUpgradePackExplosive || _id == WeaponId_ItemUpgradePackIncendiary)
#define IsMelee(_id)				(_id == WeaponId_TerrorMeleeWeapon || _id == WeaponId_Chainsaw)
#define IsWeaponT1(_id)				(IsSubMachinegun(_id) || _id == WeaponId_PumpShotgun || _id == WeaponId_Chrome || _id == WeaponId_Pistol)
#define IsWeaponT2(_id)				(_id == WeaponId_AutoShotgun || _id == WeaponId_SPAS || _id == WeaponId_AssaultRifle || _id == WeaponId_AK47 || _id == WeaponId_Desert || _id == WeaponId_SG552 || _id == WeaponId_MagnumPistol || IsSniper(_id))
#define IsWeaponT3(_id)				(_id == WeaponId_M60 || _id == WeaponId_GrenadeLauncher)
#define IsNotGunWeapon(_id)			(IsGrenadeWeapon(_id) || IsMedicalWeapon(_id) || IsPillsWeapon(_id) || IsCarryWeapon(_id) || _id == Weapon_Melee || _id == Weapon_Chainsaw)
#define IsGunWeapon(_id)			(IsSubMachinegun(_id) || IsShotgun(_id) || IsAssaultRifle(_id) || IsSniper(_id) || IsPistol(_id))
#define IsGrenadeWeapon(_id)		(_id == Weapon_Molotov || _id == Weapon_PipeBomb || _id == Weapon_Vomitjar)
#define IsMedicalWeapon(_id)		(_id == Weapon_FirstAidKit || _id == Weapon_Defibrillator || _id == Weapon_FireAmmo || _id == Weapon_ExplodeAmmo)
#define IsPillsWeapon(_id)			(_id == Weapon_PainPills || _id == Weapon_Adrenaline)
#define IsCarryWeapon(_id)			(_id == Weapon_Gascan || _id == Weapon_Fireworkcrate || _id == Weapon_Propanetank || _id == Weapon_Oxygentank || _id == Weapon_Gnome || _id == Weapon_Cola)

float& CBaseWeapon::GetSpread()
{
	return *reinterpret_cast<float*>(reinterpret_cast<DWORD>(this) + indexes::GetSpread);
}

FileWeaponInfo_t * CBaseWeapon::GetWeaponInfo()
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
		GetInvalidWeaponInfoHandle = reinterpret_cast<FnGetInvalidSlot>(Utils::CalcInstAddress(Utils::FindPattern(XorStr("client.dll"), SIG_GET_INVALID_SLOT)));
		GetFileWeaponInfoFromHandle = reinterpret_cast<FnGetData>(Utils::FindPattern(XorStr("client.dll"), SIG_GET_WEAPON_INFO));
	}

	char buffer[255];
	sprintf_s(buffer, "weapon_%s", GetWeaponName());
	uint16_t slot = LookupWeaponInfoSlot(buffer);
	if (slot == GetInvalidWeaponInfoHandle())
		return nullptr;

	return GetFileWeaponInfoFromHandle(slot);
}

FileWeaponInfo_t * CBaseWeapon::GetWeaponData()
{
	using Fn = FileWeaponInfo_t*(__thiscall*)(LPVOID);
	static Fn GetCSWpnData = reinterpret_cast<Fn>(Utils::FindPattern(XorStr("client.dll"), SIG_GET_WEAPON_DATA));
	return GetCSWpnData(this);
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
	return Utils::GetVTableFunction<Fn>(this, indexes::GetWeaponID)(this);
}

int CBaseWeapon::GetWeaponId()
{
	return *reinterpret_cast<int*>(reinterpret_cast<DWORD>(this) + indexes::GetWeaponId);
}

void CBaseWeapon::UpdateSpread()
{
	using Fn = void(__thiscall*)(CBaseWeapon*);
	static Fn UpdateMaxSpread = reinterpret_cast<Fn>(Utils::FindPattern(XorStr("client.dll"), SIG_UPDATE_WEAPON_SPREAD));
	return UpdateMaxSpread(this);
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
	// 部分武器拥有弹药设定，但是它并不是枪
	if (GetClip() == 0)
		return false;

	int weaponId = GetWeaponID();

	// 霰弹枪在填装的时候如果弹夹内有子弹也可以开枪的
	if (IsShotgun(weaponId) && IsReloading())
		return true;

	return (GetPrimaryAttackDelay() <= 0.0f);
}

bool CBaseWeapon::IsFireGun()
{
	if(GetWeaponData()->iMaxClip1 <= 0)
		return false;

	int weaponId = GetWeaponID();
	return IsGunWeapon(weaponId);
}

bool CBaseWeapon::CanShove()
{
	return (GetSecondryAttackDelay() <= 0.0f);
}
