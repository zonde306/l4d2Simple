#pragma once
#include "BaseFeatures.h"
#include <vector>
#include <functional>

class CGEL_AntiAntiCheat;
class CAntiAntiCheat : public CBaseFeatures
{
public:
	CAntiAntiCheat();
	~CAntiAntiCheat();
	friend CGEL_AntiAntiCheat;

	virtual void OnMenuDrawing() override;
	virtual bool OnUserMessage(int msgid, bf_read msgdata) override;
	virtual bool OnProcessGetCvarValue(const std::string& cvars, std::string& result) override;
	virtual bool OnProcessSetConVar(const std::string& cvars, std::string& value) override;
	virtual bool OnProcessClientCommand(const std::string& cmd) override;
	virtual bool OnEmitSound(std::string& sample, int& entity, int& channel, float& volume,
		SoundLevel_t& level, int& flags, int& pitch, Vector& origin, Vector& direction,
		bool& updatePosition, float& soundTime) override;
	virtual bool OnSendNetMsg(INetMessage& msg, bool& bForceReliable, bool& bVoice) override;
	virtual void OnGameEventClient(IGameEvent* event) override;
	virtual void OnGameEvent(IGameEvent* event, bool dontBroadcast) override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;
	virtual void OnConnect() override;

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

	static bool MatchNotCaseSensitive(const char c1, const char c2);

	// 解除第三人称受到 mp_gamemode 的限制
	void UpdatePatchThirdPerson(bool install);

private:
	bool m_bBlockCRCCheck = true;
	bool m_bBlockNullSound = true;
	bool m_bNoHeartbeat = true;
	bool m_bPatchThirdPerson = false;
	bool m_bLogConVarInfo = false;

private:
	std::vector<int> m_BlockUserMessage;
	std::vector<std::pair<std::string, std::string>> m_BlockQuery, m_BlockSetting;
	std::vector<std::string> m_BlockExecute;

	char m_szFilterText[255];
	char m_szRenameText[255];
	CGEL_AntiAntiCheat* m_pEventListener;
};

extern CAntiAntiCheat* g_pAntiAntiCheat;

class CGEL_AntiAntiCheat : public IGameEventListener2
{
public:
	friend CAntiAntiCheat;

	virtual void FireGameEvent(IGameEvent *event) override;

private:
	CAntiAntiCheat* m_pParent = nullptr;
};
