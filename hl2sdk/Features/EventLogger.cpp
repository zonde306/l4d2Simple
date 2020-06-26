#include "EventLogger.h"
#include "../hook.h"

CEventLogger* g_pEventLogger = nullptr;

CEventLogger::CEventLogger() : CBaseFeatures::CBaseFeatures()
{
	m_pEventListener = new CGEL_EventLogger();
	m_pEventListener->m_pParent = this;

	//g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_spawn"), false);
	//g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_first_spawn"), false);
	//g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_death"), false);
	//g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_hurt"), false);
	//g_pInterface->GameEvent->AddListener(m_pEventListener, XorStr("player_team"), false);
	g_pClientHook->m_ProtectedEventListeners.insert(m_pEventListener);
}

CEventLogger::~CEventLogger()
{
	g_pInterface->GameEvent->RemoveListener(m_pEventListener);
	delete m_pEventListener;
	
	CBaseFeatures::~CBaseFeatures();
}

void CEventLogger::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Event Logger")))
		return;

	ImGui::Checkbox(XorStr("EventLogger Active"), &m_bActive);
	
	ImGui::Separator();
	ImGui::Checkbox(XorStr("Subscription Spawn/Respawn"), &m_bLogSpawn);
	ImGui::Checkbox(XorStr("Subscription Death/Killed"), &m_bLogDeath);
	ImGui::Checkbox(XorStr("Subscription Change Team"), &m_bLogTeam);
	ImGui::Checkbox(XorStr("Subscription Attack Damage"), &m_bLogTakeDamage);

	ImGui::TreePop();
}

void CEventLogger::OnGameEventClient(IGameEvent* event)
{
	std::string_view name = event->GetName();
	if (name == XorStr("player_spawn") || name == XorStr("player_first_spawn") || name == XorStr("player_death") ||
		name == XorStr("player_hurt") || name == XorStr("player_team"))
		m_pEventListener->FireGameEvent(event);
}

void CEventLogger::OnGameEvent(IGameEvent* event, bool dontBroadcast)
{
	std::string_view name = event->GetName();
	if (name == XorStr("player_spawn") || name == XorStr("player_first_spawn") || name == XorStr("player_death") ||
		name == XorStr("player_hurt") || name == XorStr("player_team"))
		m_pEventListener->FireGameEvent(event);
}

void CEventLogger::OnPlayerSpawn(int client)
{
	if (!m_bActive)
		return;

	CBasePlayer* player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(client));
	if (player == nullptr)
		return;

	std::string character = player->GetCharacterName();
	std::string name = player->GetName();
	if (character.empty() || name.empty())
		return;

	Utils::log(XorStr("player %s (%s) spawnning."), name.c_str(), character.c_str());

	if (m_bLogSpawn)
	{
		if (player->IsSurvivor())
			g_pDrawing->PrintInfo(CDrawing::SKYBLUE, XorStr("survivor %s (%s) repsawn"), name.c_str(), character.c_str());
		else
			g_pDrawing->PrintInfo(CDrawing::RED, XorStr("infected %s (%s) repsawn"), name.c_str(), character.c_str());
	}
}

void CEventLogger::OnPlayerDeath(int victim, int attacker)
{
	if (!m_bActive)
		return;

	CBasePlayer* player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(victim));
	if (player == nullptr)
		return;

	std::string character = player->GetCharacterName();
	std::string name = player->GetName();
	if (character.empty() || name.empty())
		return;

	Utils::log(XorStr("player %s (%s) die."), name.c_str(), character.c_str());

	if (m_bLogDeath)
	{
		if (player->IsSurvivor())
			g_pDrawing->PrintInfo(CDrawing::SKYBLUE, XorStr("survivor %s (%s) die"), name.c_str(), character.c_str());
		else
			g_pDrawing->PrintInfo(CDrawing::RED, XorStr("infected %s (%s) killed"), name.c_str(), character.c_str());
	}
}

void CEventLogger::OnPlayerTeam(int client, int oldTeam, int newTeam)
{
	if (!m_bActive)
		return;

	CBasePlayer* player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(client));
	if (player == nullptr)
		return;

	std::string name = player->GetName();
	if (name.empty())
		return;

	if (oldTeam == 0 && newTeam > 0)
		Utils::log(XorStr("player %s join game."), name.c_str());
	else if(oldTeam > 0 && newTeam == 0)
		Utils::log(XorStr("player %s left game."), name.c_str());
	else
		Utils::log(XorStr("player %s join team %d"), name.c_str(), newTeam);

	if (m_bLogTeam)
	{
		switch (newTeam)
		{
			case 0:
			{
				if (oldTeam == 0)
					break;
				
				g_pDrawing->PrintInfo(CDrawing::YELLOW, XorStr("player %s left game"), name.c_str());
				break;
			}
			case 1:
			{
				if (oldTeam == 2)
					g_pDrawing->PrintInfo(CDrawing::SKYBLUE, XorStr("survivor %s idle"), name.c_str());
				else if(oldTeam == 3)
					g_pDrawing->PrintInfo(CDrawing::RED, XorStr("infected %s idle"), name.c_str());
				
				break;
			}
			case 2:
			{
				if (oldTeam == 1)
					g_pDrawing->PrintInfo(CDrawing::SKYBLUE, XorStr("spectator %s become survivor"), name.c_str());
				else if (oldTeam == 3)
					g_pDrawing->PrintInfo(CDrawing::RED, XorStr("infected %s rebel to survivor"), name.c_str());

				break;
			}
			case 3:
			{
				if (oldTeam == 1)
					g_pDrawing->PrintInfo(CDrawing::SKYBLUE, XorStr("spectator %s become infected"), name.c_str());
				else if (oldTeam == 2)
					g_pDrawing->PrintInfo(CDrawing::RED, XorStr("survivor %s rebel to infected"), name.c_str());

				break;
			}
		}
	}
}

void CEventLogger::OnPlayerTakeDamage(int victim, int attacker, int damage)
{
	if (!m_bActive)
		return;

	if (attacker != g_pInterface->Engine->GetLocalPlayer())
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	CBasePlayer* player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(victim));
	if (local == nullptr || player == nullptr || !local->IsAlive() || !player->IsAlive())
		return;

	if (m_bLogTakeDamage && player->IsSpecialInfected())
		g_pInterface->Engine->ClientCmd_Unrestricted(XorStr("play \"ui/littlereward.wav\""));
}

void CGEL_EventLogger::FireGameEvent(IGameEvent * event)
{
	const char* eventName = event->GetName();
	if (!_stricmp(eventName, XorStr("player_spawn")) || !_stricmp(eventName, XorStr("player_first_spawn")))
	{
		int client = g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("userid")));
		if (client <= 0 || client > 64)
			return;

		m_pParent->OnPlayerSpawn(client);
		return;
	}

	if (!_stricmp(eventName, XorStr("player_death")))
	{
		int victim = g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("userid")));
		if (victim <= 0 || victim > 64)
			return;

		int attacker = g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("attacker")));
		if (attacker <= 0 || attacker > 64)
			attacker = event->GetInt(XorStr("attackerentid"));
		if (attacker <= 0 || attacker > g_pInterface->EntList->GetHighestEntityIndex())
			attacker = -1;

		m_pParent->OnPlayerDeath(victim, attacker);
		return;
	}

	if (!_stricmp(eventName, XorStr("player_team")))
	{
		if (event->GetBool(XorStr("isbot")))
			return;
		
		int client = g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("userid")));
		if (client <= 0 || client > 64)
			return;

		m_pParent->OnPlayerTeam(client, event->GetInt(XorStr("oldteam")), event->GetInt(XorStr("team")));
		return;
	}

	if (!_stricmp(eventName, XorStr("player_hurt")))
	{
		int damage = event->GetInt(XorStr("dmg_health")) + event->GetInt(XorStr("dmg_armor"));
		if (damage <= 0)
			return;

		int victim = g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("userid")));
		if (victim <= 0 || victim > 64)
			return;

		int attacker = g_pInterface->Engine->GetPlayerForUserID(event->GetInt(XorStr("attacker")));
		if (attacker <= 0 || attacker > 64)
			attacker = event->GetInt(XorStr("attackerentid"));
		if (attacker <= 0 || attacker > g_pInterface->EntList->GetHighestEntityIndex())
			attacker = -1;

		m_pParent->OnPlayerTakeDamage(victim, attacker, damage);
		return;
	}
}
