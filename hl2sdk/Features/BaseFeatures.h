﻿#pragma once
#include "../Interfaces/IBaseClientState.h"
#include "../interfaces.h"
#include "../Structs/baseplayer.h"
#include "../Structs/baseweapon.h"
#include "../../l4d2Simple2/drawing.h"
#include "../../l4d2Simple2/config.h"
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
	virtual void OnGameEventClient(IGameEvent*);
	virtual void OnRenderView(CViewSetup&);
	virtual void OnGameEvent(IGameEvent*, bool);
	virtual void OnDrawModel(DrawModelState_t&, ModelRenderInfo_t&, matrix3x4_t*);
	virtual void OnOverrideView(CViewSetup*);
	virtual bool OnGetViewModelFOV(float& fov);

	// 以下函数返回 false 阻止调用原函数
	// 如果不打算阻止原函数，则需要返回 true
	virtual bool OnSendMove();
	virtual bool OnUserMessage(int, bf_read);
	virtual bool OnProcessGetCvarValue(const std::string&, std::string&);
	virtual bool OnProcessSetConVar(const std::string&, std::string&);
	virtual bool OnProcessClientCommand(const std::string&);

	// 返回 true 使用修改后的参数来调用原函数
	// 如果不打算修改原函数调用，需要返回 false
	virtual bool OnFindMaterial(std::string&, std::string&);

	// 返回 true 使用修改后的参数来调用原函数
	// 如果不打算修改原函数调用，需要返回 false
	virtual bool OnEmitSound(std::string&, int&, int&, float&, SoundLevel_t&, int&, int&, Vector&, Vector&, bool&, float&);

	// 返回 true 使用修改后的参数来调用原函数
	// 如果不打算修改原函数调用，需要返回 false
	virtual bool OnSendNetMsg(INetMessage&, bool&, bool&);

	// 配置文件
	virtual void OnConfigLoading(CProfile&);
	virtual void OnConfigSave(CProfile&);
	virtual void OnMenuOpened();
	virtual void OnMenuClosed();

	// 实体创建和销毁
	virtual void OnEntityCreated(CBaseEntity*);
	virtual void OnEntityDeleted(CBaseEntity*);

private:
	size_t m_iHookIndex;

protected:
	bool m_bMenuOpen = false;
};

#define IMGUI_TIPS(_text)	if (ImGui::IsItemHovered()) ImGui::SetTooltip(XorStr(u8##_text))
