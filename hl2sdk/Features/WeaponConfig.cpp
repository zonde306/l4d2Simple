#include "WeaponConfig.h"
#include "../hook.h"
#include <fstream>
#include <sstream>

CWeaponConfig* g_pWeaponConfig;

void CWeaponConfig::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Presets")))
		return;

	static const char* NONE = XorStr("default");
	const char* current = NONE;
	size_t size = m_Rules.size();

	if (m_iRuleSelection >= 0 && m_iRuleSelection < size)
	{
		current = m_Rules[m_iRuleSelection].m_name.c_str();
	}

	if (ImGui::BeginCombo(XorStr("Presets"), current))
	{
		bool selected = false;
		for (size_t i = 0; i < size; ++i)
		{
			const char* name = m_Rules[i].m_name.c_str();
			selected = (current == name);
			if (ImGui::Selectable(name, &selected))
			{
				m_iRuleSelection = i;
				strcpy_s(m_szCurrentName, name);
				ChangePreset();
			}
		}

		selected = (m_iRuleSelection == -1);
		if (ImGui::Selectable(NONE, &selected))
		{
			m_iRuleSelection = -1;
			m_szCurrentName[0] = '\0';
			ChangePreset();
		}

		ImGui::EndCombo();
	}

	ImGui::Separator();

	ImGui::InputText(XorStr("Name"), m_szCurrentName, 64);
	IMGUI_TIPS("预设文件名");

	ImGui::InputText(XorStr("Weapon"), m_szCurrentWeapon, 64);
	IMGUI_TIPS("武器类名，支持通配符");

	m_iCurrentTeam = (ImGui::RadioButton(XorStr("Unlimit"), m_iCurrentTeam == 0) ? 0 : m_iCurrentTeam);
	ImGui::SameLine();
	m_iCurrentTeam = (ImGui::RadioButton(XorStr("Survivor"), m_iCurrentTeam == 2) ? 2 : m_iCurrentTeam);
	ImGui::SameLine();
	m_iCurrentTeam = (ImGui::RadioButton(XorStr("Infected"), m_iCurrentTeam == 3) ? 3 : m_iCurrentTeam);

	ImGui::Checkbox(XorStr("Speed"), &m_bInSpeed);
	ImGui::SameLine();
	ImGui::Checkbox(XorStr("Duck"), &m_bInDuck);
	ImGui::SameLine();
	ImGui::Checkbox(XorStr("Score"), &m_bInScore);
	ImGui::SameLine();
	ImGui::Checkbox(XorStr("Flashlight"), &m_bInFlashlight);

	ImGui::Checkbox(XorStr("Caps Lock"), &m_bOnCapsLock);
	ImGui::SameLine();
	ImGui::Checkbox(XorStr("Scoril Lock"), &m_bOnScrollLock);
	ImGui::SameLine();
	ImGui::Checkbox(XorStr("Num Lock"), &m_bOnNumLock);

	ImGui::Separator();

	bool save = ImGui::Button(XorStr("Save"));
	IMGUI_TIPS("添加/保存预设");

	ImGui::SameLine();
	bool reload = ImGui::Button(XorStr("Reload"));
	IMGUI_TIPS("加载预设");

	ImGui::SameLine();
	bool del = ImGui::Button(XorStr("Remove"));
	IMGUI_TIPS("删除预设");

	bool up = ImGui::Button(XorStr("Up"));
	IMGUI_TIPS("上移");

	ImGui::SameLine();
	bool down = ImGui::Button(XorStr("Down"));
	IMGUI_TIPS("下移");

	ImGui::TreePop();

	if (save)
		AddOrUpdatePreset();
	else if (reload)
		LoadFromFile(m_szCurrentName);
	else if (del)
		RemovePreset();
	else if (up)
		MoveUp();
	else if (down)
		MoveDown();
}

void CWeaponConfig::OnConfigLoading(CProfile& cfg)
{
	if (m_bPresetChanging)
		return;

	m_DefaultRule = cfg;

	std::string mainKeys = XorStr("Preset");
	std::string presets = cfg.GetString(mainKeys, XorStr("presets"), "");
	for (std::string preset : Utils::StringSplit(presets, ","))
	{
		LoadFromFile(Utils::StringTrim(preset).c_str());
	}
}

void CWeaponConfig::OnConfigSave(CProfile& cfg)
{
	if (m_bPresetChanging)
		return;

	std::string mainKeys = XorStr("Preset");

	std::stringstream builder;
	for (auto it = m_Rules.begin(); it != m_Rules.end(); ++it)
	{
		builder << it->m_name;
		if (it + 1 != m_Rules.end())
			builder << u8", ";
	}

	if(m_iRuleSelection != -1)
		cfg = m_DefaultRule;
	
	cfg.SetValue(mainKeys, XorStr("presets"), builder.str());

	if (m_iRuleSelection == -1)
		m_DefaultRule = cfg;
}

void CWeaponConfig::OnCreateMove(CUserCmd* cmd, bool*)
{
	if (m_bMenuOpen)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (weapon == nullptr || !weapon->IsValid())
		return;

	static FetchActiveRule_t lastData;
	FetchActiveRule_t curData;
	curData.m_weapon = weapon->GetWeaponName();
	curData.m_team = local->GetTeam();
	curData.m_speed = !!(cmd->buttons & IN_SPEED);
	curData.m_duck = !!(cmd->buttons & IN_DUCK);
	curData.m_score = !!(cmd->buttons & IN_SCORE);
	curData.m_flashlight = (cmd->impulse == 100);
	curData.m_caps = !!(GetKeyState(VK_CAPITAL) & 1);
	curData.m_scroll = !!(GetKeyState(VK_SCROLL) & 1);
	curData.m_num = !!(GetKeyState(VK_NUMLOCK) & 1);
	curData.m_attack = !!(cmd->buttons & IN_ATTACK);
	curData.m_attack2 = !!(cmd->buttons & IN_ATTACK2);
	curData.m_use = !!(cmd->buttons & IN_USE);
	if (lastData == curData)
		return;

	static size_t last = std::string::npos;
	size_t current = FetchActiveRule(curData);
	if (last == current)
		return;

	m_iRuleSelection = current;
	ChangePreset();
	last = current;
	lastData = curData;
}

void CWeaponConfig::SaveToFile(const char* preset)
{
	size_t id = FindRule(preset);
	if (id == std::string::npos)
		return;

	CProfile config;

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnConfigSave(config);

	const Rule_t& rule = m_Rules[id];
	std::string mainKeys = XorStr("Preset");
	config.SetValue(mainKeys, XorStr("name"), preset);
	config.SetValue(mainKeys, XorStr("order"), static_cast<int>(id));
	config.SetValue(mainKeys, XorStr("weapon"), rule.m_weapon);
	config.SetValue(mainKeys, XorStr("team"), rule.m_team);
	config.SetValue(mainKeys, XorStr("btn_speed"), rule.m_speed);
	config.SetValue(mainKeys, XorStr("btn_duck"), rule.m_duck);
	config.SetValue(mainKeys, XorStr("btn_score"), rule.m_score);
	config.SetValue(mainKeys, XorStr("btn_flashlight"), rule.m_flashlight);
	config.SetValue(mainKeys, XorStr("btn_caps"), rule.m_caps);
	config.SetValue(mainKeys, XorStr("btn_scroll"), rule.m_scroll);
	config.SetValue(mainKeys, XorStr("btn_num"), rule.m_num);
	config.SetValue(mainKeys, XorStr("btn_attack"), rule.m_attack);
	config.SetValue(mainKeys, XorStr("btn_attack2"), rule.m_attack2);
	config.SetValue(mainKeys, XorStr("btn_use"), rule.m_use);

	std::fstream file(Utils::BuildPath(XorStr("\\%s.ini"), preset), std::ios::out | std::ios::beg | std::ios::trunc);
	if (file.bad() || !file.is_open())
		return;

	config.Save(file);

	file.close();
}

void CWeaponConfig::LoadFromFile(const char* preset)
{
	std::fstream file(Utils::BuildPath(XorStr("\\%s.ini"), preset), std::ios::in | std::ios::beg);
	if (file.bad() || !file.is_open())
		return;

	CProfile config;
	config.Load(file);
	file.close();

	size_t id = FindRule(preset);
	if (id == std::string::npos)
	{
		m_Rules.emplace_back();
		id = m_Rules.size() - 1;
	}

	Rule_t& rule = m_Rules[id];
	std::string mainKeys = XorStr("Preset");
	rule.m_config = config;
	rule.m_name = preset;
	rule.m_weapon = config.GetString(mainKeys, XorStr("weapon"), rule.m_weapon);
	rule.m_team = config.GetInteger(mainKeys, XorStr("team"), rule.m_team);
	rule.m_speed = config.GetBoolean(mainKeys, XorStr("btn_speed"), rule.m_speed);
	rule.m_duck = config.GetBoolean(mainKeys, XorStr("btn_duck"), rule.m_duck);
	rule.m_score = config.GetBoolean(mainKeys, XorStr("btn_score"), rule.m_score);
	rule.m_flashlight = config.GetBoolean(mainKeys, XorStr("btn_flashlight"), rule.m_flashlight);
	rule.m_caps = config.GetBoolean(mainKeys, XorStr("btn_caps"), rule.m_caps);
	rule.m_scroll = config.GetBoolean(mainKeys, XorStr("btn_scroll"), rule.m_scroll);
	rule.m_num = config.GetBoolean(mainKeys, XorStr("btn_num"), rule.m_num);
	rule.m_attack = config.GetBoolean(mainKeys, XorStr("btn_attack"), rule.m_attack);
	rule.m_attack2 = config.GetBoolean(mainKeys, XorStr("btn_attack2"), rule.m_attack2);
	rule.m_use = config.GetBoolean(mainKeys, XorStr("btn_use"), rule.m_use);
	rule.m_order = id;
}

void CWeaponConfig::ChangePreset()
{
	CProfile* config = nullptr;
	if (m_iRuleSelection < 0 || m_iRuleSelection >= m_Rules.size())
		config = &m_DefaultRule;
	else
		config = &(m_Rules[m_iRuleSelection].m_config);

	m_bPresetChanging = true;
	
	for (auto inst : g_pClientHook->_GameHook)
		if (inst && inst.get() != this)
			inst->OnConfigLoading(*config);

	m_bPresetChanging = false;

	if (m_iRuleSelection != -1)
	{
		Rule_t& rule = m_Rules[m_iRuleSelection];
		strcpy_s(m_szCurrentName, rule.m_name.c_str());
		strcpy_s(m_szCurrentWeapon, rule.m_weapon.c_str());
		m_iCurrentTeam = rule.m_team;
		m_bInSpeed = rule.m_speed;
		m_bInDuck = rule.m_duck;
		m_bInScore = rule.m_score;
		m_bInFlashlight = rule.m_flashlight;
		m_bOnCapsLock = rule.m_caps;
		m_bOnScrollLock = rule.m_scroll;
		m_bOnNumLock = rule.m_num;
		m_bInAttack = rule.m_attack;
		m_bInAttack2 = rule.m_attack2;
		m_bInUse = rule.m_use;
	}
	else
	{
		m_szCurrentName[0] = '\0';
		m_szCurrentWeapon[0] = '\0';
		m_iCurrentTeam = 0;
		m_bInSpeed = false;
		m_bInDuck = false;
		m_bInScore = false;
		m_bInFlashlight = false;
		m_bOnCapsLock = false;
		m_bOnScrollLock = false;
		m_bOnNumLock = false;
		m_bInAttack = false;
		m_bInAttack2 = false;
		m_bInUse = false;
	}
}

void CWeaponConfig::AddOrUpdatePreset()
{
	if (m_szCurrentName[0] == '\0')
		return;

	size_t id = FindRule(m_szCurrentName);
	if (id == std::string::npos)
	{
		m_Rules.emplace_back();
		id = m_Rules.size() - 1;
		m_Rules[id].m_name = m_szCurrentName;
	}

	Rule_t& rule = m_Rules[id];

	rule.m_weapon = m_szCurrentWeapon;
	rule.m_team = m_iCurrentTeam;
	rule.m_speed = m_bInSpeed;
	rule.m_duck = m_bInDuck;
	rule.m_score = m_bInScore;
	rule.m_flashlight = m_bInFlashlight;
	rule.m_caps = m_bOnCapsLock;
	rule.m_scroll = m_bOnScrollLock;
	rule.m_num = m_bOnNumLock;
	rule.m_attack = m_bInAttack;
	rule.m_attack2 = m_bInAttack2;
	rule.m_use = m_bInUse;

	CProfile& config = m_Rules[id].m_config;

	m_bPresetChanging = true;

	for (auto inst : g_pClientHook->_GameHook)
		if (inst && inst.get() != this)
			inst->OnConfigSave(config);

	m_bPresetChanging = false;

	std::string mainKeys = XorStr("Preset");
	config.SetValue(mainKeys, XorStr("name"), rule.m_name);
	config.SetValue(mainKeys, XorStr("order"), static_cast<int>(id));
	config.SetValue(mainKeys, XorStr("weapon"), rule.m_weapon);
	config.SetValue(mainKeys, XorStr("team"), rule.m_team);
	config.SetValue(mainKeys, XorStr("btn_speed"), rule.m_speed);
	config.SetValue(mainKeys, XorStr("btn_duck"), rule.m_duck);
	config.SetValue(mainKeys, XorStr("btn_score"), rule.m_score);
	config.SetValue(mainKeys, XorStr("btn_flashlight"), rule.m_flashlight);
	config.SetValue(mainKeys, XorStr("btn_caps"), rule.m_caps);
	config.SetValue(mainKeys, XorStr("btn_scroll"), rule.m_scroll);
	config.SetValue(mainKeys, XorStr("btn_num"), rule.m_num);
	config.SetValue(mainKeys, XorStr("btn_attack"), rule.m_attack);
	config.SetValue(mainKeys, XorStr("btn_attack2"), rule.m_attack2);
	config.SetValue(mainKeys, XorStr("btn_use"), rule.m_use);

	SaveToFile(m_Rules[id].m_name.c_str());
	m_iRuleSelection = id;
}

void CWeaponConfig::RemovePreset()
{
	if (m_szCurrentName[0] == '\0')
		return;

	auto it = std::find_if(m_Rules.begin(), m_Rules.end(), [this](const Rule_t& v) { return v.m_name == m_szCurrentName; });
	if (it == m_Rules.end())
		return;

	m_Rules.erase(it);
	m_iRuleSelection = -1;
	ChangePreset();
}

void CWeaponConfig::MoveUp()
{
	if (m_szCurrentName[0] == '\0')
		return;

	auto it = std::find_if(m_Rules.begin(), m_Rules.end(), [this](const Rule_t& v) { return v.m_name == m_szCurrentName; });
	if (it == m_Rules.end() || it == m_Rules.begin())
		return;

	std::iter_swap(it, it - 1);
	m_iRuleSelection = std::distance(m_Rules.begin(), it - 1);
}

void CWeaponConfig::MoveDown()
{
	if (m_szCurrentName[0] == '\0')
		return;

	auto it = std::find_if(m_Rules.begin(), m_Rules.end(), [this](const Rule_t& v) { return v.m_name == m_szCurrentName; });
	if (it == m_Rules.end() || it + 1 == m_Rules.end())
		return;

	std::iter_swap(it, it + 1);
	m_iRuleSelection = std::distance(m_Rules.begin(), it + 1);
}

size_t CWeaponConfig::FindRule(std::string_view preset)
{
	auto it = std::find_if(m_Rules.begin(), m_Rules.end(), [preset](const Rule_t& v) { return v.m_name == preset; });
	if (it == m_Rules.end())
		return std::string::npos;

	return std::distance(m_Rules.begin(), it);
}

size_t CWeaponConfig::FetchActiveRule(const FetchActiveRule_t& data)
{
	size_t size = m_Rules.size();
	for (size_t i = 0; i < size; ++i)
	{
		Rule_t& rule = m_Rules[i];
		if (
			(rule.m_speed && !data.m_speed) ||
			(rule.m_duck && !data.m_duck) ||
			(rule.m_score && !data.m_score) ||
			(rule.m_flashlight && !data.m_flashlight) ||
			(rule.m_caps && !data.m_caps) ||
			(rule.m_scroll && !data.m_scroll) ||
			(rule.m_num && !data.m_num) ||
			(rule.m_attack && !data.m_attack) ||
			(rule.m_attack2 && !data.m_attack2) ||
			(rule.m_use && !data.m_use)
		)
			continue;

		if (rule.m_team != 0 && data.m_team != rule.m_team)
			continue;

		if (!Utils::StringEqual(data.m_weapon, rule.m_weapon, false))
			continue;

		return i;
	}

	return std::string::npos;
}

bool CWeaponConfig::Rule_t::operator<(const Rule_t& other)
{
	return m_order < other.m_order;
}

bool CWeaponConfig::FetchActiveRule_t::operator==(const FetchActiveRule_t& other) const
{
	return (
		m_weapon == other.m_weapon &&
		m_team == other.m_team &&
		m_speed == other.m_speed &&
		m_duck == other.m_duck &&
		m_score == other.m_score &&
		m_flashlight == other.m_flashlight &&
		m_caps == other.m_caps &&
		m_scroll == other.m_scroll &&
		m_num == other.m_num &&
		m_attack == other.m_attack &&
		m_attack2 == other.m_attack2 &&
		m_use == other.m_use
	);
}
