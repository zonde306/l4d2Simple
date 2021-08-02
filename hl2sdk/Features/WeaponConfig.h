#pragma once
#include "BaseFeatures.h"
#include <vector>
#include <sstream>

class CWeaponConfig : public CBaseFeatures
{
public:
	virtual void OnMenuDrawing() override;
	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

private:
	std::vector<std::stringstream> m_Rules;
	std::vector<std::string> m_RuleNames;
	std::vector<std::string> m_CondWeapon;
	std::vector<int> m_CondTeam;
	std::vector<int> m_CondButton;
	int m_iRuleSelection = -1;

	bool m_bAcitve = false;
};

