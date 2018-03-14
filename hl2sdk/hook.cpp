#include "hook.h"
#include "indexes.h"
#include "./Utils/checksum_md5.h"
#include "./Structs/convar.h"
#include "../l4d2Simple2/vmt.h"
#include "../l4d2Simple2/xorstr.h"
#include "../detours/detourxs.h"
#include <memory>

std::unique_ptr<CClientHook> g_pClientHook;

#define SIG_CL_MOVE					XorStr("55 8B EC 83 EC 34 83 3D")
#define SIG_CL_SENDMOVE				XorStr("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 8D 4D CC")
#define SIG_WRITE_USERCMD			XorStr("55 8B EC 8B 45 10 83 EC 0C B9")
#define SIG_START_DRAWING			XorStr("55 8B EC 64 A1 ? ? ? ? 6A FF 68 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 14")
#define SIG_FINISH_DRAWING			XorStr("55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 51 56 6A 00")
#define SIG_FX_FIREBULLET			XorStr("55 8B EC 8B 0D ? ? ? ? 83 EC 10 53")
#define SIG_WEAPON_ID_TO_ALIAS		XorStr("55 8B EC 8B 45 08 83 F8 43")
#define SIG_LOOKUP_WEAPON_INFO		XorStr("55 8B EC 8B 45 08 83 EC 08 85 C0 74 18")
#define SIG_INVALOID_WEAPON_INFO	XorStr("B8 ? ? ? ? C3")
#define SIG_GET_WEAPON_FILE_INFO	XorStr("55 8B EC 66 8B 45 08 66 3B 05 ? ? ? ? 73 1A")
#define SIG_PROCCESS_SET_CONVAR		XorStr("55 8B EC 8B 49 08 83 EC 0C")
#define SIG_CREATEMOVESHARED		XorStr("55 8B EC E8 ? ? ? ? 8B C8 85 C9 75 06 B0 01")

#define PRINT_OFFSET(_name,_ptr)	{ss.str("");\
	ss << _name << XorStr(" - Found: 0x") << std::hex << std::uppercase << _ptr << std::oct << std::nouppercase;\
	Utils::log(ss.str().c_str());}

static std::unique_ptr<DetourXS> g_pDetourCL_Move, g_pDetourProcessSetConVar, g_pDetourCreateMove;
static std::unique_ptr<CVmtHook> g_pHookClient, g_pHookClientState, g_pHookVGui, g_pHookClientMode,
	g_pHookPanel, g_pHookPrediction, g_pHookRenderView;

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

	HMODULE vstdlib = GetModuleHandleA(XorStr("vstdlib.dll"));
	if (vstdlib != NULL)
	{
		RandomSeed = reinterpret_cast<FnRandomSeed>(GetProcAddress(vstdlib, XorStr("RandomSeed")));
		RandomFloat = reinterpret_cast<FnRandomFloat>(GetProcAddress(vstdlib, XorStr("RandomFloat")));
		RandomFloatExp = reinterpret_cast<FnRandomFloatExp>(GetProcAddress(vstdlib, XorStr("RandomFloatExp")));
		RandomInt = reinterpret_cast<FnRandomInt>(GetProcAddress(vstdlib, XorStr("RandomInt")));
		RandomGaussianFloat = reinterpret_cast<FnRandomGaussianFloat>(GetProcAddress(vstdlib, XorStr("RandomGaussianFloat")));
		InstallUniformRandomStream = reinterpret_cast<FnInstallUniformRandomStream>(GetProcAddress(vstdlib, XorStr("InstallUniformRandomStream")));

		PRINT_OFFSET(XorStr("RandomSeed"), RandomSeed);
		PRINT_OFFSET(XorStr("RandomFloat"), RandomFloat);
		PRINT_OFFSET(XorStr("RandomFloatExp"), RandomFloatExp);
		PRINT_OFFSET(XorStr("RandomInt"), RandomInt);
		PRINT_OFFSET(XorStr("RandomGaussianFloat"), RandomGaussianFloat);
		PRINT_OFFSET(XorStr("InstallUniformRandomStream"), InstallUniformRandomStream);
	}

	CL_SendMove = reinterpret_cast<FnCL_SendMove>(Utils::FindPattern(XorStr("engine.dll"), SIG_CL_SENDMOVE));
	PRINT_OFFSET(XorStr("CL_SendMove"), CL_SendMove);

	WriteUserCmd = reinterpret_cast<FnWriteUsercmd>(Utils::FindPattern(XorStr("client.dll"), SIG_WRITE_USERCMD));
	PRINT_OFFSET(XorStr("WriteUserCmd"), WriteUserCmd);

	StartDrawing = reinterpret_cast<FnStartDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_START_DRAWING));
	PRINT_OFFSET(XorStr("StartDrawing"), StartDrawing);

	FinishDrawing = reinterpret_cast<FnFinishDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_FINISH_DRAWING));
	PRINT_OFFSET(XorStr("FinishDrawing"), FinishDrawing);

	/*
	if (oCL_Move == nullptr || !g_pDetourCL_Move)
	{
		oCL_Move = reinterpret_cast<FnCL_Move>(Utils::FindPattern(XorStr("engine.dll"), SIG_CL_MOVE));

		PRINT_OFFSET(XorStr("CL_Move"), oCL_Move);

		if (oCL_Move != nullptr)
		{
			g_pDetourCL_Move = std::make_unique<DetourXS>(oCL_Move, Hooked_CL_Move);
			oCL_Move = reinterpret_cast<FnCL_Move>(g_pDetourCL_Move->GetTrampoline());
		}
	}
	*/

	if (oProcessSetConVar == nullptr || !g_pHookClientState)
	{
		oProcessSetConVar = reinterpret_cast<FnProcessSetConVar>(Utils::FindPattern(XorStr("engine.dll"), SIG_PROCCESS_SET_CONVAR));

		PRINT_OFFSET(XorStr("CBaseClientState::ProcessSetConVar"), oProcessSetConVar);

		if (oProcessSetConVar != nullptr)
		{
			g_pDetourProcessSetConVar = std::make_unique<DetourXS>(oProcessSetConVar, Hooked_ProcessSetConVar);
			oProcessSetConVar = reinterpret_cast<FnProcessSetConVar>(g_pDetourProcessSetConVar->GetTrampoline());
		}
	}

	if (oCreateMoveShared == nullptr || !g_pHookClientMode)
	{
		oCreateMoveShared = reinterpret_cast<FnCreateMoveShared>(Utils::FindPattern(XorStr("client.dll"), SIG_CREATEMOVESHARED));

		PRINT_OFFSET(XorStr("ClientModeShared::CreateMove"), oCreateMoveShared);

		g_pDetourCreateMove = std::make_unique<DetourXS>(oCreateMoveShared, Hooked_CreateMoveShared);
		oCreateMoveShared = reinterpret_cast<FnCreateMoveShared>(g_pDetourCreateMove->GetTrampoline());
	}

	if (g_pClientInterface->Client != nullptr && !g_pHookClient)
	{
		g_pHookClient = std::make_unique<CVmtHook>(g_pClientInterface->Client);
		oCreateMove = reinterpret_cast<FnCreateMove>(g_pHookClient->HookFunction(indexes::CreateMove, Hooked_CreateMove));
		oFrameStageNotify = reinterpret_cast<FnFrameStageNotify>(g_pHookClient->HookFunction(indexes::FrameStageNotify, Hooked_FrameStageNotify));
		oDispatchUserMessage = reinterpret_cast<FnDispatchUserMessage>(g_pHookClient->HookFunction(indexes::DispatchUserMessage, Hooked_DispatchUserMessage));
		// oWriteUsercmdDeltaToBuffer = reinterpret_cast<FnWriteUsercmdDeltaToBuffer>(g_pHookClient->HookFunction(indexes::WriteUsercmdDeltaToBuffer, Hooked_WriteUsercmdDeltaToBuffer));
		g_pHookClient->InstallHook();
	}
	else
	{
		hookSuccess = false;
	}

	if (g_pClientInterface->Panel != nullptr && !g_pHookPanel)
	{
		g_pHookPanel = std::make_unique<CVmtHook>(g_pClientInterface->Panel);
		oPaintTraverse = reinterpret_cast<FnPaintTraverse>(g_pHookPanel->HookFunction(indexes::PaintTraverse, Hooked_PaintTraverse));
		g_pHookPanel->InstallHook();
	}
	else
	{
		hookSuccess = false;
	}
	
	if (g_pClientInterface->EngineVGui != nullptr && !g_pHookVGui)
	{
		g_pHookVGui = std::make_unique<CVmtHook>(g_pClientInterface->EngineVGui);
		oEnginePaint = reinterpret_cast<FnEnginePaint>(g_pHookVGui->HookFunction(indexes::EnginePaint, Hooked_EnginePaint));
		g_pHookVGui->InstallHook();
	}

	if (g_pClientInterface->Prediction != nullptr && !g_pHookPrediction)
	{
		g_pHookPrediction = std::make_unique<CVmtHook>(g_pClientInterface->Prediction);
		oRunCommand = reinterpret_cast<FnRunCommand>(g_pHookPrediction->HookFunction(indexes::RunCommand, Hooked_RunCommand));
		g_pHookPrediction->InstallHook();
	}

	if (g_pClientInterface->RenderView != nullptr && !g_pHookRenderView)
	{
		g_pHookRenderView = std::make_unique<CVmtHook>(g_pClientInterface->RenderView);
		oSceneEnd = reinterpret_cast<FnSceneEnd>(g_pHookRenderView->HookFunction(indexes::SceneEnd, Hooked_SceneEnd));
		g_pHookRenderView->InstallHook();
	}

	return (g_pHookClient && g_pHookPanel && g_pHookVGui && g_pHookPrediction && g_pHookRenderView);
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

	// Gateway
	static auto gwCL_Move = [](DWORD _edi, DWORD _esi, float accumulated_extra_samples, bool bFinalTick) -> void
	{
		// 不支持 push byte. 所以只能 push word
		DWORD wFinalTick = bFinalTick;
		DWORD dwOriginFunction = reinterpret_cast<DWORD>(g_pClientHook->oCL_Move);

		__asm
		{
			// 堆栈传参
			push	wFinalTick
			push	accumulated_extra_samples

			// 寄存器传参
			mov		esi, _esi
			mov		edi, _edi

			// 调用原函数(其实是个蹦床)
			call	dwOriginFunction

			// 清理堆栈(需要内存对齐)
			add		esp, 8
		};
	};

	// 默认的 1 次调用，如果不调用会导致游戏冻结
	// 参数 bFinalTick 相当于 bSendPacket
	// 连续调用可以实现加速效果，但是需要破解 m_nTickBase 才能用
	gwCL_Move(_edi, _esi, accumulated_extra_samples, bFinalTick);
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
		const char* panelName = g_pClientInterface->Panel->GetName(panel);
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

	if ((FocusOverlayPanel > 0 && panel == FocusOverlayPanel) ||
		(MatSystemTopPanel > 0 && panel == MatSystemTopPanel))
	{
		for (const auto& inst : g_pClientHook->_GameHook)
			inst->OnPaintTraverse(panel);

#ifdef _DEBUG
		if (panel == FocusOverlayPanel)
		{
			g_pClientInterface->Surface->DrawSetColor(255, 0, 0, 255);
			g_pClientInterface->Surface->DrawFilledRect(60, 60, 70, 70);
		}
#endif
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
		g_pClientHook->StartDrawing(g_pClientInterface->Surface);

		for (const auto& inst : g_pClientHook->_GameHook)
			inst->OnEnginePaint(mode);

#ifdef _DEBUG
		g_pClientInterface->Surface->DrawSetColor(0, 0, 255, 255);
		g_pClientInterface->Surface->DrawFilledRect(70, 70, 80, 80);
#endif

		g_pClientHook->FinishDrawing(g_pClientInterface->Surface);
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

	if(g_pClientInterface->MoveHelper == nullptr)
		g_pClientInterface->MoveHelper = movehelper;
}

#undef GetLocalPlayer

void __fastcall CClientHook::Hooked_CreateMove(IBaseClientDll *_ecx, LPVOID _edx, int sequence_number, float input_sample_frametime, bool active)
{
	DWORD _ebp;
	__asm mov _ebp, ebp;

	// .text:100BBA20			bSendPacket = byte ptr -1
	g_pClientHook->bSendPacket = reinterpret_cast<bool*>(*reinterpret_cast<byte**>(_ebp) - 1);

	g_pClientHook->bCreateMoveFinish = false;

	g_pClientHook->oCreateMove(_ecx, sequence_number, input_sample_frametime, active);

	if (g_pClientHook->bCreateMoveFinish)
		return;

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CreateMove Success."));
	}
#endif

	CVerifiedUserCmd* verified = &g_pClientInterface->Input->m_pVerifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
	CUserCmd* cmd = g_pClientInterface->Input->GetUserCmd(sequence_number);

	/*
	// 验证 CRC 用的
	CVerifiedUserCmd* verified = &((*reinterpret_cast<CVerifiedUserCmd**>(
		reinterpret_cast<DWORD>(g_pClientInterface->Input) + 0xE0))[sequence_number % 150]);
	
	// 玩家输入
	CUserCmd* cmd = &((*reinterpret_cast<CUserCmd**>(
		reinterpret_cast<DWORD>(g_pClientInterface->Input) + 0xDC))[sequence_number % 150]);
	*/
	
	// 本地玩家
	CBaseEntity* localPlayer = reinterpret_cast<CBaseEntity*>(g_pClientInterface->EntList->GetClientEntity(
		g_pClientInterface->Engine->GetLocalPlayer()));

	static CMoveData movedata;
	int flags = localPlayer->GetNetProp<int>(XorStr("DT_BasePlayer"), XorStr("m_fFlags"));
	float serverTime = g_pClientInterface->GlobalVars->interval_per_tick * localPlayer->GetNetProp<int>(XorStr("DT_BasePlayer"), XorStr("m_nTickBase"));
	float oldCurtime = g_pClientInterface->GlobalVars->curtime;
	float oldFrametime = g_pClientInterface->GlobalVars->frametime;

	if (g_pClientInterface->MoveHelper != nullptr)
	{
		// TODO: 设置随机数的种子为 MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF
		// 目前还不知道随机数种子用哪个设置
		if(g_pClientHook->RandomSeed)
			g_pClientHook->RandomSeed(MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF);

		// 设置需要预测的时间（帧）
		g_pClientInterface->GlobalVars->curtime = serverTime;
		g_pClientInterface->GlobalVars->frametime = g_pClientInterface->GlobalVars->interval_per_tick;

		// 启动错误检查
		g_pClientInterface->GameMovement->StartTrackPredictionErrors(localPlayer);

		// 清空预测结果的数据
		ZeroMemory(&movedata, sizeof(CMoveData));

		// 设置需要预测的玩家
		g_pClientInterface->MoveHelper->SetHost(localPlayer);

		// 开始预测
		g_pClientInterface->Prediction->SetupMove(localPlayer, cmd, g_pClientInterface->MoveHelper, &movedata);
		g_pClientInterface->GameMovement->ProcessMovement(localPlayer, &movedata);
		g_pClientInterface->Prediction->FinishMove(localPlayer, cmd, &movedata);
	}

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnCreateMove(cmd, g_pClientHook->bSendPacket);

	if (g_pClientInterface->MoveHelper != nullptr)
	{
		// 结束预测
		g_pClientInterface->GameMovement->FinishTrackPredictionErrors(localPlayer);
		g_pClientInterface->MoveHelper->SetHost(nullptr);

		// TODO: 设置随机数种子为 -1
		// 目前还不知道随机数种子用哪个设置
		if (g_pClientHook->RandomSeed)
			g_pClientHook->RandomSeed(-1);

		// 还原备份
		g_pClientInterface->GlobalVars->curtime = oldCurtime;
		g_pClientInterface->GlobalVars->frametime = oldFrametime;

		// 修复一些错误
		localPlayer->GetNetProp<int>(XorStr("DT_BasePlayer"), XorStr("m_fFlags")) = flags;
		localPlayer->GetNetPropLocal<int>(XorStr("DT_BasePlayer"), XorStr("m_iHideHUD")) = flags;
	}

	// 手动进行 CRC 验证
	// 如果是在 Hooked_CreateMoveShared 则不需要手动验证
	verified->m_cmd = *cmd;
	verified->m_crc = cmd->GetChecksum();
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

		g_pHookClientMode = std::make_unique<CVmtHook>(pointer);
		oCreateMoveShared = reinterpret_cast<FnCreateMoveShared>(g_pHookClientMode->HookFunction(indexes::SharedCreateMove, Hooked_CreateMoveShared));
		g_pHookClientMode->InstallHook();
	}
}

bool __fastcall CClientHook::Hooked_CreateMoveShared(IClientMode* _ecx, LPVOID _edx, float flInputSampleTime, CUserCmd* cmd)
{
	g_pClientHook->InstallClientModeHook(_ecx);
	
	g_pClientHook->oCreateMoveShared(_ecx, flInputSampleTime, cmd);
	g_pClientHook->bCreateMoveFinish = true;

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CreateMoveShared Success."));
	}
#endif

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnCreateMove(cmd, g_pClientHook->bSendPacket);

	// 必须要返回 false 否则会出现 bug
	return false;
}

bool __fastcall CClientHook::Hooked_DispatchUserMessage(IBaseClientDll* _ecx, LPVOID _edx, int msgid, bf_read* data)
{
	bool blockMessage = false;
	for (const auto& inst : g_pClientHook->_GameHook)
	{
		if(!inst->OnUserMessage(msgid, *data))
			blockMessage = true;
	}

	if(!blockMessage)
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
	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnFrameStageNotify(stage);
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
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	std::string newResult, tmpValue;
	bool blockQuery = false;
	for (const auto& inst : g_pClientHook->_GameHook)
	{
		if (!inst->OnProcessGetCvarValue(gcv, tmpValue))
			blockQuery = true;
		else if (!tmpValue.empty())
			newResult = std::move(tmpValue);
	}

	/*
	if (newResult.empty())
		return oProcessGetCvarValue(_ecx, gcv);
	*/

	// 空字符串
	char resultBuffer[256];
	resultBuffer[0] = '\0';

	CLC_RespondCvarValue returnMsg;
	returnMsg.m_iCookie = gcv->m_iCookie;
	returnMsg.m_szCvarName = gcv->m_szCvarName;
	returnMsg.m_szCvarValue = resultBuffer;
	returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;

	ConVar* cvar = g_pClientInterface->Cvar->FindVar(gcv->m_szCvarName);
	if (cvar == nullptr)
	{
		// 服务器查询了一个不存在的 Cvar
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_CvarNotFound;
		resultBuffer[0] = '\0';
		returnMsg.m_szCvarValue = resultBuffer;
	}
	else if (cvar->IsFlagSet(FCVAR_SERVER_CANNOT_QUERY) || blockQuery)
	{
		// 这玩意不可以查询
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_CvarProtected;
		resultBuffer[0] = '\0';
		returnMsg.m_szCvarValue = resultBuffer;
	}
	else if (cvar->IsFlagSet(FCVAR_NEVER_AS_STRING))
	{
		// 可以被查询，但无法转换成字符串
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;
		strcpy_s(resultBuffer, XorStr("FCVAR_NEVER_AS_STRING"));
		returnMsg.m_szCvarValue = resultBuffer;
	}
	else
	{
		// 可以被查询
		returnMsg.m_eStatusCode = eQueryCvarValueStatus_ValueIntact;

		auto it = g_pClientHook->m_serverConVar.find(gcv->m_szCvarName);
		if (it != g_pClientHook->m_serverConVar.end())
		{
			// 把服务器提供的 ConVar 还回去
			strcpy_s(resultBuffer, it->second.c_str());
			returnMsg.m_szCvarValue = resultBuffer;
		}
		else if (!newResult.empty())
		{
			// 发送修改后的 ConVar
			strcpy_s(resultBuffer, newResult.c_str());
			returnMsg.m_szCvarValue = resultBuffer;
		}
		else
		{
			// 发送默认值，服务器应该检查不出来吧...
			strcpy_s(resultBuffer, cvar->GetDefault());
			returnMsg.m_szCvarValue = resultBuffer;
		}
	}

	// 返回给服务器
	gcv->GetNetChannel()->SendNetMsg(returnMsg);

	return true;
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
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	bool blockSetting = false;
	for (const auto& inst : g_pClientHook->_GameHook)
	{
		if (!inst->OnProcessSetConVar(scv))
			blockSetting = true;
	}

	if(!blockSetting)
		g_pClientHook->oProcessSetConVar(_ecx, scv);

	for (const auto& cvar : scv->m_ConVars)
	{
		// 纪录从服务器发送的 ConVar
		// 等服务器查询时把它返回
		if (cvar.name[0] != '\0')
			g_pClientHook->m_serverConVar[cvar.name] = cvar.value;
	}

	return true;
}

bool __fastcall CClientHook::Hooked_ProcessStringCmd(CBaseClientState* _ecx, LPVOID _edx, NET_StringCmd* sc)
{
	g_pClientHook->oProccessStringCmd(_ecx, sc);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	bool blockExecute = false;
	for (const auto& inst : g_pClientHook->_GameHook)
	{
		if (!inst->OnProcessClientCommand(sc))
			blockExecute = true;
	}

	if (!blockExecute)
		g_pClientHook->oProccessStringCmd(_ecx, sc);

	return true;
}

bool __fastcall CClientHook::Hooked_WriteUsercmdDeltaToBuffer(IBaseClientDll* _ecx, LPVOID _edx, bf_write* buf, int from, int to, bool isnewcommand)
{
	// oWriteUsercmdDeltaToBuffer(_ecx, buf, from, to, isnewcommand);
	
#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	// 强制更新本地玩家命令
	g_pClientHook->WriteUserCmd(buf, g_pClientInterface->Input->GetUserCmd(to), g_pClientInterface->Input->GetUserCmd(from));
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

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnSceneEnd();
}
