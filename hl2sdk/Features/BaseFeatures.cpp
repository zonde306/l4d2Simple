#include "BaseFeatures.h"
#include "../hook.h"

CBaseFeatures::CBaseFeatures()
{
	g_pClientHook->_GameHook.emplace_back(this);
	m_iHookIndex = g_pClientHook->_GameHook.size() - 1;
}

CBaseFeatures::~CBaseFeatures()
{
	g_pClientHook->_GameHook.erase(g_pClientHook->_GameHook.begin() + m_iHookIndex);
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
	return false;
}

void CBaseFeatures::OnFrameStageNotify(ClientFrameStage_t)
{
}

bool CBaseFeatures::OnProcessGetCvarValue(SVC_GetCvarValue *, std::string&)
{
	return false;
}

bool CBaseFeatures::OnProcessSetConVar(NET_SetConVar *)
{
	return false;
}

bool CBaseFeatures::OnProcessClientCommand(NET_StringCmd *)
{
	return false;
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
