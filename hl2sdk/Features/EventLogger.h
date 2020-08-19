#pragma once
#include "BaseFeatures.h"

class CGEL_EventLogger;
class CEventLogger : public CBaseFeatures
{
public:
	CEventLogger();
	~CEventLogger();
	friend CGEL_EventLogger;

	virtual void OnMenuDrawing() override;
	virtual void OnGameEventClient(IGameEvent* event) override;
	virtual void OnGameEvent(IGameEvent* event, bool dontBroadcast) override;
	virtual bool OnUserMessage(int id, bf_read bf) override;

protected:
	void OnPlayerSpawn(int client);
	void OnPlayerDeath(int victim, int attacker);
	void OnPlayerTeam(int client, int oldTeam, int newTeam);
	void OnPlayerTakeDamage(int victim, int attacker, int damage);

private:
	bool m_bActive = true;
	bool m_bLogSpawn = true;
	bool m_bLogDeath = true;
	bool m_bLogTeam = true;
	bool m_bLogTakeDamage = true;
	bool m_bLogUserMsg = false;

private:
	CGEL_EventLogger *m_pEventListener;
};

extern CEventLogger* g_pEventLogger;

class CGEL_EventLogger : public IGameEventListener2
{
public:
	friend CEventLogger;
	
	virtual void FireGameEvent(IGameEvent *event) override;

private:
	CEventLogger* m_pParent = nullptr;
};
