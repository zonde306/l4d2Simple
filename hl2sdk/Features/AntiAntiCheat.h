#pragma once
#include "BaseFeatures.h"
#include <vector>

class CAntiAntiCheat : public CBaseFeatures
{
public:
	CAntiAntiCheat();
	~CAntiAntiCheat();

	virtual void OnMenuDrawing() override;
	virtual bool OnUserMessage(int msgid, bf_read msgdata);
	virtual bool OnProcessGetCvarValue(const std::string& cvars, std::string& result);
	virtual bool OnProcessSetConVar(const std::string& cvars, std::string& value);
	virtual bool OnProcessClientCommand(const std::string& cmd);

	virtual void OnConfigLoading(const config_type& data);
	virtual void OnConfigSave(config_type& data);

	// 搜索字符串用到的函数
	// source 为被搜索的字符串
	// match 为搜索用的字符串，支持 * 和 ? 通配符
	// caseSensitive 为是否区分大小写
	bool FindMatchString(const std::string& source, const std::string& match, bool caseSensitive = true);

private:
	bool FindMatchString2(const std::string& source, const std::pair<std::string, std::string>& match, bool caseSensitive = true);

	template<typename T>
	void CreateMenuList(const std::string& name, std::vector<T>& set, std::function<std::string(T)> display,
		std::function<void(typename std::vector<T>::iterator&, std::string)> change);

private:
	std::vector<int> m_BlockUserMessage;
	std::vector<std::pair<std::string, std::string>> m_BlockQuery, m_BlockSetting;
	std::vector<std::string> m_BlockExecute;

	char m_szFilterText[255];
	char m_szRenameText[255];
};

template<typename T>
inline void CAntiAntiCheat::CreateMenuList(const std::string& name, std::vector<T>& set,
	std::function<std::string(T)> display,
	std::function<void(typename std::vector<T>::iterator&, std::string)> change)
{
	if (!ImGui::TreeNode(name.c_str()))
		return;

	ImGui::BeginChild(("_" + name).c_str(), ImVec2(0, 300), true);

	for (auto it = set.begin(); it != set.end(); )
	{
		std::string disName = display(*it);
		if (disName.empty())
			continue;

		if (m_szFilterText[0] != '\0' && disName.find(m_szFilterText) == std::string::npos)
			continue;

		bool deleted = false;
		ImGui::Selectable(disName.c_str());

		// 右键菜单
		if (ImGui::BeginPopupContextItem())
		{
			deleted = ImGui::Selectable(XorStr("delete"));
			
			if (ImGui::Selectable(XorStr("copy")))
				ImGui::SetClipboardText(disName.c_str());

			if (ImGui::Selectable(XorStr("paste")))
				change(it, ImGui::GetClipboardText());

			ImGui::EndPopup();
		}

		if (deleted)
			it = set.erase(it);
		else
			++it;
	}

	ImGui::EndChild();
	ImGui::TreePop();
}

extern CAntiAntiCheat* g_pAntiAntiCheat;
