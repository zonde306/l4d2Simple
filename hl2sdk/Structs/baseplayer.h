#pragma once
#include "baseentity.h"

class CBaseWeapon;

class CBasePlayer : public CBaseEntity
{
public:
	Vector GetEyePosition();
	QAngle GetEyeAngles();
	float GetFriction();
	int GetTeam();
	Vector GetVelocity();
	int GetHealth();
	bool IsAlive();
	int GetMoveType();
	Vector& GetPunch();
	int& GetTickBase();
	int& GetFlags();

	// 倒地
	bool IsIncapacitated();

	// 挂边
	bool IsHangingFromLedge();

	// 临时血量
	int GetTempHealth();

	// 当前武器
	CBaseWeapon* GetActiveWeapon();

	// 当前瞄准的队友
	int GetCrosshairID();

	// 特感是否为灵魂状态
	bool IsGhost();

	// 获取弹药数量
	int GetAmmo(int ammoType);

	// 获取当前控制者 (生还者被特感控)
	CBasePlayer* GetAttacker();

	// 获取当前的被控制者 (感染者控制的生还者)
	CBasePlayer* GetVictim();
};
