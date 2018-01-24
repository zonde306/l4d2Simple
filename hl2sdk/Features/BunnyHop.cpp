#include "BunnyHop.h"

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

	CBaseEntity* player = GetLocalPlayer();
	if (player == nullptr || !IsPlayerAlive(player) || IsPlayerInLadder(player))
		return;

	int flags = player->GetNetProp<MoveType>(XorStr("DT_BasePlayer"), XorStr("m_fFlags"));

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		if (m_pszAutoBhopMode == m_arrAutoBhopModeList[1].c_str())
			DoNormalAutoBhop(pCmd, flags);
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
	}
}

void CBunnyHop::OnMenuDrawing()
{
	if (!ImGui::Checkbox(XorStr("BunnyHop"), &m_bAcitve))
		return;

	if (!ImGui::Begin(XorStr("BunnyHop"), &m_bShowMenu))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginCombo(XorStr("AutoBhop"), m_pszAutoBhopMode))
	{
		for (const std::string& item : m_arrAutoBhopModeList)
		{
			bool selected = (m_pszAutoBhopMode == item.c_str());

			if (ImGui::Selectable(item.c_str(), &selected))
				m_pszAutoBhopMode = item.c_str();

			if(selected)
				ImGui::SetItemDefaultFocus();
		}
		
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo(XorStr("AutoStrafe"), m_pszAutoStrafeMode))
	{
		for (const std::string& item : m_arrAutoStrafeModeList)
		{
			bool selected = (m_pszAutoStrafeMode == item.c_str());

			if (ImGui::Selectable(item.c_str(), &selected))
				m_pszAutoStrafeMode = item.c_str();

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	ImGui::End();
}

void CBunnyHop::DoNormalAutoBhop(CUserCmd * pCmd, int flags)
{
	if ((pCmd->buttons & IN_JUMP) && (flags & FL_ONGROUND))
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

void CBunnyHop::DoExtraAutoStrafe(CBaseEntity* player, CUserCmd * pCmd, int flags)
{
	if (flags & FL_ONGROUND)
		return;

	Vector velocity = player->GetNetProp<Vector>(XorStr("DT_BasePlayer"), XorStr("m_vecVelocity[0]"));
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
