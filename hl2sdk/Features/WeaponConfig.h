#pragma once
#include "BaseFeatures.h"
#include "../../l4d2Simple2/config.h"
#include <vector>
#include <sstream>
#include <string>
#include <string_view>

class CWeaponConfig : public CBaseFeatures
{
public:
	virtual void OnMenuDrawing() override;
	virtual void OnConfigLoading(CProfile& cfg) override;
	virtual void OnConfigSave(CProfile& cfg) override;
	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;

protected:
	void SaveToFile(const char* preset);
	void LoadFromFile(const char* preset);
	void ChangePreset();
	void AddOrUpdatePreset();
	void RemovePreset();
	void MoveUp();
	void MoveDown();
	size_t FindRule(std::string_view preset);

	struct FetchActiveRule_t
	{
		std::string m_weapon;
		int m_team;

		bool m_speed;
		bool m_duck;
		bool m_score;
		bool m_flashlight;
		bool m_caps;
		bool m_scroll;
		bool m_num;
		bool m_attack;
		bool m_attack2;
		bool m_use;

		bool operator==(const FetchActiveRule_t& other) const;
	};

	size_t FetchActiveRule(const FetchActiveRule_t& data);

private:
	struct Rule_t
	{
		CProfile m_config;
		std::string m_name;
		std::string m_weapon;
		int m_team;
		size_t m_order;

		bool operator<(const Rule_t& other);

		bool m_speed;
		bool m_duck;
		bool m_score;
		bool m_flashlight;
		bool m_caps;
		bool m_scroll;
		bool m_num;
		bool m_attack;
		bool m_attack2;
		bool m_use;
	};

	CProfile m_DefaultRule;
	std::vector<Rule_t> m_Rules;
	int m_iRuleSelection = -1;
	bool m_bPresetChanging = false;

	char m_szCurrentName[64];
	char m_szCurrentWeapon[64];
	int m_iCurrentTeam = 0;

	bool m_bInSpeed = false;
	bool m_bInDuck = false;
	bool m_bInScore = false;
	bool m_bInFlashlight = false;
	bool m_bOnCapsLock = false;
	bool m_bOnScrollLock = false;
	bool m_bOnNumLock = false;
	bool m_bInAttack = false;
	bool m_bInAttack2 = false;
	bool m_bInUse = false;
};

extern CWeaponConfig* g_pWeaponConfig;
