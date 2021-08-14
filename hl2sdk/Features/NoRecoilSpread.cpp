#include "NoRecoilSpread.h"
#include "../Utils/math.h"
#include "../hook.h"
#include "../../l4d2Simple2/config.h"
#include "../Structs/MRecipientFilter.h"

CViewManager* g_pViewManager = nullptr;

#define IsSingleWeapon(_id)			(_id == Weapon_Pistol || _id == Weapon_ShotgunPump || _id == Weapon_ShotgunAuto || _id == Weapon_SniperHunting || _id == Weapon_ShotgunChrome || _id == Weapon_SniperMilitary || _id == Weapon_ShotgunSpas || _id == Weapon_PistolMagnum || _id == Weapon_SniperAWP || _id == Weapon_SniperScout)
#define NUM_NEW_COMMAND_BITS		4
#define MAX_NEW_COMMANDS			((1 << NUM_NEW_COMMAND_BITS)-1)
#define IsShotgun(_id)				(_id == WeaponId_PumpShotgun || _id == WeaponId_Chrome || _id == WeaponId_AutoShotgun || _id == WeaponId_SPAS)

CViewManager::CViewManager() : CBaseFeatures::CBaseFeatures()
{
	m_vecAngles.Invalidate();
	m_pEventListener = new CVM_ShotgunSound();
	//g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("weapon_fire"), false);
	//g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("bullet_impact"), false);
	g_pClientHook->m_ProtectedEventListeners.insert(m_pEventListener);
}

CViewManager::~CViewManager()
{
	g_pInterface->GameEvent->RemoveListener(m_pEventListener);
	delete m_pEventListener;

	CBaseFeatures::~CBaseFeatures();
}

void CViewManager::OnCreateMove(CUserCmd * cmd, bool * bSendPacket)
{
	m_pEventListener->m_bIsGameTick = false;
	if (m_bMenuOpen)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive() || local->IsHangingFromLedge() ||
		local->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedGun")) ||
		local->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedWeapon")))
		return;

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (weapon == nullptr)
		return;

	if (m_bRapidFire)
		RunRapidFire(cmd, local, weapon);

	bool canFire = ((cmd->buttons & IN_ATTACK2) ? weapon->CanShove() : weapon->CanFire());
	bool isGun = weapon->IsFireGun();
	m_bHasFiring = ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2));
	RunSilentAngles(cmd, bSendPacket, canFire);

	if (isGun && canFire && m_bHasFiring)
	{
		RunNoRecoilSpread(cmd, weapon, bSendPacket);
		m_bLastFired = !!(cmd->buttons & IN_ATTACK);
		m_vecLastFiredAngles = cmd->viewangles;
		m_iKeepTicks = 0;
	}
	else if (isGun && !canFire && m_bLastFired && (m_bNoRecoil || m_bNoSpread))
	{
		SmoothAngles(cmd, bSendPacket);
		cmd->buttons |= IN_ATTACK;
		m_bLastFired = false;
	}

	m_bHasSilent = !(*bSendPacket);
	m_vecViewAngles = cmd->viewangles;

	if (!(*bSendPacket))
	{
		++m_iPacketBlocked;

		// 修复命令被丢弃的问题
		// bSendPacket 设置为 false 超过上限会导致命令被丢弃
		if (m_iPacketBlocked > MAX_NEW_COMMANDS)
		{
			*bSendPacket = true;
			m_iPacketBlocked = 0;
			Utils::log(XorStr("Warring: choked commands out of range"));
		}
	}

	if (m_bFakeLag && (GetAsyncKeyState(VK_CAPITAL) & 0x8000))
		*bSendPacket = false;

	if (*bSendPacket && m_iPacketBlocked != 0)
		m_iPacketBlocked = 0;

	// 修复移动不正确
	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);
	math::CorrectMovement(viewAngles, cmd, cmd->forwardmove, cmd->sidemove);

	if (m_bFakeAngleBug && canFire && m_bHasFiring)
	{
		static int lastChocked = 0;
		if (lastChocked >= 5)
		{
			*bSendPacket = true;
			lastChocked = 0;
		}
		else
		{
			*bSendPacket = false;
			lastChocked += 1;
			cmd->viewangles.x = cmd->viewangles.y = NAN;
		}
	}
}

void CViewManager::OnFrameStageNotify(ClientFrameStage_t stage)
{
	if (m_bMenuOpen)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		//Vector& ViewPunch = local->GetPunch();
		Vector& m_vecPunchAngle = local->GetNetProp<Vector>(XorStr("DT_TerrorPlayer"), XorStr("m_vecPunchAngle"));
		Vector& m_vecPunchAngleVel = local->GetNetProp<Vector>(XorStr("DT_TerrorPlayer"), XorStr("m_vecPunchAngleVel"));

		// m_vecPunchAngle or m_vecPunchAngleVel
		m_vecPunch = m_vecPunchAngle + m_vecPunchAngleVel;

		if (m_bNoVisRecoil)
		{
			//ViewPunch.x = 0.0f;
			//ViewPunch.y = 0.0f;
			m_vecPunchAngle.x = 0.0f;
			m_vecPunchAngle.y = 0.0f;
			m_vecPunchAngleVel.x = 0.0f;
			m_vecPunchAngleVel.y = 0.0f;
		}
	}

	if (stage == FRAME_RENDER_START && m_bRealAngles && m_bHasFiring)
	{
		Vector eyeOrigin = local->GetEyePosition();
		Vector aimOrigin = eyeOrigin + m_vecViewAngles.Forward().Scale(1000.0f);
		g_pInterface->DebugOverlay->AddLineOverlay(eyeOrigin, aimOrigin, 255, 255, 255, false, 1.0f);
	}
}

void CViewManager::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Aim Helper")))
		return;

	ImGui::Checkbox(XorStr("No Recoil"), &m_bNoRecoil);
	IMGUI_TIPS("无后坐力。");

	ImGui::SliderFloat(XorStr("No Recoil Factor"), &m_fRecoilFactor, 0.0f, 1.0f, ("%.2f"));
	IMGUI_TIPS("无后坐力效率。");

	ImGui::Checkbox(XorStr("No Visual Recoil"), &m_bNoVisRecoil);
	IMGUI_TIPS("无后屏幕坐力。");

	ImGui::Checkbox(XorStr("No Spread"), &m_bNoSpread);
	IMGUI_TIPS("子弹无扩散（提升射击精度）。");

	ImGui::SliderFloat(XorStr("No Spread Factor"), &m_fSpreadFactor, 0.0f, 1.0f, ("%.2f"));
	IMGUI_TIPS("子弹无扩散效率。");

	ImGui::Checkbox(XorStr("Rapid Fire"), &m_bRapidFire);
	IMGUI_TIPS("手枪/狙击枪连射（按住开枪键）。");

	ImGui::Checkbox(XorStr("Spread/Recoil Silent"), &m_bSilentNoSpread);
	IMGUI_TIPS("无后坐力/子弹无扩散 防止被观察者看出来。");

	ImGui::Checkbox(XorStr("Real Angles"), &m_bRealAngles);
	IMGUI_TIPS("调试用，显示真正的角度。");

	ImGui::Checkbox(XorStr("Fake Lag"), &m_bFakeLag);
	IMGUI_TIPS("丢包模拟 按住 大小写锁定(Caps Lock) 生效。");

	ImGui::Checkbox(XorStr("chocked exploit"), &m_bFakeAngleBug);
	IMGUI_TIPS("炸服用，开启后按住鼠标左键(开枪)启动。用后记得手动 disconnect，否则会卡住。\n最好是拿枪有子弹时用，否则可能会导致游戏无响应。");
	
	ImGui::Checkbox(XorStr("shotgun sound"), &(m_pEventListener->m_bShotgunSound));
	IMGUI_TIPS("第三人称使用霰弹枪播放开枪声音。");

	ImGui::SliderInt(XorStr("Smooth Ticks"), &m_iSmoothTicks, 0, 10);
	IMGUI_TIPS("平滑，用于避免被 liac 检测");

	ImGui::TreePop();
}

void CViewManager::OnConfigLoading(CProfile& cfg)
{
	const std::string mainKeys = XorStr("AimHelper");
	
	m_bNoRecoil = cfg.GetBoolean(mainKeys, XorStr("aimhelper_recoil"), m_bNoRecoil);
	m_bNoVisRecoil = cfg.GetBoolean(mainKeys, XorStr("aimhelper_visual_recoil"), m_bNoVisRecoil);
	m_bNoSpread = cfg.GetBoolean(mainKeys, XorStr("aimhelper_spread"), m_bNoSpread);
	m_bRapidFire = cfg.GetBoolean(mainKeys, XorStr("aimhelper_rapid_fire"), m_bRapidFire);
	m_bSilentNoSpread = cfg.GetBoolean(mainKeys, XorStr("aimhelper_silent"), m_bSilentNoSpread);
	m_bRealAngles = cfg.GetBoolean(mainKeys, XorStr("aimhelper_real_angles"), m_bRealAngles);
	m_bFakeLag = cfg.GetBoolean(mainKeys, XorStr("aimhelper_fake_lag"), m_bFakeLag);
	m_pEventListener->m_bShotgunSound = cfg.GetBoolean(mainKeys, XorStr("aimhelper_shotgun"), m_pEventListener->m_bShotgunSound);
	m_fSpreadFactor = cfg.GetFloat(mainKeys, XorStr("aimhelper_spread_factor"), m_fSpreadFactor);
	m_fRecoilFactor = cfg.GetFloat(mainKeys, XorStr("aimhelper_recoil_factor"), m_fRecoilFactor);
	m_iSmoothTicks = cfg.GetInteger(mainKeys, XorStr("aimhelper_smooth_ticks"), m_iSmoothTicks);
}

void CViewManager::OnConfigSave(CProfile& cfg)
{
	const std::string mainKeys = XorStr("AimHelper");
	
	cfg.SetValue(mainKeys, XorStr("aimhelper_recoil"), m_bNoRecoil);
	cfg.SetValue(mainKeys, XorStr("aimhelper_visual_recoil"), m_bNoVisRecoil);
	cfg.SetValue(mainKeys, XorStr("aimhelper_spread"), m_bNoSpread);
	cfg.SetValue(mainKeys, XorStr("aimhelper_rapid_fire"), m_bRapidFire);
	cfg.SetValue(mainKeys, XorStr("aimhelper_silent"), m_bSilentNoSpread);
	cfg.SetValue(mainKeys, XorStr("aimhelper_real_angles"), m_bRealAngles);
	cfg.SetValue(mainKeys, XorStr("aimhelper_fake_lag"), m_bFakeLag);
	cfg.SetValue(mainKeys, XorStr("aimhelper_shotgun"), m_pEventListener->m_bShotgunSound);
	cfg.SetValue(mainKeys, XorStr("aimhelper_spread_factor"), m_fSpreadFactor);
	cfg.SetValue(mainKeys, XorStr("aimhelper_recoil_factor"), m_fRecoilFactor);
	cfg.SetValue(mainKeys, XorStr("aimhelper_smooth_ticks"), m_iSmoothTicks);
}

void CViewManager::OnConnect()
{
	m_bFakeAngleBug = false;
}

void CViewManager::OnDisconnect()
{
	m_bFakeAngleBug = false;
}

void CViewManager::OnGameEventClient(IGameEvent* event)
{
	std::string_view name = event->GetName();
	if (name == XorStr("weapon_fire") || name == XorStr("bullet_impact"))
		m_pEventListener->FireGameEvent(event);
}

void CViewManager::OnGameEvent(IGameEvent* event, bool dontBroadcast)
{
	std::string_view name = event->GetName();
	if (name == XorStr("weapon_fire") || name == XorStr("bullet_impact"))
		m_pEventListener->FireGameEvent(event);
}

void CViewManager::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bRealAngles || !m_bHasFiring || m_bMenuOpen)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	Vector screen;
	Vector aimOrigin = local->GetEyePosition() + m_vecViewAngles.Forward().Scale(100.0f);
	if (!math::WorldToScreenEx(aimOrigin, screen))
		return;

	D3DCOLOR color = CDrawing::ORANGE;
	if (m_bHasSilent)
		color = CDrawing::GREEN;

	g_pDrawing->DrawLine(static_cast<int>(screen.x - 10), static_cast<int>(screen.y - 10),
		static_cast<int>(screen.x + 10), static_cast<int>(screen.y + 10), CDrawing::YELLOW);
	g_pDrawing->DrawLine(static_cast<int>(screen.x + 10), static_cast<int>(screen.y - 10),
		static_cast<int>(screen.x - 10), static_cast<int>(screen.y + 10), CDrawing::YELLOW);

	Vector& m_vecPunchAngle = local->GetNetProp<Vector>(XorStr("DT_TerrorPlayer"), XorStr("m_vecPunchAngle"));
	Vector& m_vecPunchAngleVel = local->GetNetProp<Vector>(XorStr("DT_TerrorPlayer"), XorStr("m_vecPunchAngleVel"));
	g_pDrawing->DrawText(screen.x / 2, screen.y / 3, CDrawing::RED, true, XorStr("m_vecPunchAngle = %.2f, %.2f, %.2f"), m_vecPunchAngle.x, m_vecPunchAngle.y, m_vecPunchAngle.z);
	g_pDrawing->DrawText(screen.x / 2, screen.y / 3 + 16, CDrawing::RED, true, XorStr("m_vecPunchAngleVel = %.2f, %.2f, %.2f"), m_vecPunchAngleVel.x, m_vecPunchAngleVel.y, m_vecPunchAngleVel.z);
}

bool CViewManager::ApplySilentAngles(const QAngle& viewAngles, int ticks)
{
	if (m_bSilentFire || m_iSilentTicks > -1 || ticks < 1)
		return false;

	m_iSilentTicks = ticks;
	m_vecSilentAngles = viewAngles;
	return true;
}

bool CViewManager::ApplySilentFire(const QAngle& viewAngles)
{
	if (m_bSilentFire || m_iSilentTicks > -1)
		return false;

	m_bSilentFire = true;
	m_vecSilentAngles = viewAngles;
	return true;
}

void CViewManager::RunNoRecoilSpread(CUserCmd * cmd, CBaseWeapon* weapon, bool* bSendPacket)
{
	auto spread = g_pClientPrediction->GetWeaponSpread(cmd->random_seed, weapon);
	m_vecSpread.x = spread.first;
	m_vecSpread.y = spread.second;
	// m_vecSpread.z = 0.0f;

	// 被普感锤也算是后坐力
	if (m_bNoRecoil)
		RemoveRecoil(cmd);

	// 射击才有扩散
	if (m_bNoSpread && (cmd->buttons & IN_ATTACK))
		RemoveSpread(cmd);

	if (m_bSilentNoSpread)
		*bSendPacket = false;
}

bool CViewManager::StartSilent(CUserCmd * cmd)
{
	if (m_vecAngles.IsValid())
		return false;

	m_vecAngles = cmd->viewangles;
	m_fSideMove = cmd->sidemove;
	m_fForwardMove = cmd->forwardmove;
	m_fUpMove = cmd->upmove;

	return true;
}

bool CViewManager::FinishSilent(CUserCmd * cmd)
{
	if (!m_vecAngles.IsValid())
		return false;

	cmd->viewangles = m_vecAngles;
	cmd->sidemove = m_fSideMove;
	cmd->forwardmove = m_fForwardMove;
	cmd->upmove = m_fUpMove;

	m_vecAngles.Invalidate();
	return true;
}

void CViewManager::RemoveSpread(CUserCmd * cmd)
{
	if (!(cmd->buttons & IN_ATTACK))
		return;
	
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (weapon == nullptr/* || !weapon->CanFire()*/)
		return;

	cmd->viewangles.x -= m_vecSpread.x * m_fSpreadFactor;
	cmd->viewangles.y -= m_vecSpread.y * m_fSpreadFactor;
}

void CViewManager::RemoveRecoil(CUserCmd * cmd)
{
	if (!(cmd->buttons & IN_ATTACK))
		return;
	
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	/*
	Vector punch = player->GetPunch();
	cmd->viewangles.x -= punch.x;
	cmd->viewangles.y -= punch.y;
	*/

	m_vecPunchAngle = player->GetNetProp<Vector>(XorStr("DT_TerrorPlayer"), XorStr("m_vecPunchAngle"));
	// Vector& m_vecPunchAngleVel = player->GetNetProp<Vector>(XorStr("DT_TerrorPlayer"), XorStr("m_vecPunchAngleVel"));
	cmd->viewangles.x -= m_vecPunchAngle.x * m_fRecoilFactor;
	cmd->viewangles.y -= m_vecPunchAngle.y * m_fRecoilFactor;
}

void CViewManager::SmoothAngles(CUserCmd* cmd, bool* bSendPacket)
{
	if (m_iKeepTicks >= m_iSmoothTicks)
		return;

	++m_iKeepTicks;

	if (m_iKeepTicks <= 1)
	{
		cmd->viewangles = m_vecLastFiredAngles;
	}
	else if (m_iKeepTicks > 1)
	{
		QAngle view;
		g_pInterface->Engine->GetViewAngles(view);

		cmd->viewangles = math::Lerp(m_vecLastFiredAngles, view, static_cast<float>(m_iKeepTicks - 1) / static_cast<float>(m_iSmoothTicks - 1));
	}

	if (m_bSilentNoSpread)
		*bSendPacket = false;
}

void CViewManager::RunRapidFire(CUserCmd* cmd, CBasePlayer* local, CBaseWeapon* weapon)
{
	if (!(cmd->buttons & IN_ATTACK)/* || IsUsingMinigun(local)*/)
		return;

	if (weapon->IsReloading())
		return;

	if (weapon->GetNetProp<float>(XorStr("DT_BaseCombatWeapon"), XorStr("m_flCycle")) > 0.0f)
		return;

	// weapon->GetNetProp<BYTE>(XorStr("DT_BaseCombatWeapon"), XorStr("m_isHoldingFireButton")) = 0;

	int weaponId = weapon->GetWeaponID();
	if ((local->GetTeam() == 3 || IsSingleWeapon(weaponId)) &&
		!m_bRapidIgnore && weapon->GetPrimaryAttackDelay() <= 0.0f)
	{
		cmd->buttons &= ~IN_ATTACK;
		m_bRapidIgnore = true;
	}
	else
	{
		m_bRapidIgnore = false;
	}
}

void CViewManager::RunSilentAngles(CUserCmd* cmd, bool* bSendPacket, bool canFire)
{
	if (!(*bSendPacket))
		return;

	if (m_bSilentFire)
	{
		if (canFire && m_bHasFiring && m_vecSilentAngles.IsValid())
		{
			StartSilent(cmd);
			cmd->viewangles = m_vecSilentAngles;
			*bSendPacket = false;
		}
		else
		{
			FinishSilent(cmd);
			m_bSilentFire = false;
			m_vecSilentAngles.Invalidate();
		}
	}
	else if (m_iSilentTicks > -1)
	{
		if (m_iSilentTicks > 0 && m_vecSilentAngles.IsValid())
		{
			StartSilent(cmd);
			cmd->viewangles = m_vecSilentAngles;
			*bSendPacket = false;
		}
		else
		{
			m_iSilentTicks = -1;
			FinishSilent(cmd);
			m_vecSilentAngles.Invalidate();
		}

		m_iSilentTicks -= 1;
	}
}

bool CViewManager::IsUsingMinigun(CBasePlayer * player)
{
	if (player->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMinigun")) > 0)
		return true;

	if (player->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedWeapon")) > 0)
		return true;
	
	return false;
}

void CVM_ShotgunSound::FireGameEvent(IGameEvent* event)
{
	if (!m_bShotgunSound)
		return;
	
	const char* eventName = event->GetName();
	if (!_stricmp(eventName, XorStr("weapon_fire")) || !_stricmp(eventName, XorStr("bullet_impact")))
	{
		static ConVar* thirdMode = g_pInterface->Cvar->FindVar(XorStr("c_thirdpersonshoulder"));
		if (m_bIsGameTick || thirdMode->GetInt() == 0)
			return;
		
		int attacker = g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("userid")));
		if (attacker < 1 || attacker > 32)
			return;
		
		CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
		if (g_pInterface->EntList->GetClientEntity(attacker) != local || !local->IsAlive())
			return;

		CBaseWeapon* weapon = local->GetActiveWeapon();
		if (weapon == nullptr)
			return;

		int weaponId = weapon->GetWeaponID();
		int upgrade = weapon->GetNetProp<byte>(XorStr("DT_TerrorGun"), XorStr("m_upgradeBitVec"));
		int numAmmo = weapon->GetNetProp<byte>(XorStr("DT_TerrorGun"), XorStr("m_nUpgradedPrimaryAmmoLoaded"));

		MRecipientFilter filter;
		filter.AddRecipient(attacker);

		switch (weaponId)
		{
			case WeaponId_PumpShotgun:
			{
				if((upgrade & 1) && numAmmo > 0)
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/shotgun/gunfire/shotgun_fire_1_incendiary.wav"), 1.0f, SNDLEVEL_NONE);
				else
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/shotgun/gunfire/shotgun_fire_1.wav"), 1.0f, SNDLEVEL_NONE);
				break;
			}
			case WeaponId_Chrome:
			{
				if ((upgrade & 1) && numAmmo > 0)
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/shotgun_chrome/gunfire/shotgun_fire_1_incendiary.wav"), 1.0f, SNDLEVEL_NONE);
				else
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/shotgun_chrome/gunfire/shotgun_fire_1.wav"), 1.0f, SNDLEVEL_NONE);
				break;
			}
			case WeaponId_AutoShotgun:
			{
				if ((upgrade & 1) && numAmmo > 0)
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/auto_shotgun/gunfire/auto_shotgun_fire_1_incendiary.wav"), 1.0f, SNDLEVEL_NONE);
				else
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/auto_shotgun/gunfire/auto_shotgun_fire_1.wav"), 1.0f, SNDLEVEL_NONE);
				break;
			}
			case WeaponId_SPAS:
			{
				if ((upgrade & 1) && numAmmo > 0)
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/auto_shotgun_spas/gunfire/shotgun_fire_1_incendiary.wav"), 1.0f, SNDLEVEL_NONE);
				else
					g_pInterface->Sound->EmitSound(filter, -1, SNDCHAN_WEAPON, XorStr("weapons/auto_shotgun_spas/gunfire/shotgun_fire_1.wav"), 1.0f, SNDLEVEL_NONE);
				break;
			}
		}

		m_bIsGameTick = true;
	}
}

bool CVM_ShotgunSound::HasShotgun(CBaseWeapon* weapon)
{
	if (weapon == nullptr)
		return false;

	int weaponId = weapon->GetWeaponID();
	return IsShotgun(weaponId);
}
