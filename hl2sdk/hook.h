#pragma once
#include "interfaces.h"
#include "./Interfaces/IBaseClientState.h"
#include "./Features/BaseFeatures.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <optional>

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
typedef IMaterial*(__thiscall* FnFindMaterial)(IMaterialSystem*, char const*, const char*, bool, const char*);
typedef int(__thiscall* FnKeyInput)(IClientMode*, int, ButtonCode_t, const char*);
typedef bool(__thiscall* FnFireEventClientSide)(IGameEventManager2*, IGameEvent*);
typedef void(__thiscall* FnRenderView)(IBaseClientDll*, const CViewSetup&, int, int);
typedef bool(__thiscall* FnFireEvent)(IGameEventManager2*, IGameEvent*, bool);
typedef void(__thiscall* FnDrawModelExecute)(IVModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
typedef void(__thiscall* FnEmitSound)(IEngineSound*, IRecipientFilter&, int, int, const char*, float, SoundLevel_t, int, int, const Vector*, const Vector*, CUtlVector<Vector>*, bool, float, int);
typedef bool(__thiscall* FnSendNetMsg)(INetChannel*, INetMessage&, bool, bool);

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
