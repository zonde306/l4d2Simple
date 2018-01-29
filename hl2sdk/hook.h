#pragma once
#include "./Features/BaseFeatures.h"
#include "interfaces.h"
#include "./Interfaces/IBaseClientState.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

// 虚函数
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
typedef void(__thiscall* FnSceneEnd)(IVRenderView*);

// 非虚函数
typedef void(__thiscall* FnStartDrawing)(ISurface*);
typedef void(__thiscall* FnFinishDrawing)(ISurface*);
typedef void(__cdecl* FnCL_SendMove)();
typedef void(__cdecl* FnCL_Move)(float, bool);
typedef void(__cdecl* FnWriteUsercmd)(bf_write*, CUserCmd*, CUserCmd*);

// 导出函数
typedef void(__cdecl* FnRandomSeed)(int iSeed);
typedef float(__cdecl* FnRandomFloat)(float flMinVal, float flMaxVal);
typedef float(__cdecl* FnRandomFloatExp)(float flMinVal, float flMaxVal, float flExponent);
typedef int(__cdecl* FnRandomInt)(int iMinVal, int iMaxVal);
typedef float(__cdecl* FnRandomGaussianFloat)(float flMean, float flStdDev);
typedef void(__cdecl* FnInstallUniformRandomStream)(IUniformRandomStream* pStream);

namespace hook
{
	bool InstallHook();
	bool UninstallHook();

	extern std::vector<std::shared_ptr<CBaseFeatures>> _GameHook;
	
	// 被 Hook 后的原函数
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
	extern FnSceneEnd oSceneEnd;

	extern bool* bSendPacket;

	// 搜索特征码得到的函数
	extern FnStartDrawing StartDrawing;
	extern FnFinishDrawing FinishDrawing;
	extern FnCL_SendMove CL_SendMove;
	extern FnWriteUsercmd WriteUserCmd;

	// 通过导出表得到的函数
	extern FnRandomSeed RandomSeed;
	extern FnRandomFloat RandomFloat;
	extern FnRandomFloatExp RandomFloatExp;
	extern FnRandomInt RandomInt;
	extern FnRandomGaussianFloat RandomGaussianFloat;
	extern FnInstallUniformRandomStream InstallUniformRandomStream;
}
