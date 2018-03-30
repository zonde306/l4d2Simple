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

bool CBaseFeatures::OnProcessGetCvarValue(SVC_GetCvarValue *, std::string&)
{
	return true;
}

bool CBaseFeatures::OnProcessSetConVar(NET_SetConVar *)
{
	return true;
}

bool CBaseFeatures::OnProcessClientCommand(NET_StringCmd *)
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
	return false;
}

void CBaseFeatures::OnKeyInput(bool, ButtonCode_t, const char *)
{
}

void CBaseFeatures::OnGameEvent(IGameEvent *)
{
}

void CBaseFeatures::OnRenderView(CViewSetup &)
{
}
