#pragma once
#include "../Interfaces/IBaseClientState.h"
#include "../interfaces.h"
#include "../Structs/baseplayer.h"
#include "../Structs/baseweapon.h"
#include "../../l4d2Simple2/drawing.h"
#include <imgui.h>

// 接口类
class CBaseFeatures
{
public:
	CBaseFeatures();
	virtual ~CBaseFeatures();

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual void OnShutdown();

	virtual void OnCreateMove(CUserCmd*, bool*);
	virtual void OnPaintTraverse(VPANEL);
	virtual void OnEnginePaint(PaintMode_t);
	virtual bool OnUserMessage(int, bf_read);
	virtual void OnFrameStageNotify(ClientFrameStage_t);
	virtual bool OnProcessGetCvarValue(SVC_GetCvarValue*, std::string&);
	virtual bool OnProcessSetConVar(NET_SetConVar*);
	virtual bool OnProcessClientCommand(NET_StringCmd*);
	virtual void OnSceneEnd();
	virtual void OnMenuDrawing();
	virtual void OnScreenDrawing();
	virtual bool OnSendMove();
	virtual bool OnFindMaterial(std::string&, std::string&);
	virtual void OnKeyInput(bool, ButtonCode_t, const char*);

private:
	size_t m_iHookIndex;
};
