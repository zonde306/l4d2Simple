#pragma once

class IGlobalVarsBase
{
public:
	float realtime;
	int framecount;
	float absoluteframetime;
	float absoluteframestarttimestddev;
	float curtime;
	float frametime;
	int maxClients;
	int tickcount;
	float interval_per_tick;
	float interpolation_amount;
	int simTicksThisFrame;
	int network_protocol;
	void* pSaveData;
	bool m_bClient;
	bool m_bRemoteClient;

private:
	int nTimestampNetworkingBase;
	int nTimestampRandomizeWindow;
};

class CGlobalVarsBase
{
public:
	CGlobalVarsBase(bool bIsClient);

	bool IsClient() const;
	int GetNetworkBase(int nTick, int nEntity);

public:
	float realtime;
	int framecount;
	float absoluteframetime;
	float absoluteframestarttimestddev;
	float curtime;
	float frametime;
	int maxClients;
	int tickcount;
	float interval_per_tick;
	float interpolation_amount;
	int simTicksThisFrame;
	int network_protocol;
	void* pSaveData;
	bool m_bClient;
	bool m_bRemoteClient;

private:
	int nTimestampNetworkingBase;
	int nTimestampRandomizeWindow;
};

class IPlayerInfoManager
{
public:
	virtual void* GetPlayerInfo(void* pEdict) = 0;
	virtual CGlobalVarsBase* GetGlobalVars() = 0;
};

inline int CGlobalVarsBase::GetNetworkBase(int nTick, int nEntity)
{
	int nEntityMod = nEntity % nTimestampRandomizeWindow;
	int nBaseTick = nTimestampNetworkingBase * (int)((nTick - nEntityMod) / nTimestampNetworkingBase);
	return nBaseTick;
}

inline CGlobalVarsBase::CGlobalVarsBase(bool bIsClient) : m_bClient(bIsClient), 
    nTimestampNetworkingBase(100), nTimestampRandomizeWindow(32) 
{

}

inline bool CGlobalVarsBase::IsClient() const
{
	return m_bClient;
}