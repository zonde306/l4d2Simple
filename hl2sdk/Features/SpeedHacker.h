#pragma once
#include "BaseFeatures.h"
#include <deque>

class CSpeedHacker : public CBaseFeatures
{
public:
	CSpeedHacker();
	~CSpeedHacker();
	
	virtual void OnMenuDrawing() override;
	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	void RunPositionAdjustment(CUserCmd* cmd);
	void RunBacktracking(CUserCmd* cmd);
	void RecordBacktracking(CUserCmd* cmd);
	void RunForwardtrack(CUserCmd* cmd);

private:
	float m_fOriginSpeed = 1.0f;
	float m_fUseSpeed = 1.0f;
	float m_fFireSpeed = 1.0f;
	float m_fWalkSpeed = 1.0f;
	bool m_bActive = false;
	bool m_bPositionAdjustment = false;
	bool m_bBacktrack = false;
	bool m_bForwardtrack = false;

private:
	struct _Backtrack
	{
	public:
		int32_t m_iTickCount;
		Vector m_vecOrigin;
		_Backtrack(int tickCount, const Vector& origin);
	};

	std::deque<_Backtrack> m_Backtracking[64];
	int m_iBacktrackingTarget = -1;
};

extern CSpeedHacker* g_pSpeedHacker;
