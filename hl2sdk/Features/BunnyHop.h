#pragma once
#include "BaseFeatures.h"

class CBunnyHop : public CBaseFeatures
{
public:
	CBunnyHop();
	~CBunnyHop();

	virtual void OnCreateMove(CUserCmd*, bool*) override;
	virtual void OnMenuDrawing() override;

	virtual void OnConfigLoading(CProfile& cfg) override;
	virtual void OnConfigSave(CProfile& cfg) override;

	virtual void OnDisconnect() override;

protected:
	void DoNormalAutoBhop(CBasePlayer*, CUserCmd* pCmd, int flags);
	void DoSafeAutoBhop(CUserCmd* pCmd, int flags);
	void DoNormalAutoBhopEx(CBasePlayer*, CUserCmd* pCmd, int flags);

	bool CanRunAutoStrafe(CUserCmd* pCmd, int flags);
	void DoForwardAutoStrafe(CUserCmd* pCmd, int flags);
	void DoBackAutoStrafe(CUserCmd* pCmd, int flags);
	void DoLeftAutoStrafe(CUserCmd* pCmd, int flags);
	void DoRightAutoStrafe(CUserCmd* pCmd, int flags);
	void DoNoFallDamage(CUserCmd* pCmd, int flags);

	float GetDelta(float hiSpeed, float maxSpeed, float airAcceleRate);
	void DoExtraAutoStrafe(CBasePlayer* player, CUserCmd* pCmd, int flags);
	void DoFullAutoStrafe(CBasePlayer* player, CUserCmd* pCmd, int flags);

	void DoEdgeJump(CUserCmd* pCmd, int flags);
	bool IsOnLadder(CBasePlayer* player);
	bool IsNearGround(CBasePlayer* player, int tick, int flags);

private:
	bool m_bShowMenu = false;
	bool m_bAcitve = true;
	bool m_bAutoStrafe = false;
	bool m_bEdgeJump = false;
	bool m_bNoDuckCooldown = false;
	float m_fEdgeJumpSpeed = 150.0f;
	bool m_bNoFallDamage = false;

	const char* m_pszAutoBhopMode = nullptr;
	const char* m_pszAutoStrafeMode = nullptr;
	size_t m_iBhopMode = 2;
	size_t m_iStrafeMode = 0;

	bool m_bLastJump = false;
	bool m_bShouldFake = false;

	std::array<std::string, 4> m_arrAutoBhopModeList
	{
		XorStr("Disabled"),
		XorStr("Normal"),
		XorStr("SMAC-Safe"),
		XorStr("LiAC-Safe")
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
