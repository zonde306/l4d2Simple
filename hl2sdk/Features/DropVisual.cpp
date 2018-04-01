#include "DropVisual.h"
#include "../hook.h"
#include "../Utils/math.h"

CVisualDrop* g_pVisualDrop = nullptr;

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

CVisualDrop::CVisualDrop() : CBaseFeatures::CBaseFeatures()
{
}

CVisualDrop::~CVisualDrop()
{
	CBaseFeatures::~CBaseFeatures();
}

void CVisualDrop::OnEnginePaint(PaintMode_t mode)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return;

	Vector myOrigin = local->GetAbsOrigin();
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	for (int i = g_pInterface->Engine->GetMaxClients() + 1; i <= maxEntity; ++i)
	{
		CBaseWeapon* entity = reinterpret_cast<CBaseWeapon*>(g_pInterface->EntList->GetClientEntity(i));
		if (entity == nullptr || entity->GetOwner() != nullptr)
			continue;

		Vector origin = entity->GetAbsOrigin();
		if (math::GetVectorDistance(myOrigin, origin, true) > m_fMaxDistance)
			continue;

		Vector screenPosition;
		if (!math::WorldToScreenEx(origin, screenPosition))
			continue;

		int classId = entity->GetClassID();
		if (classId == ET_WeaponAmmoSpawn)
		{
			DrawAmmoStack(entity, screenPosition);
			continue;
		}
		if (classId == ET_WeaponSpawn)
		{
			DrawWeaponSpawn(entity, screenPosition);
			continue;
		}
		if (classId == ET_BaseUpgradeItem)
		{
			DrawUpgradeSpawn(entity, screenPosition);
			continue;
		}
		if (classId == ET_WeaponMelee)
		{
			DrawMelee(entity, screenPosition);
			continue;
		}
		if (classId == ET_SurvivorDeathModel)
		{
			DrawSurvovorDeadbody(entity, screenPosition);
			continue;
		}
		if (classId == ET_PhysicsProp)
		{
			if (DrawCarryProps(entity, screenPosition))
				continue;
		}

		DrawOtherWeapon(entity, screenPosition);
	}
}

void CVisualDrop::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Visual Weapon/Items")))
		return;

	ImGui::Checkbox(XorStr("Tier1 Weapon"), &m_bTier1);
	IMGUI_TIPS("显示 T1 武器，例如单喷和冲锋枪。");

	ImGui::Checkbox(XorStr("Tier2 Weapon"), &m_bTier2);
	IMGUI_TIPS("显示 T2 武器，例如步枪/连喷/狙击。");

	ImGui::Checkbox(XorStr("Tier3 Weapon"), &m_bTier3);
	IMGUI_TIPS("显示 T3 武器，例如机枪和榴弹。");

	ImGui::Checkbox(XorStr("Melee Weapon"), &m_bMelee);
	IMGUI_TIPS("显示近战武器，非官方的近战武器可能显示不出来。");

	ImGui::Checkbox(XorStr("Medical"), &m_bAidKit);
	IMGUI_TIPS("显示医疗品，例如包/药/针/电击器。");

	ImGui::Checkbox(XorStr("Grenade"), &m_bGrenade);
	IMGUI_TIPS("显示投资武器，例如火瓶/土雷/胆汁。");

	ImGui::Checkbox(XorStr("Ammo/Upgrade"), &m_bAmmoUpgrade);
	IMGUI_TIPS("显示子弹和武器升级，例如子弹堆/燃烧子弹/高爆子弹/激光。");

	ImGui::Checkbox(XorStr("Carry"), &m_bCarry);
	IMGUI_TIPS("显示可携带道具，例如油桶/煤气罐/氧气瓶/烟花。");

	ImGui::Checkbox(XorStr("Projectile"), &m_bProjectile);
	IMGUI_TIPS("显示飞行物，但目前好像没效果。");

	ImGui::Checkbox(XorStr("Deadbody"), &m_bDeadbody);
	IMGUI_TIPS("显示生还者尸体，用于电击器救队友。");

	ImGui::SliderFloat(XorStr("Visual Max Distance"), &m_fMaxDistance, 500.0f, 3000.0f, XorStr("%.0f"));
	IMGUI_TIPS("显示物品的范围。");

	ImGui::TreePop();
}

void CVisualDrop::OnConfigLoading(const config_type & data)
{
	if (data.find(XorStr("itemesp_t1")) == data.end())
		return;
	
	m_bTier1 = data.at(XorStr("itemesp_t1")).at(0) == '1';
	m_bTier2 = data.at(XorStr("itemesp_t2")).at(0) == '1';
	m_bTier3 = data.at(XorStr("itemesp_t3")).at(0) == '1';
	m_bMelee = data.at(XorStr("itemesp_melee")).at(0) == '1';
	m_bAidKit = data.at(XorStr("itemesp_aid_kit")).at(0) == '1';
	m_bGrenade = data.at(XorStr("itemesp_grenade")).at(0) == '1';
	m_bAmmoUpgrade = data.at(XorStr("itemesp_upgrade_ammo")).at(0) == '1';
	m_bCarry = data.at(XorStr("itemesp_carry")).at(0) == '1';
	m_bProjectile = data.at(XorStr("itemesp_projectile")).at(0) == '1';
	m_bDeadbody = data.at(XorStr("itemesp_deadbody")).at(0) == '1';
}

void CVisualDrop::OnConfigSave(config_type & data)
{
	data[XorStr("itemesp_t1")] = std::to_string(m_bTier1);
	data[XorStr("itemesp_t2")] = std::to_string(m_bTier2);
	data[XorStr("itemesp_t3")] = std::to_string(m_bTier3);
	data[XorStr("itemesp_melee")] = std::to_string(m_bMelee);
	data[XorStr("itemesp_aid_kit")] = std::to_string(m_bAidKit);
	data[XorStr("itemesp_grenade")] = std::to_string(m_bGrenade);
	data[XorStr("itemesp_upgrade_ammo")] = std::to_string(m_bAmmoUpgrade);
	data[XorStr("itemesp_carry")] = std::to_string(m_bCarry);
	data[XorStr("itemesp_projectile")] = std::to_string(m_bProjectile);
	data[XorStr("itemesp_deadbody")] = std::to_string(m_bDeadbody);
}

void CVisualDrop::DrawWeaponSpawn(CBaseWeapon * weapon, const Vector & screen)
{
	if (!CanDrawWeapon(weapon))
		return;

	if (weapon->GetWeaponID() == Weapon_Melee)
	{
		DrawMelee(weapon, screen);
		return;
	}

	g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
		CDrawing::LAWNGREEN, true, weapon->GetWeaponName());
}

void CVisualDrop::DrawUpgradeSpawn(CBaseWeapon * weapon, const Vector & screen)
{
	if (!m_bAmmoUpgrade)
		return;

	// TODO: 获取具体的模型名来进行快速检查
	// - 或者使用更好的字符串搜索算法
	const model_t* model = weapon->GetModel();
	if (model->name[0] != 'm')
		return;

	if (model->name[7] == 'p' && model->name[13] == 't')
	{
		if (model->name[20] == 'e')
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::WHITE, true, XorStr("upgrade_explosive"));
		}
		else if (model->name[20] == 'i')
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::WHITE, true, XorStr("upgrade_incendiary"));
		}
	}
	else if (model->name[7] == 'w' && model->name[26] == 'l' && model->name[32] == 's')
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::WHITE, true, XorStr("upgrade_laser"));
	}

	/*
	if(!_stricmp(model->name, XorStr("models/props/terror/exploding_ammo.mdl")))
		g_pDrawing->DrawText(screen.x, screen.y, CDrawing::WHITE, true, XorStr("upgrade_explosive"));
	else if (!_stricmp(model->name, XorStr("models/props/terror/incendiary_ammo.mdl")))
		g_pDrawing->DrawText(screen.x, screen.y, CDrawing::WHITE, true, XorStr("upgrade_incendiary"));
	else if (!_stricmp(model->name, XorStr("models/w_models/Weapons/w_laser_sights.mdl")))
		g_pDrawing->DrawText(screen.x, screen.y, CDrawing::WHITE, true, XorStr("upgrade_laser"));
	*/
}

void CVisualDrop::DrawAmmoStack(CBaseWeapon * weapon, const Vector & screen)
{
	if (!m_bAmmoUpgrade)
		return;

	const model_t* model = weapon->GetModel();
	if (!_stricmp(model->name, XorStr("models/props/terror/ammo_stack.mdl")))
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::WHITE, true, XorStr("ammo_stack"));
	}
	else if (!_stricmp(model->name, XorStr("models/props_unique/spawn_apartment/coffeeammo.mdl")))
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::WHITE, true, XorStr("ammo_coffee"));
	}
	else
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::WHITE, true, XorStr("ammo_unknown"));
	}
}

void CVisualDrop::DrawMelee(CBaseWeapon * weapon, const Vector & screen)
{
	if (!CanDrawWeapon(weapon))
		return;

	const model_t* model = weapon->GetModel();
	// if(strstr(model->name, XorStr("models/weapons/melee/w_")) == model->name)
	if (model->name[0] == 'm' && model->name[7] == 'w' && model->name[15] == 'm' &&
		model->name[21] == 'w' && model->name[22] == '_' && strrchr(model->name, '.') != nullptr)
	{
		std::string melee = model->name;
		melee = melee.substr(23, melee.rfind('.') - 23);
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::GREEN, true, melee.c_str());
	}
}

void CVisualDrop::DrawSurvovorDeadbody(CBaseWeapon * weapon, const Vector & screen)
{
	if (!m_bDeadbody)
		return;

	const model_t* model = weapon->GetModel();
	if (model->name[0] != 'm' || model->name[7] != 's' || model->name[17] != 's')
		return;

	if (model->name[26] == 'g')				// models/survivors/survivor_gambler.mdl
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::PURPLE, true, XorStr("Nick"));		// 西装
	}
	else if (model->name[26] == 'p')		// models/survivors/survivor_producer.mdl
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::PURPLE, true, XorStr("Rochelle"));	// 黑妹
	}
	else if (model->name[26] == 'c')		// models/survivors/survivor_coach.mdl
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::PURPLE, true, XorStr("Coach"));		// 黑胖
	}
	else if (model->name[26] == 'n')		// models/survivors/survivor_namvet.mdl
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::PURPLE, true, XorStr("Bill"));		// 老头
	}
	else if (model->name[26] == 't')		// models/survivors/survivor_teenangst.mdl
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::PURPLE, true, XorStr("Zoey"));		// 萌妹
	}
	else if (model->name[26] == 'b')		// models/survivors/survivor_biker.mdl
	{
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::PURPLE, true, XorStr("Francis"));	// 背心
	}
	else if (model->name[26] == 'm')
	{
		if (model->name[27] == 'e')		// models/survivors/survivor_mechanic.mdl
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::PURPLE, true, XorStr("Ellis"));	// 帽子
		}
		else if (model->name[27] == 'a')	// models/survivors/survivor_manager.mdl
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::PURPLE, true, XorStr("Louis"));	// 光头
		}
	}
}

void CVisualDrop::DrawOtherWeapon(CBaseWeapon * weapon, const Vector & screen)
{
	int classId = weapon->GetClassID();
	if (classId == ET_MolotovProjectile)
	{
		if (m_bProjectile)
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::BLUE, true, XorStr("molotov_projectile"));
		}
		return;
	}
	if (classId == ET_PipeBombProjectile)
	{
		if (m_bProjectile)
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::BLUE, true, XorStr("pipiebomb_projectile"));
		}
		return;
	}
	if (classId == ET_VomitJarProjectile)
	{
		if (m_bProjectile)
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::BLUE, true, XorStr("vomitjar_projectile"));
		}
		return;
	}
	if (classId == ET_TankRock)
	{
		if (m_bProjectile)
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::GREEN, true, XorStr("tank_rock"));
		}
		return;
	}
	if (classId == ET_SpitterProjectile)
	{
		if (m_bProjectile)
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::GREEN, true, XorStr("spit_projectile"));
		}
		return;
	}
	if (classId == ET_GrenadeProjectile)
	{
		if (m_bProjectile)
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::GREEN, true, XorStr("grenade_projectile"));
		}
		return;
	}

	if (classId == ET_WeaponFirstAidKit || classId == ET_WeaponDefibrillator ||
		classId == ET_WeaponPainPills || classId == ET_WeaponAdrenaline ||
		classId == ET_WeaponPipeBomb || classId == ET_WeaponMolotov ||
		classId == ET_WeaponVomitjar || classId == ET_WeaponMagnum ||
		classId == ET_WeaponExplosive || classId == ET_WeaponIncendiary ||
		classId == ET_WeaponGascan || classId == ET_WeaponCola || classId == ET_WeaponGnome)
	{
		DrawWeaponSpawn(weapon, screen);
		return;
	}
}

bool CVisualDrop::DrawCarryProps(CBaseWeapon * weapon, const Vector & screen)
{
	if(!m_bCarry)
		return false;

	const model_t* model = weapon->GetModel();
	if (model->name[0] != 'm' || model->name[7] != 'p')
		return false;

	if (model->name[13] == 'e' && model->name[23] == 'o')
	{
		// models/props_equipment/oxygentank01.mdl
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::LAWNGREEN, true, XorStr("oxygentank"));
		return true;
	}

	if (model->name[13] != 'j')
		return false;

	if (model->name[18] == 'p')
	{
		// models/props_junk/propanecanister001a.mdl
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::LAWNGREEN, true, XorStr("propanetank"));
		return true;
	}

	if (model->name[18] == 'e')
	{
		// models/props_junk/explosive_box001.mdl
		g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
			CDrawing::LAWNGREEN, true, XorStr("firework"));
		return true;
	}

	if (model->name[18] == 'g')
	{
		// models/props_junk/gascan001a.mdl
		if (weapon->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSkin")) > 0)
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::LAWNGREEN, true, XorStr("gascan_scavenge"));
		}
		else
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				CDrawing::LAWNGREEN, true, XorStr("gascan"));
		}

		return true;
	}

	return false;
}

bool CVisualDrop::CanDrawWeapon(CBaseWeapon * weapon)
{
	if (weapon == nullptr)
		return false;
	
	int weaponId = weapon->GetWeaponID();
	
	if (IsWeaponT1(weaponId))
		return m_bTier1;

	if (IsWeaponT2(weaponId))
		return m_bTier2;

	if (IsWeaponT3(weaponId))
		return m_bTier3;

	if (IsMedical(weaponId))
		return m_bAidKit;

	if (IsMelee(weaponId))
		return m_bMelee;

	if (IsAmmoPack(weaponId))
		return m_bAmmoUpgrade;

	if (IsCarryWeapon(weaponId))
		return m_bCarry;

	if (IsGrenadeWeapon(weaponId))
		return m_bGrenade;

	return false;
}
