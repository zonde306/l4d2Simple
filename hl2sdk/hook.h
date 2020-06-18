#pragma once
#include "interfaces.h"
#include "./Interfaces/IBaseClientState.h"
#include "./Features/BaseFeatures.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <map>
#include <optional>

// 虚函数
using FnPaintTraverse = void(__thiscall*)(IVPanel*, VPANEL, bool, bool);
using FnCreateMoveShared = bool(__thiscall*)(IClientMode*, float, CUserCmd*);
using FnCreateMove = void(__thiscall*)(IBaseClientDll*, int, float, bool);
using FnFrameStageNotify = void(__thiscall*)(IBaseClientDll*, ClientFrameStage_t);
using FnRunCommand = void(__thiscall*)(IPrediction*, CBaseEntity*, CUserCmd*, IMoveHelper*);
using FnDispatchUserMessage = bool(__thiscall*)(IBaseClientDll*, int, bf_read*);
using FnEnginePaint = void(__thiscall*)(IEngineVGui*, PaintMode_t);
using FnEngineKeyEvent = bool(__thiscall*)(IEngineVGui*, const InputEvent_t&);
using FnProcessGetCvarValue = bool(__thiscall*)(CBaseClientState*, SVC_GetCvarValue*);
using FnProcessSetConVar = bool(__thiscall*)(CBaseClientState*, NET_SetConVar*);
using FnProccessStringCmd = bool(__thiscall*)(CBaseClientState*, NET_StringCmd*);
using FnWriteUsercmdDeltaToBuffer = bool(__thiscall*)(IBaseClientDll*, bf_write*, int, int, bool);
using FnSceneEnd = void(__thiscall*)(IVRenderView*);
using FnFindMaterial = IMaterial*(__thiscall*)(IMaterialSystem*, char const*, const char*, bool, const char*);
using FnKeyInput = int(__thiscall*)(IClientMode*, int, ButtonCode_t, const char*);
using FnFireEventClientSide = bool(__thiscall*)(IGameEventManager2*, IGameEvent*);
using FnRenderView = void(__thiscall*)(IBaseClientDll*, const CViewSetup&, int, int);
using FnFireEvent = bool(__thiscall*)(IGameEventManager2*, IGameEvent*, bool);
using FnDrawModelExecute = void(__thiscall*)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
using FnEmitSound = void(__thiscall*)(IEngineSound*, IRecipientFilter&, int, int, const char*, float, SoundLevel_t, int, int, const Vector*, const Vector*, CUtlVector<Vector>*, bool, float, int);
using FnSendNetMsg = bool(__thiscall*)(INetChannel*, INetMessage&, bool, bool);
using FnOverrideView = void(__thiscall*)(IClientMode*, CViewSetup*);
using FnGetViewModelFOV = float(__thiscall*)(IClientMode*);

// 非虚函数
typedef void(__cdecl* FnCL_SendMove)();
typedef void(__cdecl* FnCL_Move)(float, bool);

class CClientHook
{
public:
	~CClientHook();

	bool Init();
	bool UninstallHook();
	void Shutdown();

	void InstallClientStateHook(CBaseClientState* pointer);
	void InstallClientModeHook(IClientMode* pointer);

protected:
	static void __cdecl Hooked_CL_Move(float, bool);
	static void __cdecl Call_CL_Move(DWORD _edi, DWORD _esi, float accumulated_extra_samples, bool bFinalTick);
	static void __cdecl Hooked_CL_SendMove();
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
	static bool __fastcall Hooked_WriteUsercmdDeltaToBuffer(IBaseClientDll*, LPVOID, int, bf_write*, int, int, bool);
	static void __fastcall Hooked_SceneEnd(IVRenderView*, LPVOID);
	static IMaterial* __fastcall Hooked_FindMaterial(IMaterialSystem*, LPVOID, char const*, const char*, bool, const char*);
	static int __fastcall Hooked_KeyInput(IClientMode*, LPVOID, int, ButtonCode_t, const char*);
	static bool __fastcall Hooked_FireEventClientSide(IGameEventManager2*, LPVOID, IGameEvent*);
	static void __fastcall Hooked_RenderView(IBaseClientDll*, LPVOID, const CViewSetup&, int, int);
	static bool __fastcall Hooked_FireEvent(IGameEventManager2*, LPVOID, IGameEvent*, bool);
	static void __fastcall Hooked_DrawModelExecute(IVModelRender*, LPVOID, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	static void __fastcall Hooked_EmitSound(IEngineSound*, LPVOID, IRecipientFilter&, int, int, const char*, float, SoundLevel_t, int, int, const Vector*, const Vector*, CUtlVector<Vector>*, bool, float, int);
	static bool __fastcall Hooked_SendNetMsg(INetChannel*, LPVOID, INetMessage&, bool, bool);
	static void __fastcall Hooked_OverrideView(IClientMode*, LPVOID, CViewSetup*);
	static float __fastcall Hooked_GetViewModelFOV(IClientMode*, LPVOID);

public:
	std::vector<std::shared_ptr<CBaseFeatures>> _GameHook;

public:
	void InitFeature();
	void LoadConfig();
	void SaveConfig();

	ConVar* GetDummyConVar(const std::string& cvar, const std::optional<std::string>& value = {});
	bool RestoreDummyConVar(const std::string& cvar);

private:
	// 被 Hook 后的原函数
	FnCL_Move oCL_Move = nullptr;
	FnCL_SendMove oCL_SendMove = nullptr;
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
	FnKeyInput oKeyInput = nullptr;
	FnFireEventClientSide oFireEventClientSide = nullptr;
	FnRenderView oRenderView = nullptr;
	FnFireEvent oFireEvent = nullptr;
	FnDrawModelExecute oDrawModelExecute = nullptr;
	FnEmitSound oEmitSound = nullptr;
	FnOverrideView oOverrideView = nullptr;
	FnGetViewModelFOV oGetViewModelFOV = nullptr;

public:
	bool* bSendPacket;
	FnFindMaterial oFindMaterial = nullptr;
	FnSendNetMsg oSendNetMsg = nullptr;

private:
	bool bCreateMoveFinish = false;
};

extern std::unique_ptr<CClientHook> g_pClientHook;

#undef GetServerTime
#undef GetLocalPlayer
#undef GetCurrentTime
class CBasePlayer;

class CClientPrediction
{
public:
	void Init();

	bool StartPrediction(CUserCmd* cmd);
	bool FinishPrediction();

	float GetServerTime();
	CBasePlayer* GetLocalPlayer();
	std::pair<float, float> GetWeaponSpread(int seed, CBaseWeapon* weapon);
	float GetCurrentTime(CUserCmd* cmd);

	// 改名字
	// 游戏自带的 setinfo name 和 name 并不能动态改名
	void SetName(const char* name, ...);

	inline int GetFlags() { return m_iFlags; };

private:
	CMoveData m_MoveData;
	int m_iFlags = 0;
	float m_fCurTime = 0.0f;
	float m_fFrameTime = 0.0f;
	int* m_pSpreadRandomSeed = nullptr;
	int* m_pPredictionRandomSeed = nullptr;
	int m_iTickBase = 0;
	bool m_bInPrediction = false;
};

extern std::unique_ptr<CClientPrediction> g_pClientPrediction;
extern std::map<std::string, std::string> g_ServerConVar;
