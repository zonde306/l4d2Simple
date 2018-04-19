#pragma once
#include "BaseFeatures.h"

class CVisualPlayer : public CBaseFeatures
{
public:
	CVisualPlayer();
	~CVisualPlayer();

	virtual void OnEnginePaint(PaintMode_t mode) override;
	virtual void OnSceneEnd() override;
	virtual void OnMenuDrawing() override;
	virtual void OnFrameStageNotify(ClientFrameStage_t stage) override;
	virtual bool OnFindMaterial(std::string& materialName, std::string& textureGroupName) override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	bool HasTargetVisible(CBasePlayer* entity);

public:
	void DrawBox(bool friendly, CBasePlayer* entity, const Vector& head, const Vector& foot);
	void DrawBone(CBasePlayer* entity, bool friendly);
	void DrawHeadBox(CBasePlayer* entity, const Vector& head, float distance = 1000.0f);

	std::string DrawName(int index, bool separator = false);
	std::string DrawHealth(CBasePlayer* entity, bool separator = false);
	std::string DrawWeapon(CBasePlayer* entity, bool separator = false);
	std::string DrawDistance(CBasePlayer* entity, float distance, bool separator = false);
	std::string DrawCharacter(CBasePlayer* entity, bool separator = false);
	bool DrawSpectator(CBasePlayer* player, CBasePlayer* local, int index, int line);

	enum DrawPosition_t
	{
		DP_Anywhere = 0,
		DP_Left = (1 << 0),
		DP_Right = (1 << 1),
		DP_Top = (1 << 2),
		DP_Bottom = (1 << 3),
	};

	int GetTextMaxWide(const std::string& text);
	DrawPosition_t GetTextPosition(const std::string& text, Vector& head);
	D3DCOLOR GetDrawColor(CBasePlayer* entity);

	Vector GetSeePosition(CBasePlayer* player, const Vector& eyePosition, const QAngle& eyeAngles);

private:
	bool m_bBox = false;
	bool m_bName = false;
	bool m_bHealth = false;
	bool m_bDistance = false;
	bool m_bWeapon = false;
	bool m_bCharacter = false;
	bool m_bBone = false;
	bool m_bDrawToLeft = false;
	bool m_bHeadBox = false;
	bool m_bChams = false;
	bool m_bSpectator = true;

	bool m_bBarrel = false;
	float m_fBarrelDistance = 1500.0f;

	bool m_bNoVomit = true;
	bool m_bCleanVision = true;
	bool m_bCleanGhost = true;

private:
	HFont m_hSurfaceFont = 0;
	int m_iLocalPlayer = 0;
	int m_iNextLine = 0;
};

extern CVisualPlayer* g_pVisualPlayer;
