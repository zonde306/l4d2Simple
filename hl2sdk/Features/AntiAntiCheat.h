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

extern CAntiAntiCheat* g_pAntiAntiCheat;
