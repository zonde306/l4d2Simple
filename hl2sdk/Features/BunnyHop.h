#pragma once
#include "BaseFeatures.h"

class CBunnyHop : public CBaseFeatures
{
public:
	CBunnyHop();
	~CBunnyHop();

	virtual void OnCreateMove(CUserCmd*, bool*) override;
	virtual void OnMenuDrawing() override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

protected:
	void DoNormalAutoBhop(CBasePlayer*, CUserCmd* pCmd, int flags);
	void DoSafeAutoBhop(CUserCmd* pCmd, int flags);

	bool CanRunAutoStrafe(CUserCmd* pCmd, int flags);
	void DoForwardAutoStrafe(CUserCmd* pCmd, int flags);
	void DoBackAutoStrafe(CUserCmd* pCmd, int flags);
	void DoLeftAutoStrafe(CUserCmd* pCmd, int flags);
	void DoRightAutoStrafe(CUserCmd* pCmd, int flags);

	float GetDelta(float hiSpeed, float maxSpeed, float airAcceleRate);
	void DoExtraAutoStrafe(CBasePlayer* player, CUserCmd* pCmd, int flags);
	void DoFullAutoStrafe(CBasePlayer* player, CUserCmd* pCmd, int flags);

	void DoEdgeJump(CUserCmd* pCmd, int flags);

private:
	bool m_bShowMenu = false;
	bool m_bAcitve = true;
	bool m_bAutoStrafe = false;
	bool m_bEdgeJump = false;
	bool m_bNoDuckCooldown = false;
	float m_fEdgeJumpSpeed = 150.0f;

	const char* m_pszAutoBhopMode = nullptr;
	const char* m_pszAutoStrafeMode = nullptr;
	size_t m_iBhopMode = 2;
	size_t m_iStrafeMode = 0;

	bool m_bLastJump = false;
	bool m_bShouldFake = false;

	std::array<std::string, 3> m_arrAutoBhopModeList
	{
		XorStr("Disabled"),
		XorStr("Normal"),
		XorStr("SMAC-Safe")
	};

	std::array<std::string, 7> m_arrAutoStrafeModeList
	{
		XorStr("Disabled"),
		XorStr("Forward"),
		XorStr("Back"),
		XorStr("Left"),
		XorStr("Right"),
		XorStr("Don't turn"),
		XorStr("Full Speed")
	};
};

extern CBunnyHop* g_pBunnyHop;
