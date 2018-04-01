#include "BunnyHop.h"
#include "../hook.h"
#include <iostream>

#ifndef M_PI
#define M_PI	3.14159265358979323846
#define M_PI_F	((float)M_PI)
#endif

#ifndef RAD2DEG
#define RAD2DEG(x)  ((float)(x) * (float)(180.f / M_PI_F))
#define RadiansToDegrees RAD2DEG
#define DEG2RAD(x)  ((float)(x) * (float)(M_PI_F / 180.f))
#define DegreesToRadians DEG2RAD
#endif

#define IsBadMoveType(_mt)		(_mt == MOVETYPE_LADDER || _mt == MOVETYPE_NOCLIP || _mt == MOVETYPE_OBSERVER)

CBunnyHop* g_pBunnyHop = nullptr;

CBunnyHop::CBunnyHop() : CBaseFeatures::CBaseFeatures()
{
}

CBunnyHop::~CBunnyHop()
{
	CBaseFeatures::~CBaseFeatures();
}

void CBunnyHop::OnCreateMove(CUserCmd* pCmd, bool* bSendPacket)
{
	if (!m_bAcitve)
		return;

	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive() || player->IsIncapacitated() || player->IsHangingFromLedge())
		return;

	// int flags = player->GetFlags();
	int flags = g_pClientPrediction->GetFlags();

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		/*
		if (m_pszAutoBhopMode == m_arrAutoBhopModeList[1].c_str())
			DoNormalAutoBhop(player, pCmd, flags);
		else if(m_pszAutoBhopMode == m_arrAutoBhopModeList[2].c_str())
			DoSafeAutoBhop(pCmd, flags);

		if (m_pszAutoStrafeMode == m_arrAutoStrafeModeList[1].c_str())
			DoForwardAutoStrafe(pCmd, flags);
		else if (m_pszAutoStrafeMode == m_arrAutoStrafeModeList[2].c_str())
			DoBackAutoStrafe(pCmd, flags);
		else if (m_pszAutoStrafeMode == m_arrAutoStrafeModeList[3].c_str())
			DoLeftAutoStrafe(pCmd, flags);
		else if (m_pszAutoStrafeMode == m_arrAutoStrafeModeList[4].c_str())
			DoRightAutoStrafe(pCmd, flags);
		else if (m_pszAutoStrafeMode == m_arrAutoStrafeModeList[5].c_str())
			DoExtraAutoStrafe(player, pCmd, flags);
		*/

		if(m_iBhopMode == 1)
			DoNormalAutoBhop(player, pCmd, flags);
		else if(m_iBhopMode == 2)
			DoSafeAutoBhop(pCmd, flags);

		switch (m_iStrafeMode)
		{
		case 1:
			DoForwardAutoStrafe(pCmd, flags);
			break;
		case 2:
			DoBackAutoStrafe(pCmd, flags);
			break;
		case 3:
			DoLeftAutoStrafe(pCmd, flags);
			break;
		case 4:
			DoRightAutoStrafe(pCmd, flags);
			break;
		case 5:
			DoExtraAutoStrafe(player, pCmd, flags);
			break;
		case 6:
			DoFullAutoStrafe(player, pCmd, flags);
			break;
		}
	}
}

void CBunnyHop::OnMenuDrawing()
{
	/*
	if (!ImGui::Checkbox(XorStr("BunnyHop"), &m_bAcitve))
		return;

	if (!ImGui::Begin(XorStr("BunnyHop"), &m_bShowMenu))
	{
		ImGui::End();
		return;
	}
	*/
	
	if (!ImGui::TreeNode(XorStr("BunnyHop")))
		return;

	ImGui::Checkbox(XorStr("AutoBunnyHop Allow"), &m_bAcitve);
	IMGUI_TIPS("自动连跳，不选上时下面的东西无效。");

	if (m_pszAutoBhopMode == nullptr)
		m_pszAutoBhopMode = m_arrAutoBhopModeList[m_iBhopMode].c_str();

	if (ImGui::BeginCombo(XorStr("AutoBhop"), m_pszAutoBhopMode))
	{
		for (size_t i = 0; i < m_arrAutoBhopModeList.size(); ++i)
		{
			bool selected = (m_pszAutoBhopMode == m_arrAutoBhopModeList[i].c_str());
			if (ImGui::Selectable(m_arrAutoBhopModeList[i].c_str(), &selected))
			{
				m_pszAutoBhopMode = m_arrAutoBhopModeList[i].c_str();
				m_iBhopMode = i;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		/*
		for (const std::string& item : m_arrAutoBhopModeList)
		{
			bool selected = (m_pszAutoBhopMode == item.c_str());

			if (ImGui::Selectable(item.c_str(), &selected))
				m_pszAutoBhopMode = item.c_str();

			if(selected)
				ImGui::SetItemDefaultFocus();
		}
		*/
		
		ImGui::EndCombo();
	}

	if (m_pszAutoStrafeMode == nullptr)
		m_pszAutoStrafeMode = m_arrAutoStrafeModeList[m_iStrafeMode].c_str();

	if (ImGui::BeginCombo(XorStr("AutoStrafe"), m_pszAutoStrafeMode))
	{
		for (size_t i = 0; i < m_arrAutoStrafeModeList.size(); ++i)
		{
			bool selected = (m_pszAutoStrafeMode == m_arrAutoStrafeModeList[i].c_str());
			if (ImGui::Selectable(m_arrAutoStrafeModeList[i].c_str(), &selected))
			{
				m_pszAutoStrafeMode = m_arrAutoStrafeModeList[i].c_str();
				m_iStrafeMode = i;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		
		/*
		for (const std::string& item : m_arrAutoStrafeModeList)
		{
			bool selected = (m_pszAutoStrafeMode == item.c_str());

			if (ImGui::Selectable(item.c_str(), &selected))
				m_pszAutoStrafeMode = item.c_str();

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		*/

		ImGui::EndCombo();
	}

	// ImGui::End();
	ImGui::TreePop();
}

void CBunnyHop::OnConfigLoading(const config_type & data)
{
	if (data.find(XorStr("bunnyhop_enable")) == data.end())
		return;
	
	m_bAcitve = data.at(XorStr("bunnyhop_enable")).at(0) == '1';
	m_iBhopMode = atoi(data.at(XorStr("bunnyhop_mode")).c_str());
	m_iStrafeMode = atoi(data.at(XorStr("bunnyhop_strafe")).c_str());
}

void CBunnyHop::OnConfigSave(config_type & data)
{
	data[XorStr("bunnyhop_enable")] = std::to_string(m_bAcitve);
	data[XorStr("bunnyhop_mode")] = std::to_string(m_iBhopMode);
	data[XorStr("bunnyhop_strafe")] = std::to_string(m_iStrafeMode);
}

void CBunnyHop::DoNormalAutoBhop(CBasePlayer* player, CUserCmd * pCmd, int flags)
{
	if (player == nullptr || !player->IsAlive())
		return;

	// CBaseEntity* ground = player->GetGroundEntity();
	bool inWater = (player->GetWaterLevel() >= 2);
	bool isBadMoveType = (player->GetMoveType() != MOVETYPE_WALK);
	
	/*
	if ((pCmd->buttons & IN_JUMP) && (flags & FL_ONGROUND))
		pCmd->buttons &= ~IN_JUMP;
	*/

	if((pCmd->buttons & IN_JUMP) && !(flags & FL_ONGROUND) && !isBadMoveType && !inWater)
		pCmd->buttons &= ~IN_JUMP;
}

void CBunnyHop::DoSafeAutoBhop(CUserCmd * pCmd, int flags)
{
	if (!m_bLastJump && m_bShouldFake)
	{
		m_bShouldFake = false;
		pCmd->buttons |= IN_JUMP;
	}
	else if (pCmd->buttons & IN_JUMP)
	{
		if (flags & FL_ONGROUND)
		{
			m_bLastJump = true;
			m_bShouldFake = true;
		}
		else
		{
			pCmd->buttons &= ~IN_JUMP;
			m_bLastJump = false;
		}
	}
	else
	{
		m_bLastJump = false;
		m_bShouldFake = false;
	}
}

bool CBunnyHop::CanRunAutoStrafe(CUserCmd * pCmd, int flags)
{
	if (flags & FL_ONGROUND)
		return false;

	if ((pCmd->buttons & IN_FORWARD) || (pCmd->buttons & IN_BACK) ||
		(pCmd->buttons & IN_MOVELEFT) || (pCmd->buttons & IN_MOVERIGHT))
		return false;

	if (pCmd->mousedx <= 1 && pCmd->mousedx >= -1)
		return false;

	return true;
}

void CBunnyHop::DoForwardAutoStrafe(CUserCmd * pCmd, int flags)
{
	if(CanRunAutoStrafe(pCmd, flags))
		pCmd->sidemove = pCmd->mousedx < 0.f ? -450.f : 450.f;
}

void CBunnyHop::DoBackAutoStrafe(CUserCmd * pCmd, int flags)
{
	if (CanRunAutoStrafe(pCmd, flags))
		pCmd->sidemove = pCmd->mousedx < 0.f ? 450.f : -450.f;
}

void CBunnyHop::DoLeftAutoStrafe(CUserCmd * pCmd, int flags)
{
	if (CanRunAutoStrafe(pCmd, flags))
		pCmd->forwardmove = pCmd->mousedx < 0.f ? -450.f : 450.f;
}

void CBunnyHop::DoRightAutoStrafe(CUserCmd * pCmd, int flags)
{
	if (CanRunAutoStrafe(pCmd, flags))
		pCmd->forwardmove = pCmd->mousedx < 0.f ? 450.f : -450.f;
}

float CBunnyHop::GetDelta(float hiSpeed, float maxSpeed, float airAcceleRate)
{
	float term = (30.0f - (airAcceleRate * maxSpeed / 66.0f)) / hiSpeed;
	if (term < 1.0f && term > -1.0f)
		return acos(term);

	return 0.0f;
}

void CBunnyHop::DoExtraAutoStrafe(CBasePlayer* player, CUserCmd * pCmd, int flags)
{
	if (flags & FL_ONGROUND)
		return;

	Vector velocity = player->GetVelocity();
	velocity.z = 0.0f;

	float speed = velocity.Length();
	if (abs(speed) >= 0.1f)
	{
		float rad = GetDelta(speed, 300.0f, 1.0f);
		if (rad > 0.0f)
		{
			float deg = fmodf(RAD2DEG(rad), 360);

			Vector forward = pCmd->viewangles.Forward().Normalize();

			if (abs(forward.Dot(velocity)) > 30.0f)
			{
				float lookYawDeg = fmodf(pCmd->viewangles.x, 360);
				Vector wishMoveYaw(pCmd->forwardmove, pCmd->sidemove, 0.0f);
				wishMoveYaw = wishMoveYaw.Normalize();

				Vector o1 = wishMoveYaw.toAngles(), o2;

				float wishMoveDegLocal = fmodf(o1.x, 360);
				float wishMoveDeg = fmodf(lookYawDeg + wishMoveDegLocal, 360);

				velocity = velocity.Normalize();
				o2 = velocity.toAngles();

				float veloYawDeg = fmodf(o2.x, 360);

				float factor;
				if (veloYawDeg - wishMoveDeg > 0.0f)
					factor = 1.0f;
				else
					factor = -1.0f;

				float finalYawDeg = fmodf(veloYawDeg + factor * deg, 360);
				float finalYawRad = DEG2RAD(finalYawDeg);

				Vector move(pCmd->forwardmove, pCmd->sidemove, 0.0f);
				speed = move.Length();

				pCmd->forwardmove = speed * cos(finalYawRad);
				pCmd->sidemove = speed * sin(finalYawRad);
			}
		}
	}
}

void CBunnyHop::DoFullAutoStrafe(CBasePlayer * player, CUserCmd * pCmd, int flags)
{
	if (player == nullptr || !player->IsAlive())
		return;

	// CBaseEntity* ground = player->GetGroundEntity();
	bool inWater = (player->GetWaterLevel() >= 2);
	bool isBadMoveType = (player->GetMoveType() != MOVETYPE_WALK);

	if ((flags & FL_ONGROUND) || isBadMoveType || inWater)
		return;
	
	if (pCmd->sidemove != 0.0f || pCmd->forwardmove != 0.0f || pCmd->mousedx > 2)
		return;

	const static auto reddeg = [](float angles) -> float
	{
		return (angles * 180.0f) / M_PI_F;
	};

	const static auto normalize = [](float angles) -> float
	{
		while (angles < -180.0f)
			angles += 360.0f;
		while (angles > 180.0f)
			angles -= 360.0f;

		return angles;
	};

	Vector velocity = player->GetVelocity();
	static ConVar* sideSpeed = g_pInterface->Cvar->FindVar(XorStr("cl_sidespeed"));

	float yawvel = reddeg(atan2f(velocity.y, velocity.x));
	float anglesdiff = normalize(pCmd->viewangles.y - yawvel);
	float sidespeed = sideSpeed->GetFloat();
	pCmd->sidemove = (anglesdiff > 0.0f ? -sidespeed : sidespeed);
	pCmd->viewangles.y = normalize(pCmd->viewangles.y - anglesdiff);
}
