#include "WeaponConfig.h"

void CWeaponConfig::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Presets")))
		return;

	ImGui::Checkbox(XorStr("Rule Allow"), &m_bAcitve);
	IMGUI_TIPS("不选上时下面的东西无效。");

	ImGui::Separator();

	static const char* NONE = XorStr("default");
	const char* current = NONE;
	size_t size = min(m_RuleNames.size(), m_Rules.size());

	if (m_iRuleSelection >= 0 && m_iRuleSelection < size)
	{
		current = m_RuleNames[m_iRuleSelection].c_str();
	}

	if (ImGui::BeginCombo(XorStr("Presets"), current))
	{
		bool selected = false;
		for (size_t i = 0; i < size; ++i)
		{
			const char* name = m_RuleNames[i].c_str();
			selected = (current == name);
			if (ImGui::Selectable(name, &selected))
			{
				m_iRuleSelection = i;
			}
		}

		selected = (m_iRuleSelection == -1);
		if (ImGui::Selectable(NONE, &selected))
		{
			m_iRuleSelection = -1;
		}

		ImGui::EndCombo();
	}

	ImGui::Separator();


}

void CWeaponConfig::OnConfigLoading(const config_type& data)
{

}

void CWeaponConfig::OnConfigSave(config_type& data)
{

}
