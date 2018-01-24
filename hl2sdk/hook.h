#pragma once
#include "./Features/BaseFeatures.h"
#include "interfaces.h"
#include "./Interfaces/IBaseClientState.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

typedef void(__cdecl* FnCL_SendMove)();
typedef void(__cdecl* FnCL_Move)(float, bool);
typedef void(__cdecl* FnWriteUsercmd)(bf_write*, CUserCmd*, CUserCmd*);
typedef void(__thiscall* FnPaintTraverse)(IVPanel*, VPANEL, bool, bool);
typedef bool(__thiscall* FnCreateMoveShared)(IClientMode*, float, CUserCmd*);
typedef void(__thiscall* FnCreateMove)(IBaseClientDll*, int, float, bool);
typedef void(__thiscall* FnFrameStageNotify)(IBaseClientDll*, ClientFrameStage_t);
typedef void(__thiscall* FnRunCommand)(IPrediction*, CBaseEntity*, CUserCmd*, IMoveHelper*);
typedef bool(__thiscall* FnDispatchUserMessage)(IBaseClientDll*, int, bf_read*);
typedef void(__thiscall* FnEnginePaint)(IEngineVGui*, PaintMode_t);
typedef bool(__thiscall* FnEngineKeyEvent)(IEngineVGui*, const InputEvent_t&);
typedef bool(__thiscall* FnProcessGetCvarValue)(CBaseClientState*, SVC_GetCvarValue*);
typedef bool(__thiscall* FnProcessSetConVar)(CBaseClientState*, NET_SetConVar*);
typedef bool(__thiscall* FnProccessStringCmd)(CBaseClientState*, NET_StringCmd*);
typedef bool(__thiscall* FnWriteUsercmdDeltaToBuffer)(IBaseClientDll*, bf_write*, int, int, bool);

// 通过修改参数 CUserCmd* 来实现各种操作，修改 bool* 以实现隐藏修改的操作
// 如果 CUserCmd::viewangles 与 IEngineClient::GetViewAngles 不同则需要修复
typedef void(__cdecl* FnHookCreateMove)(CBaseEntity*, CUserCmd*, bool*);
typedef void(__cdecl* FnHookCreateMoveShared)(CBaseEntity*, CUserCmd*, bool*);

// 使用 interfaces::Surface 来绘制屏幕
// 传入参数如果是 true 则为 FocusOverlayPanel 否则为 MatSystemTopPanel
typedef void(__cdecl* FnHookPaintTraverse)(bool);
typedef void(__cdecl* FnHookEnginePaint)();

// 返回 false 可以阻止消息发送到客户端
typedef bool(__cdecl* FnHookUserMessage)(int, bf_read);

// 在 ClientFrameStage_t::FRAME_RENDER_START 使用 interfaces::DebugOverlay 进行 3D 绘制
// 在 ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START 进行 RCS
typedef void(__cdecl* FnOnFrameStageNotify)(ClientFrameStage_t);

// 返回空字符串不做任何事情，否则修改查询返回值
typedef std::string(__cdecl* FnOnProcessGetCvarValue)(SVC_GetCvarValue*);

// 返回 false 阻止修改 ConVar
typedef bool(__cdecl* FnOnProcessSetConVar)(NET_SetConVar*);

// 返回 false 阻止执行命令
typedef bool(__cdecl* FnOnProcessStringCmd)(NET_StringCmd*);

namespace hook
{
	bool InstallHook();
	bool UninstallHook();

	extern std::vector<std::shared_ptr<CBaseFeatures>> _GameHook;
	extern std::vector<FnHookCreateMove> _CreateMove;
	extern std::vector<FnHookCreateMoveShared> _CreateMoveShared;
	extern std::vector<FnHookPaintTraverse> _PaintTraverse;
	extern std::vector<FnHookEnginePaint> _EnginePaint;
	extern std::vector<FnHookUserMessage> _UserMessage;
	extern std::vector<FnOnFrameStageNotify> _FrameStageNotify;
	extern std::vector<FnOnProcessGetCvarValue> _ProcessGetCvarValue;
	extern std::vector<FnOnProcessSetConVar> _ProcessSetConVar;
	extern std::vector<FnOnProcessStringCmd> _ProcessStringCmd;

	extern FnCL_Move oCL_Move;
	extern FnPaintTraverse oPaintTraverse;
	extern FnCreateMoveShared oCreateMoveShared;
	extern FnCreateMove oCreateMove;
	extern FnFrameStageNotify oFrameStageNotify;
	extern FnRunCommand oRunCommand;
	extern FnDispatchUserMessage oDispatchUserMessage;
	extern FnEnginePaint oEnginePaint;
	extern FnEngineKeyEvent oEngineKeyEvent;
	extern FnProcessGetCvarValue oProcessGetCvarValue;
	extern FnProcessSetConVar oProcessSetConVar;
	extern FnProccessStringCmd oProccessStringCmd;
	extern FnWriteUsercmdDeltaToBuffer oWriteUsercmdDeltaToBuffer;

	extern bool* bSendPacket;
}
