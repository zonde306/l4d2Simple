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
	virtual void OnFrameStageNotify(ClientFrameStage_t);
	virtual void OnSceneEnd();
	virtual void OnMenuDrawing();
	virtual void OnScreenDrawing();
	virtual void OnKeyInput(bool, ButtonCode_t, const char*);
	virtual void OnGameEvent(IGameEvent*);

	// 以下函数返回 false 阻止调用原函数
	// 如果不打算阻止原函数，则需要返回 true
	virtual bool OnSendMove();
	virtual bool OnUserMessage(int, bf_read);
	virtual bool OnProcessGetCvarValue(SVC_GetCvarValue*, std::string&);
	virtual bool OnProcessSetConVar(NET_SetConVar*);
	virtual bool OnProcessClientCommand(NET_StringCmd*);

	// 返回 true 使用修改后的参数来调用原函数
	// 如果不打算修改原函数调用，需要返回 false
	virtual bool OnFindMaterial(std::string&, std::string&);

private:
	size_t m_iHookIndex;
};
