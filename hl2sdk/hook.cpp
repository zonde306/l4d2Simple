﻿#include "hook.h"
#include "indexes.h"
#include "./Utils/math.h"
#include "./Utils/checksum_md5.h"
#include "./Structs/convar.h"
#include "./Structs/baseplayerresource.h"
#include "./Features/BunnyHop.h"
#include "./Features/SpeedHacker.h"
#include "./Features/TriggerBot.h"
#include "./Features/Aimbot.h"
#include "./Features/NoRecoilSpread.h"
#include "./Features/Knifebot.h"
#include "./Features/Visual.h"
#include "./Features/DropVisual.h"
#include "./Features/AntiAntiCheat.h"
#include "./Features/HackvsHack.h"
#include "./Features/EventLogger.h"
#include "./Features/QTE.h"
#include "./Features/WeaponConfig.h"
#include "../l4d2Simple2/vmt.h"
#include "../l4d2Simple2/xorstr.h"
#include "../detours/detourxs.h"
#include "../l4d2Simple2/config.h"
#include <memory>
#include <fstream>
#include <cctype>

std::unique_ptr<CClientHook> g_pClientHook;
std::unique_ptr<CClientPrediction> g_pClientPrediction;
std::map<std::string, std::string> g_ServerConVar;
std::map<std::string, std::unique_ptr<SpoofedConvar>> g_DummyConVar;
extern const VMatrix* g_pWorldToScreenMatrix;
static CLC_ListenEvents g_ListenEvents;

// 计时
extern time_t g_tpPlayingTimer;
extern time_t g_tpGameTimer;

#define SIG_CL_MOVE					XorStr("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 56 E8 ? ? ? ? 8B F0 83 7E 68 02 0F 8C ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? A1 ? ? ? ? 53 33 DB 89 5D D4 89 5D D8 8B 00 3B C3 74 2B")
#define SIG_CL_SENDMOVE				XorStr("55 8B EC B8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 56 E8 ? ? ? ? 8D B0 ? ? ? ? E8 ? ? ? ? 8B 80 ? ? ? ? 8B 0E 8D 54 08 01 89 95 ? ? ? ? E8 ? ? ? ? 8B 80 ? ? ? ? 8B 0D ? ? ? ? 8B 11 89 85 ? ? ? ? 8B 42 20 FF D0")
#define SIG_FX_FIREBULLET			XorStr("55 8B EC 8B 0D ? ? ? ? 83 EC 10 53")
#define SIG_WEAPON_ID_TO_ALIAS		XorStr("55 8B EC 8B 45 08 83 F8 37")
#define SIG_LOOKUP_WEAPON_INFO		XorStr("55 8B EC 8B 45 08 83 EC 08 85 C0")
#define SIG_INVALOID_WEAPON_INFO	XorStr("B8 ? ? ? ? C3")
#define SIG_GET_WEAPON_FILE_INFO	XorStr("55 8B EC 66 8B 45 08 66 3B 05 ? ? ? ? 73 1A")
#define SIG_PROCCESS_SET_CONVAR		XorStr("55 8B EC 8B 49 08 8B 01 8B 50 18")
#define SIG_PROCCESS_GET_CONVAR		XorStr("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC D9 EE 53 56 DD 95 ? ? ? ? 33 DB DD 95 ? ? ? ? 57 DD 9D ? ? ? ?")
#define SIG_CREATEMOVESHARED		XorStr("55 8B EC 6A FF E8 ? ? ? ? 83 C4 04 85 C0 75 06 B0 01")
#define SIG_SEND_NETMSG				XorStr("55 8B EC 56 8B F1 8D 8E ? ? ? ? E8 ? ? ? ? 85 C0 75 07 B0 01 5E 5D C2 0C 00 53")
#define SIG_RANDOM_SEED				XorStr("A3 ? ? ? ? 5D C3 55")
#define SIG_OVERRIDE_VIEW			XorStr("55 8B EC 83 EC 40 6A FF ")
#define SIG_WRITE_LISTEN_EVENTS		XorStr("55 8B EC 8B 45 08 83 EC 08 53 56 83 C0 10 33 F6 8B D9 3B C6 74 0C 6A 40 56 50 E8 ? ? ? ? 83 C4 0C 89 75 F8")
#define SIG_CHECK_FILE_CRC_SERVER	XorStr("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 57 8B F9 80 BF ? ? ? ? ? 0F 84 ? ? ? ? 83 7F 68 06 0F 85 ? ? ? ? FF 15 ? ? ? ? D9 95 ? ? ? ? D8 A7 ? ? ? ? D9 05 ? ? ? ? DF F1 DD D8")
#define SIG_EMIT_SOUND				XorStr("55 8B EC 81 EC ? ? ? ? 6A 00")
#define SIG_VALIDATE_USERCMD		XorStr("55 8B EC 53 56 57 8B F9 8B 4D 08 E8 ? ? ? ? 8B 0D ? ? ? ? 8B D8 8B 01 8B 90 ? ? ? ? FF D2")
#define SIG_MD5_PSEUDORANDOM		XorStr("55 8B EC 83 EC 6C A1 ? ? ? ? 33 C5 89 45 FC 6A 58 8D 45 A4 6A 00 50 E8 ? ? ? ? 6A 04 8D 4D 08 51")
#define SIG_ONENTITYCREATED			XorStr("55 8B EC 56 8B F1 E8 ? ? ? ? 84 C0 74 53")
#define SIG_ONENTITYDELETED			XorStr("55 8B EC 8B 45 08 53 57 8B D9")

#define PRINT_OFFSET(_name,_ptr)	{ss.str("");\
	ss << _name << XorStr(" - Found: 0x") << std::hex << std::uppercase << _ptr << std::oct << std::nouppercase;\
	Utils::log(ss.str().c_str());}

static std::unique_ptr<DetourXS> g_pDetourCL_SendMove, g_pDetourProcessSetConVar, g_pDetourCreateMove,
	g_pDetourSendNetMsg, g_pDetourWriteListenEventList, g_pDetourEmitSoundInternal,
	g_pDetourOnEntityCreated, g_pDetourOnEntityDeleted;

static std::unique_ptr<CVmtHook> g_pHookClient, g_pHookClientState, g_pHookVGui, g_pHookClientMode,
	g_pHookPanel, g_pHookPrediction, g_pHookRenderView, g_pHookMaterialSystem, g_pHookGameEvent,
	g_pHookModelRender, g_pHookEngineSound, g_pHookNetChannel;

CClientHook::~CClientHook()
{
	UninstallHook();
	Shutdown();
}

bool CClientHook::Init()
{
	if (g_pHookClient && g_pHookClientState && g_pHookVGui && g_pHookClientMode && g_pHookRenderView && g_pHookPrediction)
		return true;

	bool hookSuccess = true;
	std::stringstream ss;

	/*
	ss.sync_with_stdio(false);
	ss.tie(nullptr);
	ss.setf(std::ios::fixed);
	ss.precision(0);
	*/

	oCL_Move = reinterpret_cast<FnCL_Move>(Utils::FindPattern(XorStr("engine.dll"), SIG_CL_MOVE));
	PRINT_OFFSET(XorStr("oCL_Move"), oCL_Move);

	g_pClientPrediction = std::make_unique<CClientPrediction>();
	g_pClientPrediction->Init();

	if (oCL_SendMove == nullptr || !g_pDetourCL_SendMove)
	{
		oCL_SendMove = reinterpret_cast<FnCL_SendMove>(Utils::FindPattern(XorStr("engine.dll"), SIG_CL_SENDMOVE));
		PRINT_OFFSET(XorStr("CL_SendMove"), oCL_SendMove);

		if (oCL_SendMove != nullptr)
		{
			g_pDetourCL_SendMove = std::make_unique<DetourXS>(oCL_SendMove, Hooked_CL_SendMove);
			oCL_SendMove = reinterpret_cast<FnCL_SendMove>(g_pDetourCL_SendMove->GetTrampoline());
		}
	}

	// 在 Windows 平台有两个不同的 CClientState 类
	// 一个继承了 CBaseClientState，并且它不包含 Process 开头的方法
	// 另一个没有继承 CBaseClientState，它包含了 Process 开头的方法
	// 这两个 CClientState 是不相关的，无法通过 CBaseClientState 来挂钩
	if (oProcessSetConVar == nullptr || !g_pDetourProcessSetConVar)
	{
		oProcessSetConVar = reinterpret_cast<FnProcessSetConVar>(Utils::FindPattern(XorStr("engine.dll"), SIG_PROCCESS_SET_CONVAR));
		PRINT_OFFSET(XorStr("CBaseClientState::ProcessSetConVar"), oProcessSetConVar);

		if (oProcessSetConVar != nullptr)
		{
			g_pDetourProcessSetConVar = std::make_unique<DetourXS>(oProcessSetConVar, Hooked_ProcessSetConVar);
			oProcessSetConVar = reinterpret_cast<FnProcessSetConVar>(g_pDetourProcessSetConVar->GetTrampoline());
		}
	}

	if (oCreateMoveShared == nullptr || !g_pDetourCreateMove)
	{
		oCreateMoveShared = reinterpret_cast<FnCreateMoveShared>(Utils::FindPattern(XorStr("client.dll"), SIG_CREATEMOVESHARED));
		PRINT_OFFSET(XorStr("ClientModeShared::CreateMove"), oCreateMoveShared);

		g_pDetourCreateMove = std::make_unique<DetourXS>(oCreateMoveShared, Hooked_CreateMoveShared);
		oCreateMoveShared = reinterpret_cast<FnCreateMoveShared>(g_pDetourCreateMove->GetTrampoline());
	}

	if (g_pInterface->Client != nullptr && !g_pHookClient)
	{
		g_pHookClient = std::make_unique<CVmtHook>(g_pInterface->Client);
		oCreateMove = reinterpret_cast<FnCreateMove>(g_pHookClient->HookFunction(indexes::CreateMove, Hooked_CreateMove));
		oFrameStageNotify = reinterpret_cast<FnFrameStageNotify>(g_pHookClient->HookFunction(indexes::FrameStageNotify, Hooked_FrameStageNotify));
		oDispatchUserMessage = reinterpret_cast<FnDispatchUserMessage>(g_pHookClient->HookFunction(indexes::DispatchUserMessage, Hooked_DispatchUserMessage));
		// oWriteUsercmdDeltaToBuffer = reinterpret_cast<FnWriteUsercmdDeltaToBuffer>(g_pHookClient->HookFunction(indexes::WriteUsercmdDeltaToBuffer, Hooked_WriteUsercmdDeltaToBuffer));
		// oRenderView = reinterpret_cast<FnRenderView>(g_pHookClient->HookFunction(indexes::RenderView, Hooked_RenderView));
		g_pHookClient->InstallHook();
	}

	if (g_pInterface->Panel != nullptr && !g_pHookPanel)
	{
		g_pHookPanel = std::make_unique<CVmtHook>(g_pInterface->Panel);
		oPaintTraverse = reinterpret_cast<FnPaintTraverse>(g_pHookPanel->HookFunction(indexes::PaintTraverse, Hooked_PaintTraverse));
		g_pHookPanel->InstallHook();
	}

	if (g_pInterface->EngineVGui != nullptr && !g_pHookVGui)
	{
		g_pHookVGui = std::make_unique<CVmtHook>(g_pInterface->EngineVGui);
		oEnginePaint = reinterpret_cast<FnEnginePaint>(g_pHookVGui->HookFunction(indexes::EnginePaint, Hooked_EnginePaint));
		g_pHookVGui->InstallHook();
	}

	if (g_pInterface->Prediction != nullptr && !g_pHookPrediction)
	{
		g_pHookPrediction = std::make_unique<CVmtHook>(g_pInterface->Prediction);
		oRunCommand = reinterpret_cast<FnRunCommand>(g_pHookPrediction->HookFunction(indexes::RunCommand, Hooked_RunCommand));
		g_pHookPrediction->InstallHook();
	}

	if (g_pInterface->RenderView != nullptr && !g_pHookRenderView)
	{
		g_pHookRenderView = std::make_unique<CVmtHook>(g_pInterface->RenderView);
		oSceneEnd = reinterpret_cast<FnSceneEnd>(g_pHookRenderView->HookFunction(indexes::SceneEnd, Hooked_SceneEnd));
		g_pHookRenderView->InstallHook();
	}

	if (g_pInterface->MaterialSystem != nullptr && !g_pHookMaterialSystem)
	{
		g_pHookMaterialSystem = std::make_unique<CVmtHook>(g_pInterface->MaterialSystem);
		oFindMaterial = reinterpret_cast<FnFindMaterial>(g_pHookMaterialSystem->HookFunction(indexes::FindMaterial, Hooked_FindMaterial));
		g_pHookMaterialSystem->InstallHook();
	}

	if (g_pInterface->GameEvent != nullptr && !g_pHookGameEvent)
	{
		g_pHookGameEvent = std::make_unique<CVmtHook>(g_pInterface->GameEvent);
		oFireEventClientSide = reinterpret_cast<FnFireEventClientSide>(g_pHookGameEvent->HookFunction(indexes::FireEventClientSide, Hooked_FireEventClientSide));
		oFireEvent = reinterpret_cast<FnFireEvent>(g_pHookGameEvent->HookFunction(indexes::FireEvent, Hooked_FireEvent));
		g_pHookGameEvent->InstallHook();
	}

	if (g_pInterface->ModelRender != nullptr && !g_pHookModelRender)
	{
		g_pHookModelRender = std::make_unique<CVmtHook>(g_pInterface->ModelRender);
		oDrawModelExecute = reinterpret_cast<FnDrawModelExecute>(g_pHookModelRender->HookFunction(indexes::DrawModelExecute, Hooked_DrawModelExecute));
		g_pHookModelRender->InstallHook();
	}

	/*
	if (g_pInterface->Sound != nullptr && !g_pHookEngineSound)
	{
		g_pHookEngineSound = std::make_unique<CVmtHook>(g_pInterface->Sound);
		oEmitSound = reinterpret_cast<FnEmitSound>(g_pHookEngineSound->HookFunction(indexes::EmitSound, Hooked_EmitSound));
		g_pHookEngineSound->InstallHook();
	}
	*/

	/*
	if (g_pInterface->NetChannel != nullptr && !g_pHookNetChannel)
	{
		g_pHookNetChannel = std::make_unique<CVmtHook>(g_pInterface->NetChannel);
		oSendNetMsg = reinterpret_cast<FnSendNetMsg>(g_pHookNetChannel->HookFunction(indexes::SendNetMsg, Hooked_SendNetMsg));
		g_pHookNetChannel->InstallHook();
	}
	*/

	if (oSendNetMsg == nullptr || !g_pDetourSendNetMsg)
	{
		oSendNetMsg = reinterpret_cast<FnSendNetMsg>(Utils::FindPattern(XorStr("engine.dll"), SIG_SEND_NETMSG));
		PRINT_OFFSET(XorStr("CNetChan::SendNetMsg"), oSendNetMsg);

		if (oSendNetMsg != nullptr)
		{
			g_pDetourSendNetMsg = std::make_unique<DetourXS>(oSendNetMsg, Hooked_SendNetMsg);
			oSendNetMsg = reinterpret_cast<FnSendNetMsg>(g_pDetourSendNetMsg->GetTrampoline());
		}
	}

	/*
	if (oWriteListenEventList == nullptr || !g_pDetourWriteListenEventList)
	{
		oWriteListenEventList = reinterpret_cast<FnWriteListenEventList>(Utils::FindPattern(XorStr("engine.dll"), SIG_WRITE_LISTEN_EVENTS));
		PRINT_OFFSET(XorStr("CGameEventManager::WriteListenEventList"), oWriteListenEventList);

		if (oWriteListenEventList != nullptr)
		{
			g_pDetourWriteListenEventList = std::make_unique<DetourXS>(oWriteListenEventList, Hooked_WriteListenEventList);
			oWriteListenEventList = reinterpret_cast<FnWriteListenEventList>(g_pDetourWriteListenEventList->GetTrampoline());
		}
	}
	*/

	if (oEmitSoundInternal == nullptr || !g_pDetourEmitSoundInternal)
	{
		oEmitSoundInternal = reinterpret_cast<FnEmitSoundInternal>(Utils::FindPattern(XorStr("engine.dll"), SIG_EMIT_SOUND));
		PRINT_OFFSET(XorStr("CEngineSoundClient::EmitSoundInternal"), oEmitSoundInternal);

		if (oEmitSoundInternal != nullptr)
		{
			g_pDetourEmitSoundInternal = std::make_unique<DetourXS>(oEmitSoundInternal, Hooked_EmitSoundInternal);
			oEmitSoundInternal = reinterpret_cast<FnEmitSoundInternal>(g_pDetourEmitSoundInternal->GetTrampoline());
		}
	}

	if (oValidateUserCmd == nullptr)
	{
		oValidateUserCmd = reinterpret_cast<FnValidateUserCmd>(Utils::FindPattern(XorStr("client.dll"), SIG_VALIDATE_USERCMD));
		PRINT_OFFSET(XorStr("CInput::ValidateUserCmd"), oValidateUserCmd);
	}

	if (oMD5PseudoRandom == nullptr)
	{
		oMD5PseudoRandom = reinterpret_cast<FnMD5PseudoRandom>(Utils::FindPattern(XorStr("client.dll"), SIG_MD5_PSEUDORANDOM));
		PRINT_OFFSET(XorStr("MD5_PseudoRandom"), oMD5PseudoRandom);
	}

	if (oOnEntityCreated == nullptr || !g_pDetourOnEntityCreated)
	{
		oOnEntityCreated = reinterpret_cast<FnOnEntityCreated>(Utils::FindPattern(XorStr("client.dll"), SIG_ONENTITYCREATED));
		PRINT_OFFSET(XorStr("CClientTools::OnEntityCreated"), oOnEntityCreated);

		if (oOnEntityCreated != nullptr)
		{
			g_pDetourOnEntityCreated = std::make_unique<DetourXS>(oOnEntityCreated, Hooked_OnEntityCreated);
			oOnEntityCreated = reinterpret_cast<FnOnEntityCreated>(g_pDetourOnEntityCreated->GetTrampoline());
		}
	}

	if (oOnEntityDeleted == nullptr || !g_pDetourOnEntityDeleted)
	{
		oOnEntityDeleted = reinterpret_cast<FnOnEntityDeleted>(Utils::FindPattern(XorStr("client.dll"), SIG_ONENTITYDELETED));
		PRINT_OFFSET(XorStr("CClientTools::OnEntityDeleted"), oOnEntityDeleted);

		if (oOnEntityDeleted != nullptr)
		{
			g_pDetourOnEntityDeleted = std::make_unique<DetourXS>(oOnEntityDeleted, Hooked_OnEntityDeleted);
			oOnEntityDeleted = reinterpret_cast<FnOnEntityDeleted>(g_pDetourOnEntityDeleted->GetTrampoline());
		}
	}

	// oWriteListenEventList(g_pInterface->GameEvent, &g_ListenEvents);

	InitFeature();
	// LoadConfig();

	return (g_pHookClient && g_pHookPanel && g_pHookVGui && g_pHookPrediction &&
		g_pHookRenderView && g_pHookMaterialSystem && g_pHookModelRender);
}

void CClientHook::InitFeature()
{
	if (!g_pBunnyHop)
		g_pBunnyHop = new CBunnyHop();
	if (!g_pAimbot)
		g_pAimbot = new CAimBot();
	if (!g_pTriggerBot)
		g_pTriggerBot = new CTriggerBot();
	if (!g_pKnifeBot)
		g_pKnifeBot = new CKnifeBot();
	if (!g_pVisualPlayer)
		g_pVisualPlayer = new CVisualPlayer();
	if (!g_pVisualDrop)
		g_pVisualDrop = new CVisualDrop();
	if (!g_pAntiAntiCheat)
		g_pAntiAntiCheat = new CAntiAntiCheat();
	if (!g_pHackVsHack)
		g_pHackVsHack = new CHackVSHack();
	if (!g_pEventLogger)
		g_pEventLogger = new CEventLogger();
	if (!g_pQTE)
		g_pQTE = new CQuickTriggerEvent();

	// 这些要排在最后，否则没有效果
	if (!g_pViewManager)
		g_pViewManager = new CViewManager();
	if (!g_pSpeedHacker)
		g_pSpeedHacker = new CSpeedHacker();
	if (!g_pWeaponConfig)
		g_pWeaponConfig = new CWeaponConfig();
}

void CClientHook::LoadConfig()
{
	for (auto inst : g_pClientHook->_GameHook)
		if(inst)
			inst->OnConfigLoading(*g_pConfig.get());
}

void CClientHook::SaveConfig()
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnConfigSave(*g_pConfig.get());
	g_pConfig->SaveToFile();
}

void CClientHook::OnMenuOpened()
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnMenuOpened();
}

void CClientHook::OnMenuClosed()
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnMenuClosed();
}

ConVar * CClientHook::GetDummyConVar(const std::string & cvar, const std::optional<std::string>& value)
{
	auto it = g_DummyConVar.find(cvar);
	if (it != g_DummyConVar.end() && it->second)
	{
		if (!it->second->GetDummy())
			it->second->Spoof();
		
		return it->second->GetDummy();
	}
	
	SpoofedConvar* cv = new SpoofedConvar(cvar.c_str());
	g_DummyConVar.try_emplace(cvar, cv);

	if (value.has_value())
		cv->GetDummy()->SetValue(value->c_str());

	return cv->GetDummy();
}

bool CClientHook::RestoreDummyConVar(const std::string & cvar)
{
	auto it = g_DummyConVar.find(cvar);
	if (it == g_DummyConVar.end() || !it->second)
		return false;

	it->second->Unspoof();
	return true;
}

#define DECL_DESTORY_DETOUR(_name)		if(_name && _name->Created())\
	_name->Destroy()

#define DECL_DESTORY_VMT(_name)			if(_name)\
	_name->UninstallHook()

bool CClientHook::UninstallHook()
{
	DECL_DESTORY_DETOUR(g_pDetourCL_SendMove);
	DECL_DESTORY_DETOUR(g_pDetourProcessSetConVar);
	DECL_DESTORY_DETOUR(g_pDetourCreateMove);

	DECL_DESTORY_VMT(g_pHookClient);
	DECL_DESTORY_VMT(g_pHookPanel);
	DECL_DESTORY_VMT(g_pHookVGui);
	DECL_DESTORY_VMT(g_pHookPrediction);
	DECL_DESTORY_VMT(g_pHookRenderView);
	DECL_DESTORY_VMT(g_pHookMaterialSystem);
	DECL_DESTORY_VMT(g_pHookClientMode);
	DECL_DESTORY_VMT(g_pHookClientState);

	return true;
}

void CClientHook::Shutdown()
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnShutdown();

	_GameHook.clear();
}

void __cdecl CClientHook::Hooked_CL_Move(float accumulated_extra_samples, bool bFinalTick)
{
	DWORD _edi;
	DWORD _esi;

	__asm
	{
		mov		_edi, edi
		mov		_esi, esi
	};

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CL_Move Success."));
	}
#endif

	// 默认的 1 次调用，如果不调用会导致游戏冻结
	// 参数 bFinalTick 相当于 bSendPacket
	// 连续调用可以实现加速效果，但是需要改 m_nTickBase 才能正常使用
	Call_CL_Move(_edi, _esi, accumulated_extra_samples, bFinalTick);
}

// 由于 lambda 不支持内联汇编，所以独立出来
void __cdecl CClientHook::Call_CL_Move(DWORD _edi, DWORD _esi, float accumulated_extra_samples, bool bFinalTick)
{
	// 不支持 push byte. 所以只能 push word
	DWORD wFinalTick = bFinalTick;
	DWORD dwOriginFunction = reinterpret_cast<DWORD>(g_pClientHook->oCL_Move);

	__asm
	{
		// 栈传参
		push	wFinalTick
		push	accumulated_extra_samples

		// 寄存器传参
		mov		esi, _esi
		mov		edi, _edi

		// 调用原函数(其实是个蹦床)
		call	dwOriginFunction

		// 清理栈(需要内存对齐)
		add		esp, 8
	};
}

void CClientHook::Hooked_CL_SendMove()
{
	bool blockSendMovement = false;
	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (!inst->OnSendMove())
				blockSendMovement = true;
	}

	if (!blockSendMovement)
		g_pClientHook->oCL_SendMove();
}

void __fastcall CClientHook::Hooked_PaintTraverse(IVPanel* _ecx, LPVOID _edx, VPANEL panel, bool forcePaint, bool allowForce)
{
	g_pClientHook->oPaintTraverse(_ecx, panel, forcePaint, allowForce);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook PaintTraverse Success."));
	}
#endif

	static unsigned int MatSystemTopPanel = 0;
	static unsigned int FocusOverlayPanel = 0;
	if (MatSystemTopPanel == 0 || FocusOverlayPanel == 0)
	{
		const char* panelName = g_pInterface->Panel->GetName(panel);
		if (panelName[0] == 'M' && panelName[3] == 'S' && panelName[9] == 'T')
		{
			MatSystemTopPanel = panel;
#ifdef _DEBUG
			Utils::log("panel %s found %d", panelName, MatSystemTopPanel);
#endif
		}
		else if (panelName[0] == 'F' && panelName[5] == 'O')
		{
			FocusOverlayPanel = panel;
#ifdef _DEBUG
			Utils::log("panel %s found %d", panelName, FocusOverlayPanel);
#endif
		}
	}

	if (panel == FocusOverlayPanel)
	{
		// 在这里获取不会出错
		g_pWorldToScreenMatrix = &g_pInterface->Engine->WorldToScreenMatrix();
	}

	if (panel == FocusOverlayPanel || panel == MatSystemTopPanel)
	{
		for (auto inst : g_pClientHook->_GameHook)
			if (inst)
				inst->OnPaintTraverse(panel);

		/*
		#ifdef _DEBUG
		static HFont font = 0;
		if (font == 0)
		{
		font = g_pInterface->Surface->CreateFont();
		g_pInterface->Surface->SetFontGlyphSet(font, XorStr("Arial"), 16, FW_DONTCARE, 0, 0, FONTFLAG_OUTLINE);
		}

		if (panel == FocusOverlayPanel)
		{
		g_pInterface->Surface->DrawSetColor(255, 0, 0, 255);
		g_pInterface->Surface->DrawFilledRect(60, 60, 70, 70);
		g_pInterface->Surface->DrawSetTextPos(80, 80);
		g_pInterface->Surface->DrawSetTextColor(255, 128, 128, 255);
		g_pInterface->Surface->DrawSetTextFont(font);
		g_pInterface->Surface->DrawPrintText(L"这是一些 Surface 文本", 16);
		}
		#endif
		*/
	}
}

void __fastcall CClientHook::Hooked_EnginePaint(IEngineVGui* _ecx, LPVOID _edx, PaintMode_t mode)
{
	g_pClientHook->oEnginePaint(_ecx, mode);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook VGuiPaint Success."));
	}
#endif

	if (mode & PAINT_UIPANELS)
	{
		g_pInterface->StartDrawing(g_pInterface->Surface);

		if (g_pWorldToScreenMatrix == nullptr)
			g_pWorldToScreenMatrix = &g_pInterface->Engine->WorldToScreenMatrix();

		for (auto inst : g_pClientHook->_GameHook)
			if (inst)
				inst->OnEnginePaint(mode);

		/*
		#ifdef _DEBUG
		g_pInterface->Surface->DrawSetColor(0, 0, 255, 255);
		g_pInterface->Surface->DrawFilledRect(70, 70, 80, 80);
		#endif
		*/

		g_pInterface->FinishDrawing(g_pInterface->Surface);
	}
}

void __fastcall CClientHook::Hooked_RunCommand(IPrediction *_ecx, LPVOID _edx, CBaseEntity *player, CUserCmd *cmd, IMoveHelper *movehelper)
{
	g_pClientHook->oRunCommand(_ecx, player, cmd, movehelper);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook RunCommand Success."));
	}
#endif

	if (g_pInterface->MoveHelper == nullptr)
		g_pInterface->MoveHelper = movehelper;
}

#undef GetLocalPlayer
#define GET_INPUT_CMD(_type,_off)		(&((*reinterpret_cast<_type**>(reinterpret_cast<DWORD>(g_pInterface->Input) + _off))[sequence_number % MULTIPLAYER_BACKUP]))

void __fastcall CClientHook::Hooked_CreateMove(IBaseClientDll *_ecx, LPVOID _edx, int sequence_number, float input_sample_frametime, bool active)
{
	DWORD _ebp;
	__asm mov _ebp, ebp;

	// .text:1007D470 bSendPacket     = byte ptr -1Dh
	g_pClientHook->bSendPacket = reinterpret_cast<bool*>(*reinterpret_cast<byte**>(_ebp) - indexes::bSendPacket);

	g_pClientHook->bCreateMoveFinish = false;

	g_pClientHook->oCreateMove(_ecx, sequence_number, input_sample_frametime, active);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CreateMove Success."));
	}
#endif

	if (g_pClientHook->bCreateMoveFinish)
		return;

	// CVerifiedUserCmd* verified = GET_INPUT_CMD(CVerifiedUserCmd, 0xE0);
	// CUserCmd* cmd = GET_INPUT_CMD(CUserCmd, 0xDC);
	CUserCmd* cmd = g_pInterface->Input->GetUserCmd(-1, sequence_number);
	if (cmd == nullptr/* || verified == nullptr*/)
		return;

	g_pClientPrediction->StartPrediction(cmd);

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnCreateMove(cmd, g_pClientHook->bSendPacket);

	g_pClientPrediction->FinishPrediction();

	// 手动进行 CRC 验证
	// 如果是在 Hooked_CreateMoveShared 则不需要手动验证
	// verified->m_cmd = *cmd;
	// verified->m_crc = cmd->GetChecksum();
	g_pClientHook->oValidateUserCmd(g_pInterface->Input, cmd, sequence_number);
}

void CClientHook::InstallClientModeHook(IClientMode * pointer)
{
	if (!g_pHookClientMode)
	{
		if (g_pDetourCreateMove)
		{
			if (g_pDetourCreateMove->Created())
				g_pDetourCreateMove->Destroy();

			g_pDetourCreateMove.reset();
		}

		g_pInterface->ClientMode = pointer;
		g_pHookClientMode = std::make_unique<CVmtHook>(pointer);
		oCreateMoveShared = reinterpret_cast<FnCreateMoveShared>(g_pHookClientMode->HookFunction(indexes::SharedCreateMove, Hooked_CreateMoveShared));
		oKeyInput = reinterpret_cast<FnKeyInput>(g_pHookClientMode->HookFunction(indexes::KeyInput, Hooked_KeyInput));
		oOverrideView = reinterpret_cast<FnOverrideView>(g_pHookClientMode->HookFunction(indexes::OverrideView, Hooked_OverrideView));
		oGetViewModelFOV = reinterpret_cast<FnGetViewModelFOV>(g_pHookClientMode->HookFunction(indexes::GetViewModelFOV, Hooked_GetViewModelFOV));
		g_pHookClientMode->InstallHook();
	}
}

bool __fastcall CClientHook::Hooked_CreateMoveShared(IClientMode* _ecx, LPVOID _edx, float flInputSampleTime, CUserCmd* cmd)
{
	g_pClientHook->InstallClientModeHook(_ecx);

	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player != nullptr && player->IsAlive())
	{
		// 修复无法移动的 bug
		player->GetFlags() &= ~(FL_FROZEN | FL_FREEZING);

		// 修复 hud 消失的 bug
		player->GetNetProp<WORD>(XorStr("DT_BasePlayer"), XorStr("m_iHideHUD")) = 0;
	}

	g_pClientHook->oCreateMoveShared(_ecx, flInputSampleTime, cmd);

	// 如果这个为 0，表示 callstack 不是 CInput::CreateMove，而是 CInput::ExtraMouseSample，这个是假的，没有意义
	if (cmd->command_number == 0)
		return false;

	// 修复随机数种子为 0 的问题
	if(g_pClientHook->oMD5PseudoRandom)
		cmd->random_seed = (g_pClientHook->oMD5PseudoRandom(cmd->command_number) & 0x7FFFFFFF);
	else
		cmd->random_seed = (MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CreateMoveShared Success."));
	}
#endif

	g_pClientHook->bCreateMoveFinish = true;

	if (cmd == nullptr || cmd->command_number == 0)
		return false;

	g_pClientPrediction->StartPrediction(cmd);

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnCreateMove(cmd, g_pClientHook->bSendPacket);

	g_pClientPrediction->FinishPrediction();

	/*
	// 修复移动不正确
	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);
	math::CorrectMovement(viewAngles, cmd, cmd->forwardmove, cmd->sidemove);
	*/

	// 必须要返回 false 否则会被 engine->SetViewAngles 的
	return false;
}

bool __fastcall CClientHook::Hooked_DispatchUserMessage(IBaseClientDll* _ecx, LPVOID _edx, int msgid, bf_read* data)
{
	bool blockMessage = false;
	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (!inst->OnUserMessage(msgid, *data))
				blockMessage = true;
	}

	if (!blockMessage)
		g_pClientHook->oDispatchUserMessage(_ecx, msgid, data);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook DispatchUserMessage Success."));
	}
#endif

	// 必须返回 true 否则会有警告
	return true;
}

void __fastcall CClientHook::Hooked_FrameStageNotify(IBaseClientDll* _ecx, LPVOID _edx, ClientFrameStage_t stage)
{
	g_pClientHook->oFrameStageNotify(_ecx, stage);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnFrameStageNotify(stage);

	if (stage == FRAME_NET_UPDATE_END)
	{
		static time_t nextUpdate = 0;
		time_t currentTime = time(NULL);
		if (nextUpdate <= currentTime)
		{
			// 计时，用于每隔 1 秒触发一次
			nextUpdate = currentTime + 1;

			static bool isConnected = false;
			if (g_pInterface->Engine->IsConnected())
			{
				if (!isConnected)
				{
					isConnected = true;
					g_tpPlayingTimer = time(nullptr);
					// g_ServerConVar.clear();
					g_pPlayerResource = nullptr;
					g_pGameRulesProxy = nullptr;

					for (auto inst : g_pClientHook->_GameHook)
						if (inst)
							inst->OnConnect();


#ifdef _DEBUG
					static bool isClassUpdated = false;
					if (!isClassUpdated)
					{
						std::fstream fs(Utils::BuildPath(XorStr("classid.txt")), std::ios::out | std::ios::beg | std::ios::trunc);
						if (fs.good() && fs.is_open())
						{
							g_pInterface->NetProp->DumpClassID(fs);
							fs.close();
						}
						isClassUpdated = true;
					}
#endif
				}
			}
			else if (!g_pInterface->Engine->IsInGame())
			{
				if (isConnected)
				{
					isConnected = false;
					g_tpPlayingTimer = 0;
					g_ServerConVar.clear();
					g_pPlayerResource = nullptr;
					g_pGameRulesProxy = nullptr;

					for (auto inst : g_pClientHook->_GameHook)
					{
						if (inst)
							inst->OnDisconnect();

						g_pClientHook->SaveConfig();
					}
				}
			}
		}
	}
}

void CClientHook::InstallClientStateHook(CBaseClientState* pointer)
{
	if (!g_pHookClientState)
	{
		if (g_pDetourProcessSetConVar)
		{
			if (g_pDetourProcessSetConVar->Created())
				g_pDetourProcessSetConVar->Destroy();

			g_pDetourProcessSetConVar.reset();
		}

		g_pHookClientState = std::make_unique<CVmtHook>(pointer);
		oProcessSetConVar = reinterpret_cast<FnProcessSetConVar>(g_pHookClientState->HookFunction(indexes::ProcessSetConVar, Hooked_ProcessSetConVar));
		oProcessGetCvarValue = reinterpret_cast<FnProcessGetCvarValue>(g_pHookClientState->HookFunction(indexes::ProcessGetCvarValue, Hooked_ProcessGetCvarValue));
		oProccessStringCmd = reinterpret_cast<FnProccessStringCmd>(g_pHookClientState->HookFunction(indexes::ProcessStringCmd, Hooked_ProcessStringCmd));
		g_pHookClientState->InstallHook();
	}
}

bool __fastcall CClientHook::Hooked_ProcessGetCvarValue(CBaseClientState* _ecx, LPVOID _edx, SVC_GetCvarValue* gcv)
{
	// oProcessGetCvarValue(_ecx, gcv);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook ProcessGetCvarValue Success."));
	}
#endif

#ifdef _DEBUG
	std::string newResult, tmpValue;
	bool blockQuery = false;
	for (auto inst : g_pClientHook->_GameHook)
	{
		tmpValue.clear();

		if (inst)
		{
			if (!inst->OnProcessGetCvarValue(gcv->m_szCvarName, tmpValue))
				blockQuery = true;
			else if (!tmpValue.empty())
				newResult = std::move(tmpValue);
		}
	}

	/*
	if (newResult.empty())
	return oProcessGetCvarValue(_ecx, gcv);
	*/

	// 空字符串
	char resultBuffer[256]/*, msgBuffer[255]*/;
	// resultBuffer[0] = '\0';
	ZeroMemory(resultBuffer, 256);

	CLC_RespondCvarValue returnMsg;
	returnMsg.m_iCookie = gcv->m_iCookie;
	returnMsg.m_szCvarName = gcv->m_szCvarName;
	returnMsg.m_szCvarValue = resultBuffer;
	returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;
	// returnMsg.SetNetChannel(gcv->GetNetChannel());
	// returnMsg.SetReliable(gcv->IsReliable());

	ConVar* cvar = g_pInterface->Cvar->FindVar(gcv->m_szCvarName);
	if (cvar == nullptr)
	{
		// 服务器查询了一个不存在的 Cvar
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_CvarNotFound;

		// 这个其实是一个命令
		if (g_pInterface->Cvar->FindCommand(gcv->m_szCvarName))
			returnMsg.m_eStatusCode = eQueryCvarValueStatus_NotACvar;

		resultBuffer[0] = '\0';
		returnMsg.m_szCvarValue = resultBuffer;

		Utils::log(XorStr("[GCV] query %s, not found."), gcv->m_szCvarName);

		/*
		sprintf_s(msgBuffer, XorStr("echo \"[AAC] query %s, not found.\""), gcv->m_szCvarName);
		g_pInterface->Engine->ClientCmd_Unrestricted(msgBuffer);
		*/
	}
	else if (cvar->IsFlagSet(FCVAR_SERVER_CANNOT_QUERY))
	{
		// 这玩意不可以查询
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_CvarProtected;
		resultBuffer[0] = '\0';
		returnMsg.m_szCvarValue = resultBuffer;

		Utils::log(XorStr("[GCV] query %s, protected."), gcv->m_szCvarName);
		
		// sprintf_s(msgBuffer, XorStr("echo \"[AAC] query %s, protected.\""), gcv->m_szCvarName);
		// g_pInterface->Engine->ClientCmd_Unrestricted(msgBuffer);
	}
	/*
	else if (cvar->IsFlagSet(FCVAR_NEVER_AS_STRING))
	{
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;
		if (std::fabsf(cvar->GetFloat() - cvar->GetInt()) < 0.001f)
			sprintf_s(resultBuffer, "%d", cvar->GetInt());
		else
			sprintf_s(resultBuffer, "%f", cvar->GetFloat());

		Utils::log(XorStr("[GCV] query %s, returns %s."), gcv->m_szCvarName, resultBuffer);
		returnMsg.m_szCvarValue = resultBuffer;
	}
	else if(cvar->m_nFlags & FCVAR_NEVER_AS_STRING)
	{
		// 可以被查询，但无法转换成字符串
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;
		strcpy_s(resultBuffer, XorStr("FCVAR_NEVER_AS_STRING"));
		returnMsg.m_szCvarValue = resultBuffer;

		Utils::log(XorStr("[GCV] query %s, never as string."), gcv->m_szCvarName);

		// sprintf_s(msgBuffer, XorStr("echo \"[AAC] query %s, never as string.\""), gcv->m_szCvarName);
		// g_pInterface->Engine->ClientCmd_Unrestricted(msgBuffer);
	}
	*/
	else
	{
		// 可以被查询
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;

		auto it = g_ServerConVar.find(gcv->m_szCvarName);
		if (it != g_ServerConVar.end())
		{
			// 把服务器提供的 ConVar 还回去
			strcpy_s(resultBuffer, it->second.c_str());
			returnMsg.m_szCvarValue = resultBuffer;
		}
		else if (!newResult.empty() && !blockQuery)
		{
			// 发送假的的 ConVar
			strcpy_s(resultBuffer, newResult.c_str());
			returnMsg.m_szCvarValue = resultBuffer;
		}
		else
		{
			// 发送默认值，服务器应该检查不出来吧...
			strcpy_s(resultBuffer, cvar->GetDefault());
			returnMsg.m_szCvarValue = resultBuffer;
		}

		Utils::log(XorStr("[GCV] query %s, got %s, returns %s."), gcv->m_szCvarName, cvar->GetString(), resultBuffer);
		
		/*
		sprintf_s(msgBuffer, XorStr("echo \"[AAC] query %s, got %s, returns %s.\""), gcv->m_szCvarName, cvar->GetString(), resultBuffer);
		g_pInterface->Engine->ClientCmd_Unrestricted(msgBuffer);
		*/
	}

	// 返回给服务器
	/*
	bool isSendComplete = false;
	INetChannel* nci = reinterpret_cast<INetChannel*>(g_pInterface->Engine->GetNetChannelInfo());
	if (nci == nullptr)
	{
		if (g_pClientHook->oSendNetMsg == nullptr)
			isSendComplete = gcv->GetNetChannel()->SendNetMsg(returnMsg);
		else
			isSendComplete = g_pClientHook->oSendNetMsg(gcv->GetNetChannel(), returnMsg, false, false);
	}
	else
	{
		if (g_pClientHook->oSendNetMsg == nullptr)
			isSendComplete = nci->SendNetMsg(returnMsg);
		else
			isSendComplete = g_pClientHook->oSendNetMsg(nci, returnMsg, false, false);
	}

	// 我们的 INetChannel::SendNetMsg 可能是假的...
	if (!isSendComplete)
	{
		Utils::log(XorStr("[GCV] Warring: return %s failed... try original."), gcv->m_szCvarName);
		
		// 必须要处理，否则过不了 SMAC
		tmpValue = gcv->m_szCvarName;
		std::transform(tmpValue.begin(), tmpValue.end(), tmpValue.begin(), std::tolower);

		if (cvar && tmpValue.find(XorStr("password")) == std::string::npos)
		{
			// int flags = cvar->GetFlags();

			// 让查询器得到假的 ConVar
			// g_pClientHook->GetDummyConVar(gcv->m_szCvarName, returnMsg.m_szCvarValue);
			g_pClientHook->oProcessGetCvarValue(_ecx, gcv);
			// g_pClientHook->RestoreDummyConVar(gcv->m_szCvarName);
		}
		else
		{
			// 这是个 nullptr，可能是想坑那些有添加 cvar 的人的吧...
			g_pClientHook->oProcessGetCvarValue(_ecx, gcv);
		}
	}
	*/

	/*
	// if (!_ecx->m_NetChannel->SendNetMsg(returnMsg))
	if(!g_pClientHook->oSendNetMsg(_ecx->m_NetChannel, returnMsg, false, false))
	{
		Utils::log(XorStr("[GCV] Warring: return %s failed... try original."), gcv->m_szCvarName);
		g_pClientHook->oProcessGetCvarValue(_ecx, gcv);
	}
	*/
#endif

	return g_pClientHook->oProcessGetCvarValue(_ecx, gcv);
}

bool __fastcall CClientHook::Hooked_ProcessSetConVar(CBaseClientState* _ecx, LPVOID _edx, NET_SetConVar* scv)
{
	// oProcessSetConVar(_ecx, scv);
	g_pClientHook->InstallClientStateHook(_ecx);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook ProcessSetConVar Success."));
	}
#endif

	// char msgBuffer[255];
	decltype(scv->m_ConVars) newCvarList;

	for (auto& cvar : scv->m_ConVars)
	{
		bool blockSetting = false;
		std::string newValue = cvar.value;
		for (auto inst : g_pClientHook->_GameHook)
		{
			if (inst)
				if (!inst->OnProcessSetConVar(cvar.name, newValue))
					blockSetting = true;
		}
		
		if (!blockSetting && !newValue.empty())
			strcpy_s(cvar.value, newValue.c_str());

		// 纪录从服务器发送的 ConVar
		// 等服务器查询时把它返回
		if (cvar.name[0] != '\0')
		{
			g_ServerConVar[cvar.name] = cvar.value;
			// g_ServerConVar.try_emplace(cvar.name, cvar.value);
		}

		if (!blockSetting)
			newCvarList.AddToTail(cvar);

		/*
		// 某些 ConVar 会导致玩家无法正常游戏
		// 在这里防止这些 ConVar 被更改
		if (!_stricmp(cvar.name, XorStr("sv_pure")) || !_stricmp(cvar.name, XorStr("sv_consistency")) ||
			!_stricmp(cvar.name, XorStr("sv_allow_wait_command")) ||
			!_stricmp(cvar.name, XorStr("addons_eclipse_content")))
			blockSetting = true;
		*/

		Utils::log(XorStr("[SCV] set %s to %s."), cvar.name, cvar.value);

		/*
		sprintf_s(msgBuffer, XorStr("echo \"[AAC] %s change to %s.\""), cvar.name, cvar.value);
		g_pInterface->Engine->ClientCmd_Unrestricted(msgBuffer);
		*/
	}

	if (!newCvarList.IsEmpty())
		g_pClientHook->oProcessSetConVar(_ecx, scv);

	return true;
}

bool __fastcall CClientHook::Hooked_ProcessStringCmd(CBaseClientState* _ecx, LPVOID _edx, NET_StringCmd* sc)
{
	// g_pClientHook->oProccessStringCmd(_ecx, sc);
	// g_pClientHook->InstallClientStateHook(_ecx);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook ProcessStringCmd Success."));
	}
#endif

	std::string value = sc->m_szCommand;

	bool blockExecute = false;
	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (!inst->OnProcessClientCommand(value))
				blockExecute = true;
	}

	/*
	// 屏蔽某些坑人的 Command
	if (!_stricmp(sc->m_szCommand, XorStr("bind")) || !_stricmp(sc->m_szCommand, XorStr("sv_pure")) ||
		!_stricmp(sc->m_szCommand, XorStr("sv_consistency")) ||
		!_stricmp(sc->m_szCommand, XorStr("sv_allow_wait_command")) ||
		!_stricmp(sc->m_szCommand, XorStr("addons_eclipse_content")))
		blockExecute = true;
	*/

	/*
	char msgBuffer[255];
	sprintf_s(msgBuffer, XorStr("echo \"[AAC] execute %s.\""), sc->m_szCommand);
	g_pInterface->Engine->ClientCmd_Unrestricted(msgBuffer);
	*/

	Utils::log(XorStr("[SC] exec %s."), sc->m_szCommand);

	if (!blockExecute)
		g_pClientHook->oProccessStringCmd(_ecx, sc);

	return true;
}

bool __fastcall CClientHook::Hooked_WriteUsercmdDeltaToBuffer(IBaseClientDll* _ecx, LPVOID _edx, int solt, bf_write* buf, int from, int to, bool isnewcommand)
{
	// oWriteUsercmdDeltaToBuffer(_ecx, buf, from, to, isnewcommand);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook WriteUsercmdDeltaToBuffer Success."));
	}
#endif

	// 强制更新本地玩家命令
	g_pInterface->WriteUserCmd(buf, g_pInterface->Input->GetUserCmd(solt, to), g_pInterface->Input->GetUserCmd(solt, from));
	return !(buf->IsOverflowed());
}

void __fastcall CClientHook::Hooked_SceneEnd(IVRenderView* _ecx, LPVOID _edx)
{
	g_pClientHook->oSceneEnd(_ecx);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook SceneEnd Success."));
	}
#endif

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnSceneEnd();
}

IMaterial* __fastcall CClientHook::Hooked_FindMaterial(IMaterialSystem* _ecx, LPVOID _edx,
	char const* pMaterialName, const char* pTextureGroupName, bool complain, const char* pComplainPrefix)
{
	bool blockMaterial = false;
	std::string copyMaterialName, copyTextureGroupName;
	if (pMaterialName != nullptr)
		copyMaterialName = pMaterialName;
	if (pTextureGroupName != nullptr)
		copyTextureGroupName = pTextureGroupName;

	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (!inst->OnFindMaterial(copyMaterialName, copyTextureGroupName))
				blockMaterial = true;
	}

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FindMaterial Success."));
	}
#endif

	if (blockMaterial)
		return nullptr;

	return g_pClientHook->oFindMaterial(_ecx, copyMaterialName.c_str(),
		copyTextureGroupName.empty() ? nullptr : copyTextureGroupName.c_str(),
		complain, pComplainPrefix);
}

int __fastcall CClientHook::Hooked_KeyInput(IClientMode* _ecx, LPVOID _edx, int down, ButtonCode_t keynum, const char* pszCurrentBinding)
{
	int result = g_pClientHook->oKeyInput(_ecx, down, keynum, pszCurrentBinding);

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnKeyInput(down != 0, keynum, pszCurrentBinding);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook KeyInput Success."));
	}
#endif

	return result;
}

bool __fastcall CClientHook::Hooked_FireEventClientSide(IGameEventManager2* _ecx, LPVOID _edx, IGameEvent* event)
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnGameEventClient(event);
	
	bool result = g_pClientHook->oFireEventClientSide(_ecx, event);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FireEventClientSide Success."));
	}
#endif

	return result;
}

void __fastcall CClientHook::Hooked_RenderView(IBaseClientDll* _ecx, LPVOID _edx, const CViewSetup& view, int nClearFlags, int whatToDraw)
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnRenderView(const_cast<CViewSetup&>(view));

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook RenderView Success."));
	}
#endif

	g_pClientHook->oRenderView(_ecx, view, nClearFlags, whatToDraw);
}

bool __fastcall CClientHook::Hooked_FireEvent(IGameEventManager2* _ecx, LPVOID _edx, IGameEvent* event, bool bDontBroadcast)
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnGameEvent(event, bDontBroadcast);
	
	bool result = g_pClientHook->oFireEvent(_ecx, event, bDontBroadcast);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FireEvent Success."));
	}
#endif

	return result;
}

void __fastcall CClientHook::Hooked_DrawModelExecute(IVModelRender* _ecx, LPVOID _edx,
	const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnDrawModel(const_cast<DrawModelState_t&>(state), const_cast<ModelRenderInfo_t&>(pInfo), pCustomBoneToWorld);
	
#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook DrawModelExecute Success."));
	}
#endif

	g_pClientHook->oDrawModelExecute(_ecx, state, pInfo, pCustomBoneToWorld);
}

void __fastcall CClientHook::Hooked_EmitSound(IEngineSound* _ecx, LPVOID _edx, IRecipientFilter& filter, int iEntIndex,
	int iChannel, const char* pSample, float flVolume, SoundLevel_t iSoundlevel, int iFlags, int iPitch,
	const Vector* pOrigin, const Vector* pDirection, CUtlVector<Vector>* pUtlVecOrigins,
	bool bUpdatePositions, float soundtime, int speakerentity)
{
	bool blockSound = false;
	std::string copySample;
	Vector copyOrigin = INVALID_VECTOR, copyDirection = INVALID_VECTOR;
	if (pSample != nullptr)
		copySample = pSample;
	if (pOrigin != nullptr)
		copyOrigin = *pOrigin;
	if (pDirection != nullptr)
		copyDirection = *pDirection;

	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (!inst->OnEmitSound(copySample, iEntIndex, iChannel, flVolume, iSoundlevel, iFlags, iPitch,
				copyOrigin, copyDirection, bUpdatePositions, soundtime))
				blockSound = true;
	}
	
#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook EmitSound Success."));
	}
#endif

	if (blockSound)
		return;

	g_pClientHook->oEmitSound(_ecx, filter, iEntIndex, iChannel, copySample.c_str(),
		flVolume, iSoundlevel, iFlags, iPitch,
		copyOrigin.IsValid() ? &copyOrigin : nullptr,
		copyDirection.IsValid() ? &copyDirection : nullptr,
		pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity);
}

bool __fastcall CClientHook::Hooked_SendNetMsg(INetChannel* _ecx, LPVOID _edx, INetMessage& msg, bool bForceReliable, bool bVoice)
{
	if (g_pInterface->NetChannel != _ecx)
		g_pInterface->NetChannel = _ecx;
	
	bool blockNetMsg = false;
	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (!inst->OnSendNetMsg(msg, bForceReliable, bVoice))
				blockNetMsg = false;
	}

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook SendNetMsg Success."));
	}
#endif
	
	if (blockNetMsg)
		return false;

	// 这个会每次连接都会分配新的指针，无法使用 VMT 进行挂钩
	return g_pClientHook->oSendNetMsg(_ecx, msg, bForceReliable, bVoice);
}

void __fastcall CClientHook::Hooked_OverrideView(IClientMode* _ecx, LPVOID, CViewSetup* pSetup)
{
	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnOverrideView(pSetup);
	
	return g_pClientHook->oOverrideView(_ecx, pSetup);
}

float __fastcall CClientHook::Hooked_GetViewModelFOV(IClientMode* _ecx, LPVOID)
{
	bool replace = false;
	float fov = g_pClientHook->oGetViewModelFOV(_ecx);
	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (inst->OnGetViewModelFOV(fov))
				replace = true;
	}
	
	if (replace)
		return fov;

	return g_pClientHook->oGetViewModelFOV(_ecx);
}

void __fastcall CClientHook::Hooked_WriteListenEventList(IGameEventManager2* _ecx, LPVOID, CLC_ListenEvents* msg)
{
	// g_pClientHook->oWriteListenEventList(_ecx, msg);

	msg->m_EventArray.ClearAll();
	// msg->m_EventArray = g_ListenEvents.m_EventArray;

	/*
	// FIXME
	// and know tell the server what events we want to listen to
	for (int i = 0; i < _ecx->m_GameEvents.Count(); i++)
	{
		CGameEventDescriptor& descriptor = _ecx->m_GameEvents[i];

		bool bHasClientListener = false;

		for (int j = 0; j < descriptor.listeners.Count(); j++)
		{
			CGameEventCallback* listener = descriptor.listeners[j];
			if (g_pClientHook->m_ProtectedEventListeners.find(listener->m_pCallback) != g_pClientHook->m_ProtectedEventListeners.end())
				continue;

			if (listener->m_nListenerType == IGameEventManager2::CLIENTSIDE ||
				listener->m_nListenerType == IGameEventManager2::CLIENTSIDE_OLD)
			{
				// if we have a client side listener and server knows this event, add it
				bHasClientListener = true;
				break;
			}
		}

		if (!bHasClientListener)
			continue;

		if (descriptor.eventid == -1)
		{
			// DevMsg("Warning! Client listens to event '%s' unknown by server.\n", descriptor.name);
			continue;
		}

		msg->m_EventArray.Set(descriptor.eventid);
	}
	*/
}

void __fastcall CClientHook::Hooked_EmitSoundInternal(IEngineSound* _ecx, LPVOID, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSample,
	float flVolume, SoundLevel_t iSoundLevel, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, CUtlVector<Vector>* pUtlVecOrigins,
	bool bUpdatePositions, float soundtime, int speakerentity)
{
	bool blockSound = false;
	std::string copySample;
	Vector copyOrigin = INVALID_VECTOR, copyDirection = INVALID_VECTOR;
	if (pSample != nullptr)
		copySample = pSample;
	if (pOrigin != nullptr)
		copyOrigin = *pOrigin;
	if (pDirection != nullptr)
		copyDirection = *pDirection;

	for (auto inst : g_pClientHook->_GameHook)
	{
		if (inst)
			if (!inst->OnEmitSound(copySample, iEntIndex, iChannel, flVolume, iSoundLevel, iFlags, iPitch,
				copyOrigin, copyDirection, bUpdatePositions, soundtime))
				blockSound = true;
	}

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook EmitSoundInternal Success."));
	}
#endif

	if (blockSound)
		return;

	g_pClientHook->oEmitSoundInternal(_ecx, filter, iEntIndex, iChannel, copySample.c_str(),
		flVolume, iSoundLevel, iFlags, iPitch,
		copyOrigin.IsValid() ? &copyOrigin : nullptr,
		copyDirection.IsValid() ? &copyDirection : nullptr,
		pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity);
}

void __fastcall CClientHook::Hooked_OnEntityCreated(LPVOID _ecx, LPVOID, CBaseEntity* entity)
{
	g_pClientHook->oOnEntityCreated(_ecx, entity);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook OnEntityCreated Success."));
	}
#endif

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnEntityCreated(entity);
}

void __fastcall CClientHook::Hooked_OnEntityDeleted(LPVOID _ecx, LPVOID, CBaseEntity* entity)
{
#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook OnEntityDeleted Success."));
	}
#endif

	for (auto inst : g_pClientHook->_GameHook)
		if (inst)
			inst->OnEntityDeleted(entity);

	g_pClientHook->oOnEntityDeleted(_ecx, entity);
}

void CClientPrediction::Init()
{
	m_pSpreadRandomSeed = *reinterpret_cast<int**>(reinterpret_cast<DWORD>(g_pInterface->SharedRandomFloat) + 0x7);
	m_pPredictionRandomSeed = *reinterpret_cast<int**>(Utils::FindPattern(XorStr("client.dll"), SIG_RANDOM_SEED) + 0x1);
}

bool CClientPrediction::StartPrediction(CUserCmd* cmd)
{
	if (g_pInterface->MoveHelper == nullptr)
		return false;

	CBasePlayer* player = GetLocalPlayer();

	if (m_bInPrediction || player == nullptr)
		return false;

	// 备份数据
	m_fCurTime = g_pInterface->GlobalVars->curtime;
	m_fFrameTime = g_pInterface->GlobalVars->frametime;
	m_iTickBase = player->GetTickBase();
	m_iFlags = player->GetFlags();

	// 设置随机数种子
	// *m_pPredictionRandomSeed = (MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF);
	*m_pPredictionRandomSeed = cmd->random_seed;

	// 设置需要预测的时间（帧）
	g_pInterface->GlobalVars->curtime = GetServerTime();
	g_pInterface->GlobalVars->frametime = g_pInterface->GlobalVars->interval_per_tick;

	// 错误检查
	g_pInterface->GameMovement->StartTrackPredictionErrors(player);

	ZeroMemory(&m_MoveData, sizeof(CMoveData));
	m_MoveData.m_nButtons = cmd->buttons;

	// 预测目标
	g_pInterface->MoveHelper->SetHost(player);

	// 开始预测
	g_pInterface->Prediction->SetupMove(player, cmd, g_pInterface->MoveHelper, &m_MoveData);
	g_pInterface->GameMovement->ProcessMovement(player, &m_MoveData);
	g_pInterface->Prediction->FinishMove(player, cmd, &m_MoveData);

	m_bInPrediction = true;
	return true;
}

bool CClientPrediction::FinishPrediction()
{
	if (g_pInterface->MoveHelper == nullptr)
		return false;

	CBasePlayer* player = GetLocalPlayer();

	if (!m_bInPrediction || player == nullptr)
		return false;

	// 结束预测
	player->GetTickBase() = m_iTickBase;
	g_pInterface->GameMovement->FinishTrackPredictionErrors(player);
	g_pInterface->MoveHelper->SetHost(nullptr);
	*m_pPredictionRandomSeed = -1;

	// 还原时间
	g_pInterface->GlobalVars->curtime = m_fCurTime;
	g_pInterface->GlobalVars->frametime = m_fFrameTime;

	// 修复预测后产生的错误
	player->GetFlags() = m_iFlags;

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Client Prediction Success."));
	}
#endif

	m_bInPrediction = false;
	return true;
}

float CClientPrediction::GetServerTime()
{
	return (g_pInterface->GlobalVars->interval_per_tick * GetLocalPlayer()->GetTickBase());
}

CBasePlayer * CClientPrediction::GetLocalPlayer()
{
	return (reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(g_pInterface->Engine->GetLocalPlayer())));
}

std::pair<float, float> CClientPrediction::GetWeaponSpread(int seed, CBaseWeapon* weapon)
{
	if (weapon == nullptr || !weapon->IsFireGun())
		return std::make_pair(0.0f, 0.0f);

	int oldSeed = *m_pSpreadRandomSeed;
	*m_pSpreadRandomSeed = seed;

	float oldSpread = weapon->GetSpread();
	weapon->UpdateSpread();
	float spread = weapon->GetSpread();

	float horizontal = 0.0f, vertical = 0.0f;
	g_pInterface->SharedRandomFloat(XorStr("CTerrorGun::FireBullet HorizSpread"), -spread, spread, 0);
	__asm fstp horizontal;	// 由于优化的问题，这里使用 asm 获取返回值

	g_pInterface->SharedRandomFloat(XorStr("CTerrorGun::FireBullet VertSpread"), -spread, spread, 0);
	__asm fstp vertical;	// 由于优化的问题，这里使用 asm 获取返回值

	weapon->GetSpread() = oldSpread;
	*m_pSpreadRandomSeed = oldSeed;

	return std::make_pair(horizontal, vertical);
}

float CClientPrediction::GetCurrentTime(CUserCmd * cmd)
{
	static int tick = 0;
	static CUserCmd* lastCommand = nullptr;
	if (lastCommand == nullptr || lastCommand->hasbeenpredicted)
		tick = GetLocalPlayer()->GetTickBase();
	else
		++tick;

	lastCommand = cmd;
	return tick * g_pInterface->GlobalVars->interval_per_tick;
}

void CClientPrediction::SetName(const char * name, ...)
{
	INetChannel* nci = reinterpret_cast<INetChannel*>(g_pInterface->Engine->GetNetChannelInfo());
	if (nci == nullptr)
		return;

	char buffer[255];
	
	va_list ap;
	va_start(ap, name);
	vsprintf_s(buffer, name, ap);
	va_end(ap);

	// 要求服务端更新玩家名字
	NET_SetConVar sendConVar(XorStr("name"), buffer);
	sendConVar.SetNetChannel(nci);

	if (g_pClientHook->oSendNetMsg != nullptr)
		g_pClientHook->oSendNetMsg(nci, sendConVar, false, false);
	else
		nci->SendNetMsg(sendConVar);
}
