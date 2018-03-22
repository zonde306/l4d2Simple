#pragma once
#include "BaseFeatures.h"

class CVisualDrop : public CBaseFeatures
{
public:
	CVisualDrop();
	~CVisualDrop();

	virtual void OnEnginePaint(PaintMode_t mode) override;
	virtual void OnMenuDrawing() override;

public:
	void DrawWeaponSpawn(CBaseWeapon* weapon, const Vector& screen);
	void DrawUpgradeSpawn(CBaseWeapon* weapon, const Vector& screen);
	void DrawAmmoStack(CBaseWeapon* weapon, const Vector& screen);
	void DrawMelee(CBaseWeapon* weapon, const Vector& screen);
	void DrawSurvovorDeadbody(CBaseWeapon* weapon, const Vector& screen);
	void DrawOtherWeapon(CBaseWeapon* weapon, const Vector& screen);

	bool CanDrawWeapon(CBaseWeapon* weapon);

private:
	bool m_bTier1 = false;
	bool m_bTier2 = false;
	bool m_bTier3 = false;
	bool m_bAidKit = false;
	bool m_bAmmoUpgrade = false;
	bool m_bGrenade = false;
	bool m_bCarry = false;
	bool m_bMelee = false;
	bool m_bProjectile = false;
	bool m_bDeadbody = false;

	float m_fMaxDistance = 1000.0f;
};

extern CVisualDrop* g_pVisualDrop;
