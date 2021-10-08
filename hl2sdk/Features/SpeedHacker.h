#pragma once
#include "BaseFeatures.h"
#include <deque>
#include <climits>

class CSpeedHacker : public CBaseFeatures
{
public:
	CSpeedHacker();
	~CSpeedHacker();
	
	virtual void OnMenuDrawing() override;
	virtual void OnCreateMove(CUserCmd* cmd, bool* bSendPacket) override;
	virtual void OnEnginePaint(PaintMode_t) override;

	virtual void OnConfigLoading(CProfile& cfg) override;
	virtual void OnConfigSave(CProfile& cfg) override;

	void RunPositionAdjustment(CUserCmd* cmd, bool bSendPacket);
	void RunBacktracking(CUserCmd* cmd, bool bSendPacket);
	void RecordBacktracking(CUserCmd* cmd);
	void RunForwardtrack(CUserCmd* cmd, bool bSendPacket);

	virtual void OnDisconnect() override;

private:
	float m_fOriginSpeed = 1.0f;
	float m_fUseSpeed = 1.0f;
	float m_fFireSpeed = 1.0f;
	float m_fWalkSpeed = 1.0f;
	bool m_bActive = false;
	bool m_bPositionAdjustment = false;
	bool m_bBacktrack = false;
	bool m_bForwardtrack = false;
	bool m_bLACSafe = true;
	bool m_bDebug = false;

private:
	struct _Backtrack
	{
	public:
		int32_t m_iTickCount;
		Vector m_vecOrigin;
		_Backtrack(int tickCount, const Vector& origin);
	};

	std::deque<_Backtrack> m_Backtracking[65];
	int m_iBacktrackingTarget = -1;
	int m_iTickCount = INT_MAX;
};

extern CSpeedHacker* g_pSpeedHacker;
