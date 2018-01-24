#include "BaseFeatures.h"
#include "../hook.h"

CBaseFeatures::CBaseFeatures()
{
	hook::_GameHook.emplace_back(this);
	m_iHookIndex = hook::_GameHook.size() - 1;
}

CBaseFeatures::~CBaseFeatures()
{
	hook::_GameHook.erase(hook::_GameHook.begin() + m_iHookIndex);
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

void CBaseFeatures::OnMenuDrawing()
{
}

void CBaseFeatures::OnScreenDrawing()
{
}
