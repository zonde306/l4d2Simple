#pragma once
#include "interfaces.h"
#include "./Interfaces/IBaseClientState.h"
#include "./Features/BaseFeatures.h"
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

class CClientHook
{
public:
	bool Init();
	bool UninstallHook();

	void InstallClientStateHook(CBaseClientState* pointer);
	void InstallClientModeHook(IClientMode* pointer);

protected:
	static void __cdecl Hooked_CL_Move(float, bool);
	static void __fastcall Hooked_PaintTraverse(IVPanel*, LPVOID, VPANEL, bool, bool);
	static bool __fastcall Hooked_CreateMoveShared(IClientMode*, LPVOID, float, CUserCmd*);
	static void __fastcall Hooked_CreateMove(IBaseClientDll*, LPVOID, int, float, bool);
	static void __fastcall Hooked_FrameStageNotify(IBaseClientDll*, LPVOID, ClientFrameStage_t);
	static void __fastcall Hooked_RunCommand(IPrediction*, LPVOID, CBaseEntity*, CUserCmd*, IMoveHelper*);
	static bool __fastcall Hooked_DispatchUserMessage(IBaseClientDll*, LPVOID, int, bf_read*);
	static void __fastcall Hooked_EnginePaint(IEngineVGui*, LPVOID, PaintMode_t);
	static bool __fastcall Hooked_ProcessGetCvarValue(CBaseClientState*, LPVOID, SVC_GetCvarValue*);
	static bool __fastcall Hooked_ProcessSetConVar(CBaseClientState*, LPVOID, NET_SetConVar*);
	static bool __fastcall Hooked_ProcessStringCmd(CBaseClientState*, LPVOID, NET_StringCmd*);
	static bool __fastcall Hooked_WriteUsercmdDeltaToBuffer(IBaseClientDll*, LPVOID, bf_write*, int, int, bool);
	static void __fastcall Hooked_SceneEnd(IVRenderView*, LPVOID);

public:
	std::vector<std::shared_ptr<CBaseFeatures>> _GameHook;
	
private:
	// 被 Hook 后的原函数
	FnCL_Move oCL_Move = nullptr;
	FnPaintTraverse oPaintTraverse = nullptr;
	FnCreateMoveShared oCreateMoveShared = nullptr;
	FnCreateMove oCreateMove = nullptr;
	FnFrameStageNotify oFrameStageNotify = nullptr;
	FnRunCommand oRunCommand = nullptr;
	FnDispatchUserMessage oDispatchUserMessage = nullptr;
	FnEnginePaint oEnginePaint = nullptr;
	FnEngineKeyEvent oEngineKeyEvent = nullptr;
	FnProcessGetCvarValue oProcessGetCvarValue = nullptr;
	FnProcessSetConVar oProcessSetConVar = nullptr;
	FnProccessStringCmd oProccessStringCmd = nullptr;
	FnWriteUsercmdDeltaToBuffer oWriteUsercmdDeltaToBuffer = nullptr;
	FnSceneEnd oSceneEnd = nullptr;

public:
	bool* bSendPacket;

	// 搜索特征码得到的函数
	FnStartDrawing StartDrawing;
	FnFinishDrawing FinishDrawing;
	FnCL_SendMove CL_SendMove;
	FnWriteUsercmd WriteUserCmd;

	// 通过导出表得到的函数
	FnRandomSeed RandomSeed;
	FnRandomFloat RandomFloat;
	FnRandomFloatExp RandomFloatExp;
	FnRandomInt RandomInt;
	FnRandomGaussianFloat RandomGaussianFloat;
	FnInstallUniformRandomStream InstallUniformRandomStream;

private:
	bool bCreateMoveFinish = false;
	std::map<std::string, std::string> m_serverConVar;
};

extern std::unique_ptr<CClientHook> g_pClientHook;
