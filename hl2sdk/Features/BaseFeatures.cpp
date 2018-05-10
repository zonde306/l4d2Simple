#include "BaseFeatures.h"
#include "../hook.h"

CBaseFeatures::CBaseFeatures()
{
	g_pClientHook->_GameHook.emplace_back(this);
	m_iHookIndex = g_pClientHook->_GameHook.size() - 1;
}

CBaseFeatures::~CBaseFeatures()
{
	// g_pClientHook->_GameHook.erase(g_pClientHook->_GameHook.begin() + m_iHookIndex);
}

void CBaseFeatures::OnConnect()
{
}

void CBaseFeatures::OnDisconnect()
{
}

void CBaseFeatures::OnShutdown()
{
}

void CBaseFeatures::OnCreateMove(CUserCmd *, bool *)
{
}

void CBaseFeatures::OnPaintTraverse(VPANEL)
{
}

void CBaseFeatures::OnEnginePaint(PaintMode_t)
{
}

bool CBaseFeatures::OnUserMessage(int, bf_read)
{
	return true;
}

void CBaseFeatures::OnFrameStageNotify(ClientFrameStage_t)
{
}

bool CBaseFeatures::OnProcessGetCvarValue(const std::string&, std::string&)
{
	return true;
}

bool CBaseFeatures::OnProcessSetConVar(const std::string&, std::string&)
{
	return true;
}

bool CBaseFeatures::OnProcessClientCommand(const std::string&)
{
	return true;
}

void CBaseFeatures::OnSceneEnd()
{
}

void CBaseFeatures::OnMenuDrawing()
{
}

void CBaseFeatures::OnScreenDrawing()
{
}

bool CBaseFeatures::OnSendMove()
{
	return true;
}

bool CBaseFeatures::OnFindMaterial(std::string &, std::string &)
{
	return true;
}

bool CBaseFeatures::OnEmitSound(std::string &, int &, int &, float &, SoundLevel_t &, int &, int &, Vector &, Vector &, bool &, float &)
{
	return true;
}

bool CBaseFeatures::OnSendNetMsg(INetMessage &, bool &, bool &)
{
	return true;
}

void CBaseFeatures::OnConfigLoading(const config_type &)
{
}

void CBaseFeatures::OnConfigSave(config_type &)
{
}

void CBaseFeatures::OnKeyInput(bool, ButtonCode_t, const char *)
{
}

void CBaseFeatures::OnGameEventClient(IGameEvent *)
{
}

void CBaseFeatures::OnRenderView(CViewSetup &)
{
}

void CBaseFeatures::OnGameEvent(IGameEvent *, bool)
{
}

void CBaseFeatures::OnDrawModel(DrawModelState_t &, ModelRenderInfo_t &, matrix3x4_t *)
{
}
