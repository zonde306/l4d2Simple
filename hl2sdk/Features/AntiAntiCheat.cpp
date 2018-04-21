#include "AntiAntiCheat.h"
#include "../../l4d2Simple2/config.h"
#include <functional>
#include <cctype>

CAntiAntiCheat* g_pAntiAntiCheat = nullptr;
#define FCVAR_REMOVE	(FCVAR_CHEAT|FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOT_CONNECTED|FCVAR_SERVER_CAN_EXECUTE)
#define FCVAR_SETTING	(FCVAR_SERVER_CANNOT_QUERY)

CAntiAntiCheat::CAntiAntiCheat() : CBaseFeatures::CBaseFeatures()
{
	m_pEventListener = new CGEL_AntiAntiCheat();
	m_pEventListener->m_pParent = this;

	g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_team"), false);
	g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_spawn"), false);
	// g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_first_spawn"), false);
}

CAntiAntiCheat::~CAntiAntiCheat()
{
	g_pInterface->GameEvent->RemoveListener(m_pEventListener);
	delete m_pEventListener;
	
	CBaseFeatures::~CBaseFeatures();
}

void CAntiAntiCheat::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Anti Anti Cheat")))
		return;

	ImGui::Checkbox(XorStr("No CRC Check"), &m_bBlockCRCCheck);
	IMGUI_TIPS("屏蔽 CRC 检查，如果不是必须禁用则开启。");

	ImGui::Checkbox(XorStr("No Null Sound"), &m_bBlockNullSound);
	IMGUI_TIPS("屏蔽无效声音，可以防止服务器通过声音让客户端卡死。");

	ImGui::Checkbox(XorStr("No Heartbeat Loop"), &m_bNoHeartbeat);
	IMGUI_TIPS("屏蔽生还者黑白时的心跳声。");

	ImGui::Separator();

	// 过滤器
	ImGui::InputText(XorStr("aacFilter"), m_szFilterText, 255);
	bool changed = false;

	if (m_szFilterText[0] != '\0')
	{
		bool clicked = ImGui::Button(XorStr("Add Query"));
		IMGUI_TIPS("添加一个防止被服务器查询的 ConVar\n如果需要伪造结果，则将返回值用 空格 来隔开。\n格式为 <名字>[ <伪造值>]。");
		if (clicked && !changed)
		{
			m_BlockQuery.emplace_back(g_pConfig->ParseKeyValueEx(m_szFilterText));
			changed = true;
		}
		
		ImGui::SameLine();
		clicked = ImGui::Button(XorStr("Add Setting"));
		IMGUI_TIPS("添加一个防止被服务器修改的 ConVar\n如果需要覆盖成自己的结果，则将返回值用 空格 来隔开。\n格式为 <名字>[ <覆盖值>]。");
		if (clicked && !changed)
		{
			m_BlockSetting.emplace_back(g_pConfig->ParseKeyValueEx(m_szFilterText));
			changed = true;
		}

		ImGui::SameLine();
		clicked = ImGui::Button(XorStr("Add Command"));
		IMGUI_TIPS("添加一个防止被服务器执行的命令。");
		if (clicked && !changed)
		{
			m_BlockExecute.emplace_back(Utils::Trim(m_szFilterText));
			changed = true;
		}

		ImGui::SameLine();
		clicked = ImGui::Button(XorStr("Add UserMsg"));
		IMGUI_TIPS("添加一个防止被客户端处理的用户消息。");
		if (clicked && !changed)
		{
			m_BlockUserMessage.emplace_back(atoi(Utils::Trim(m_szFilterText).c_str()));
			changed = true;
		}
	}

	ImGui::Text(XorStr(u8"可以使用 ? 匹配单个字符，使用 * 匹配零个或多个字符。"));
	ImGui::Separator();

	CreateMenuList<std::pair<std::string, std::string>>(XorStr("Query"), m_BlockQuery,
		[](const auto& it) -> std::string
	{
		if (it.second.empty())
			return it.first;
		
		return it.first + " = " + it.second;
	},
		[](auto& it, const std::string& value) -> void
	{
		it->first = value;
	});

	CreateMenuList<std::pair<std::string, std::string>>(XorStr("Setting"), m_BlockSetting,
		[](const auto& it) -> std::string
	{
		if (it.second.empty())
			return it.first;
		
		return it.first + " = " + it.second;
	},
		[](auto& it, const std::string& value) -> void
	{
		it->first = value;
	});

	CreateMenuList<std::string>(XorStr("Execute"), m_BlockExecute,
		[](const auto& it) -> std::string
	{
		return it;
	},
		[](auto& it, const std::string& value) -> void
	{
		*it = value;
	});

	CreateMenuList<int>(XorStr("UserMessage"), m_BlockUserMessage,
		[](const auto& it) -> std::string
	{
		return std::to_string(it);
	},
		[](auto& it, const std::string& value) -> void
	{
		*it = atoi(value.c_str());
	});

	ImGui::TreePop();

	if (changed)
		m_szFilterText[0] = '\0';
}

bool CAntiAntiCheat::OnUserMessage(int msgid, bf_read msgdata)
{
	if (std::find(m_BlockUserMessage.begin(), m_BlockUserMessage.end(), msgid) !=
		m_BlockUserMessage.end())
		return false;
	
	return true;
}

bool CAntiAntiCheat::OnProcessGetCvarValue(const std::string& cvars, std::string & result)
{
	auto it = std::find_if(m_BlockQuery.begin(), m_BlockQuery.end(),
		std::bind(&CAntiAntiCheat::FindMatchString2, this, cvars, std::placeholders::_1, false));

	if (it == m_BlockQuery.end())
		return true;
	
	if (it->second.empty())
		return false;

	result = it->second;
	return true;
}

bool CAntiAntiCheat::OnProcessSetConVar(const std::string& cvars, std::string& value)
{
	auto it = std::find_if(m_BlockSetting.begin(), m_BlockSetting.end(),
		std::bind(&CAntiAntiCheat::FindMatchString2, this, cvars, std::placeholders::_1, false));

	if (it == m_BlockSetting.end())
		return true;

	if (it->second.empty())
		return false;

	value = it->second;
	return true;
}

bool CAntiAntiCheat::OnProcessClientCommand(const std::string& cmd)
{
	// ConVar 有时候也会通过这个来修改的，所以需要将空格后的东西丢弃
	auto it = std::find_if(m_BlockExecute.begin(), m_BlockExecute.end(),
		std::bind(&CAntiAntiCheat::FindMatchString, this, cmd.substr(0, cmd.find(' ')), std::placeholders::_1, false));

	if (it == m_BlockExecute.end())
		return true;
	
	return false;
}

bool CAntiAntiCheat::OnEmitSound(std::string & sample, int & entity, int & channel, float & volume, SoundLevel_t & level, int & flags, int & pitch, Vector & origin, Vector & direction, bool & updatePosition, float & soundTime)
{
	const auto stopSound = [&]() -> void
	{
		flags = (SND_STOP|SND_STOP_LOOPING);
		pitch = -1;
		volume = -1.0f;
		sample.clear();
	};
	
	if (m_bBlockNullSound && sample.find("null") != std::string::npos)
		stopSound();
	
	if(m_bNoHeartbeat && sample.find("heartbeatloop") != std::string::npos)
		stopSound();

	return true;
}

void CAntiAntiCheat::OnConfigLoading(const config_type & data)
{
	for (const auto& it : g_pConfig->GetMainKey(XorStr("AntiQuery")))
	{
		m_BlockQuery.emplace_back(it.first, it.second.m_sValue);
	}

	for (const auto& it : g_pConfig->GetMainKey(XorStr("AntiSetting")))
	{
		m_BlockSetting.emplace_back(it.first, it.second.m_sValue);
	}

	for (const auto& it : g_pConfig->GetMainKey(XorStr("AntiExecute")))
	{
		m_BlockExecute.emplace_back(it.first);
	}

	for (const auto& it : g_pConfig->GetMainKey(XorStr("AntiUserMsg")))
	{
		m_BlockUserMessage.emplace_back(atoi(it.first.c_str()));
	}

	const std::string mainKeys = XorStr("AntiAntiCheat");
	m_bBlockCRCCheck = g_pConfig->GetBoolean(mainKeys, XorStr("anticheat_crc_bypass"), m_bBlockCRCCheck);
	m_bBlockNullSound = g_pConfig->GetBoolean(mainKeys, XorStr("anticheat_nullsnd_bypass"), m_bBlockNullSound);
	m_bNoHeartbeat = g_pConfig->GetBoolean(mainKeys, XorStr("anticheat_no_heartbeat"), m_bNoHeartbeat);
}

void CAntiAntiCheat::OnConfigSave(config_type & data)
{
	std::string mainKeys = XorStr("AntiQuery");
	for (const auto& it : m_BlockQuery)
	{
		g_pConfig->SetValue(mainKeys, it.first, it.second);
	}

	mainKeys = XorStr("AntiSetting");
	for (const auto& it : m_BlockSetting)
	{
		g_pConfig->SetValue(mainKeys, it.first, it.second);
	}

	mainKeys = XorStr("AntiExecute");
	for (const auto& it : m_BlockExecute)
	{
		g_pConfig->SetValue(mainKeys, it, "");
	}

	mainKeys = XorStr("AntiUserMsg");
	for (const auto& it : m_BlockUserMessage)
	{
		g_pConfig->SetValue(mainKeys, std::to_string(it), "");
	}

	mainKeys = XorStr("AntiAntiCheat");
	g_pConfig->SetValue(mainKeys, XorStr("anticheat_crc_bypass"), m_bBlockCRCCheck);
	g_pConfig->SetValue(mainKeys, XorStr("anticheat_nullsnd_bypass"), m_bBlockNullSound);
	g_pConfig->SetValue(mainKeys, XorStr("anticheat_no_heartbeat"), m_bNoHeartbeat);
}

void CAntiAntiCheat::OnConnect()
{
	if (!m_bBlockCRCCheck)
		return;

	static ConVar* sv_consistency = g_pInterface->Cvar->FindVar(XorStr("sv_consistency"));
	static ConVar* sv_pure = g_pInterface->Cvar->FindVar(XorStr("sv_pure"));
	if (sv_consistency != nullptr)
	{
		if (sv_consistency->IsFlagSet(FCVAR_REMOVE))
			sv_consistency->RemoveFlags(FCVAR_REMOVE);
		
		sv_consistency->SetValue(0);
	}
	if (sv_pure != nullptr)
	{
		if (sv_pure->IsFlagSet(FCVAR_REMOVE))
			sv_pure->RemoveFlags(FCVAR_REMOVE);
		
		sv_pure->SetValue(0);
	}
}

bool CAntiAntiCheat::FindMatchString(const std::string & source, const std::string & match, bool caseSensitive)
{
	auto equal = [&caseSensitive](char s, char m) -> bool
	{
		if (caseSensitive)
			return (s == m);

		return (std::tolower(s) == std::tolower(m));
	};
	
	if (match.find('?') == std::string::npos && match.find('*') == std::string::npos)
		return std::equal(source.begin(), source.end(), match.begin(), match.end(), equal);
	
	size_t indexSource = 0, indexMatch = 0, lastStar = UINT_MAX;
	size_t maxSource = source.length(), maxMatch = match.length();

	while(indexSource < maxSource && indexMatch < maxMatch)
	{
		if (match[indexMatch] == '?')
		{
			// 单字符匹配
			++indexSource;
			++indexMatch;
		}
		else if (match[indexMatch] == '*')
		{
			// 合并连续的 * 符号，因为没有意义
			while (++indexMatch < maxMatch && match[indexMatch] == '*');
			lastStar = --indexMatch;
			
			// 最后一个字符是 * 时不需要进行匹配了，因为这样绝对会成功的
			if (indexMatch + 1 >= maxMatch || indexSource + 1 >= maxSource)
				return true;

			// 预测是否可以跳出多个匹配
			if (equal(match[indexMatch + 1], source[indexSource + 1]))
			{
				++indexSource;
				++indexMatch;
			}
			
			++indexSource;
		}
		else if (!equal(match[indexMatch], source[indexSource]))
		{
			if (lastStar < maxMatch)
			{
				// 尝试回溯到上一个 * 处继续匹配
				indexMatch = lastStar;
			}
			else
			{
				// 失败，没有任何可以回溯的 *
				return false;
			}
		}
	}

	return (indexSource >= maxSource && indexMatch >= maxMatch);
}

bool CAntiAntiCheat::FindMatchString2(const std::string & source, const std::pair<std::string, std::string>& match, bool caseSensitive)
{
	return FindMatchString(source, match.first, caseSensitive);
}

template<typename T>
inline void CAntiAntiCheat::CreateMenuList(const std::string& name, std::vector<T>& set,
	std::function<std::string(T)> display,
	std::function<void(typename std::vector<T>::iterator&, std::string)> change)
{
	if (!ImGui::TreeNode(name.c_str()))
		return;

	size_t length = set.size();
	if (length > 9)
		length = 9;

	length *= g_pDrawing->GetDrawTextSize(name.c_str()).second;
	ImGui::BeginChild(("_" + name).c_str(), ImVec2(0.0f, static_cast<float>(length)), true, ImGuiWindowFlags_HorizontalScrollbar);

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

void CGEL_AntiAntiCheat::FireGameEvent(IGameEvent * event)
{
	if (!m_pParent->m_bBlockCRCCheck)
		return;
	
	if (g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("userid"))) != g_pInterface->Engine->GetLocalPlayer())
		return;

	static ConVar* sv_consistency = g_pInterface->Cvar->FindVar(XorStr("sv_consistency"));
	static ConVar* sv_pure = g_pInterface->Cvar->FindVar(XorStr("sv_pure"));
	if (sv_consistency != nullptr)
	{
		if (sv_consistency->IsFlagSet(FCVAR_REMOVE))
			sv_consistency->RemoveFlags(FCVAR_REMOVE);

		sv_consistency->SetValue(0);
	}
	if (sv_pure != nullptr)
	{
		if (sv_pure->IsFlagSet(FCVAR_REMOVE))
			sv_pure->RemoveFlags(FCVAR_REMOVE);

		sv_pure->SetValue(0);
	}
}
