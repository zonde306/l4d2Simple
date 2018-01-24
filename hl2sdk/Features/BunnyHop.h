#pragma once
#include "BaseFeatures.h"

class CBunnyHop : public CBaseFeatures
{
public:
	CBunnyHop();
	~CBunnyHop();

	virtual void OnCreateMove(CUserCmd*, bool*) override;
	virtual void OnMenuDrawing() override;

protected:
	void DoNormalAutoBhop(CUserCmd* pCmd, int flags);
	void DoSafeAutoBhop(CUserCmd* pCmd, int flags);

	bool CanRunAutoStrafe(CUserCmd* pCmd, int flags);
	void DoForwardAutoStrafe(CUserCmd* pCmd, int flags);
	void DoBackAutoStrafe(CUserCmd* pCmd, int flags);
	void DoLeftAutoStrafe(CUserCmd* pCmd, int flags);
	void DoRightAutoStrafe(CUserCmd* pCmd, int flags);

	float GetDelta(float hiSpeed, float maxSpeed, float airAcceleRate);
	void DoExtraAutoStrafe(CBaseEntity* player, CUserCmd* pCmd, int flags);

private:
	bool m_bShowMenu = false;
	bool m_bAcitve = false;
	bool m_bAutoStrafe = false;
	const char* m_pszAutoBhopMode = nullptr;
	const char* m_pszAutoStrafeMode = nullptr;

	bool m_bLastJump = false;
	bool m_bShouldFake = false;

	std::array<std::string, 3> m_arrAutoBhopModeList
	{
		XorStr("Disabled"),
		XorStr("Normal"),
		XorStr("SMAC-Safe")
	};

	std::array<std::string, 6> m_arrAutoStrafeModeList
	{
		XorStr("Disabled"),
		XorStr("Forward"),
		XorStr("Back"),
		XorStr("Left"),
		XorStr("Right"),
		XorStr("Don't turn"),
	};
};
