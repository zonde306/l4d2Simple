#include "hook.h"
#include "indexes.h"
#include "./Structs/convar.h"
#include "../l4d2Simple2/vmt.h"
#include "../l4d2Simple2/xorstr.h"
#include "../detours/detourxs.h"
#include <memory>

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

#define PRINT_OFFSET(_name,_ptr)	{ss.clear();\
	ss << _name << XorStr(" - Found: 0x") << std::hex << std::uppercase << _ptr << std::oct << std::nouppercase;\
	Utils::log(ss.str().c_str());}

static std::unique_ptr<DetourXS> g_pDetourCL_Move, g_pDetourProcessSetConVar, g_pDetourCreateMove;
static std::unique_ptr<CVmtHook> g_pHookClient, g_pHookClientState, g_pHookVGui, g_pHookClientMode,
	g_pHookPanel, g_pHookPrediction;

typedef void(__thiscall* FnStartDrawing)(ISurface*);
typedef void(__thiscall* FnFinishDrawing)(ISurface*);

namespace hook
{
	std::vector<std::shared_ptr<CBaseFeatures>> _GameHook;
	std::vector<FnHookCreateMove> _CreateMove;
	std::vector<FnHookCreateMoveShared> _CreateMoveShared;
	std::vector<FnHookPaintTraverse> _PaintTraverse;
	std::vector<FnHookEnginePaint> _EnginePaint;
	std::vector<FnHookUserMessage> _UserMessage;
	std::vector<FnOnFrameStageNotify> _FrameStageNotify;
	std::vector<FnOnProcessGetCvarValue> _ProcessGetCvarValue;
	std::vector<FnOnProcessSetConVar> _ProcessSetConVar;
	std::vector<FnOnProcessStringCmd> _ProcessStringCmd;
	
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

	void InstallClientStateHook(CBaseClientState* pointer);
	void InstallClientModeHook(IClientMode* pointer);

	void __cdecl Hooked_CL_Move(float, bool);
	void __fastcall Hooked_PaintTraverse(IVPanel*, LPVOID, VPANEL, bool, bool);
	bool __fastcall Hooked_CreateMoveShared(IClientMode*, LPVOID, float, CUserCmd*);
	void __fastcall Hooked_CreateMove(IBaseClientDll*, LPVOID, int, float, bool);
	void __fastcall Hooked_FrameStageNotify(IBaseClientDll*, LPVOID, ClientFrameStage_t);
	void __fastcall Hooked_RunCommand(IPrediction*, LPVOID, CBaseEntity*, CUserCmd*, IMoveHelper*);
	bool __fastcall Hooked_DispatchUserMessage(IBaseClientDll*, LPVOID, int, bf_read*);
	void __fastcall Hooked_EnginePaint(IEngineVGui*, LPVOID, PaintMode_t);
	bool __fastcall Hooked_ProcessGetCvarValue(CBaseClientState*, LPVOID, SVC_GetCvarValue*);
	bool __fastcall Hooked_ProcessSetConVar(CBaseClientState*, LPVOID, NET_SetConVar*);
	bool __fastcall Hooked_ProcessStringCmd(CBaseClientState*, LPVOID, NET_StringCmd*);
	bool __fastcall Hooked_WriteUsercmdDeltaToBuffer(IBaseClientDll*, LPVOID, bf_write*, int, int, bool);

	FnStartDrawing StartDrawing = nullptr;
	FnFinishDrawing FinishDrawing = nullptr;
	FnCL_SendMove CL_SendMove = nullptr;
	FnWriteUsercmd WriteUserCmd = nullptr;

	bool bCreateMoveFinish = false;
	bool* bSendPacket = nullptr;
	std::map<std::string, std::string> mGotConVar;
}

bool hook::InstallHook()
{
	if (g_pHookClient && g_pHookClientState && g_pHookVGui && g_pHookClientMode)
		return true;

	bool hookSuccess = true;
	std::stringstream ss;

	CL_SendMove = reinterpret_cast<FnCL_SendMove>(Utils::FindPattern(XorStr("engine.dll"), SIG_CL_SENDMOVE));
	PRINT_OFFSET(XorStr("CL_SendMove"), CL_SendMove);

	WriteUserCmd = reinterpret_cast<FnWriteUsercmd>(Utils::FindPattern(XorStr("client.dll"), SIG_WRITE_USERCMD));
	PRINT_OFFSET(XorStr("WriteUserCmd"), WriteUserCmd);

	StartDrawing = reinterpret_cast<FnStartDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_START_DRAWING));
	PRINT_OFFSET(XorStr("StartDrawing"), StartDrawing);

	FinishDrawing = reinterpret_cast<FnFinishDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_FINISH_DRAWING));
	PRINT_OFFSET(XorStr("FinishDrawing"), FinishDrawing);

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

	if (interfaces::Client != nullptr && !g_pHookClient)
	{
		g_pHookClient = std::make_unique<CVmtHook>(interfaces::Client);
		oCreateMove = reinterpret_cast<FnCreateMove>(g_pHookClient->HookFunction(indexes::CreateMove, Hooked_CreateMove));
		oFrameStageNotify = reinterpret_cast<FnFrameStageNotify>(g_pHookClient->HookFunction(indexes::FrameStageNotify, Hooked_FrameStageNotify));
		oDispatchUserMessage = reinterpret_cast<FnDispatchUserMessage>(g_pHookClient->HookFunction(indexes::DispatchUserMessage, Hooked_DispatchUserMessage));
		oWriteUsercmdDeltaToBuffer = reinterpret_cast<FnWriteUsercmdDeltaToBuffer>(g_pHookClient->HookFunction(indexes::WriteUsercmdDeltaToBuffer, Hooked_WriteUsercmdDeltaToBuffer));
		g_pHookClient->InstallHook();
	}
	else
	{
		hookSuccess = false;
	}

	if (interfaces::Panel != nullptr && !g_pHookPanel)
	{
		g_pHookPanel = std::make_unique<CVmtHook>(interfaces::Panel);
		oPaintTraverse = reinterpret_cast<FnPaintTraverse>(g_pHookPanel->HookFunction(indexes::PaintTraverse, Hooked_PaintTraverse));
		g_pHookPanel->InstallHook();
	}
	else
	{
		hookSuccess = false;
	}
	
	if (interfaces::EngineVGui != nullptr && !g_pHookVGui)
	{
		g_pHookVGui = std::make_unique<CVmtHook>(interfaces::EngineVGui);
		oEnginePaint = reinterpret_cast<FnEnginePaint>(g_pHookPanel->HookFunction(indexes::EnginePaint, Hooked_EnginePaint));
		g_pHookPanel->InstallHook();
	}

	if (interfaces::Prediction != nullptr && !g_pHookPrediction)
	{
		g_pHookPrediction = std::make_unique<CVmtHook>(interfaces::Prediction);
		oRunCommand = reinterpret_cast<FnRunCommand>(g_pHookPrediction->HookFunction(indexes::RunCommand, Hooked_RunCommand));
		g_pHookPrediction->InstallHook();
	}

	return (g_pHookClient && g_pHookPanel && g_pHookVGui && g_pHookPrediction);
}

void __cdecl hook::Hooked_CL_Move(float accumulated_extra_samples, bool bFinalTick)
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

		__asm
		{
			// 堆栈传参
			push	wFinalTick
			push	accumulated_extra_samples

			// 寄存器传参
			mov		esi, _esi
			mov		edi, _edi

			// 调用原函数(其实是个蹦床)
			call	oCL_Move

			// 清理堆栈(需要内存对齐)
			add		esp, 8
		};
	};

	// 默认的 1 次调用，如果不调用会导致游戏冻结
	// 参数 bFinalTick 相当于 bSendPacket
	// 连续调用可以实现加速效果，但是需要破解 m_nTickBase 才能用
	gwCL_Move(_edi, _esi, accumulated_extra_samples, bFinalTick);
}

void __fastcall hook::Hooked_PaintTraverse(IVPanel* _ecx, LPVOID _edx, VPANEL panel, bool forcePaint, bool allowForce)
{
	oPaintTraverse(_ecx, panel, forcePaint, allowForce);

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
		const char* panelName = interfaces::Panel->GetName(panel);
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

	// 每一帧调用两次
	if (FocusOverlayPanel > 0 && panel == FocusOverlayPanel)
	{
		for (const FnHookPaintTraverse& func : _PaintTraverse)
			func(true);
	}
	// 每一帧调用多次 (至少20次)
	else if (MatSystemTopPanel > 0 && panel == MatSystemTopPanel)
	{
		for (const FnHookPaintTraverse& func : _PaintTraverse)
			func(false);
	}

	if ((FocusOverlayPanel > 0 && panel == FocusOverlayPanel) ||
		(MatSystemTopPanel > 0 && panel == MatSystemTopPanel))
	{
		for (const auto& inst : _GameHook)
			inst->OnPaintTraverse(panel);
	}
}

void __fastcall hook::Hooked_EnginePaint(IEngineVGui* _ecx, LPVOID _edx, PaintMode_t mode)
{
	oEnginePaint(_ecx, mode);

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
		StartDrawing(interfaces::Surface);

		for (const FnHookEnginePaint& func : _EnginePaint)
			func();

		for (const auto& inst : _GameHook)
			inst->OnEnginePaint(mode);

		FinishDrawing(interfaces::Surface);
	}
}

void __fastcall hook::Hooked_RunCommand(IPrediction *_ecx, LPVOID _edx, CBaseEntity *player, CUserCmd *cmd, IMoveHelper *movehelper)
{
	oRunCommand(_ecx, player, cmd, movehelper);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook RunCommand Success."));
	}
#endif

	if(interfaces::MoveHelper == nullptr)
		interfaces::MoveHelper = movehelper;
}

#undef GetLocalPlayer

void __fastcall hook::Hooked_CreateMove(IBaseClientDll *_ecx, LPVOID _edx, int sequence_number, float input_sample_frametime, bool active)
{
	DWORD _ebp;
	__asm mov _ebp, ebp;

	// .text:100BBA20			bSendPacket = byte ptr -1
	bSendPacket = reinterpret_cast<bool*>(*reinterpret_cast<byte**>(_ebp) - 1);

	bCreateMoveFinish = false;

	oCreateMove(_ecx, sequence_number, input_sample_frametime, active);

	if (bCreateMoveFinish)
		return;

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CreateMove Success."));
	}
#endif

	CVerifiedUserCmd* verified = &interfaces::Input->m_pVerifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
	CUserCmd* cmd = interfaces::Input->GetUserCmd(sequence_number);

	/*
	// 验证 CRC 用的
	CVerifiedUserCmd* verified = &((*reinterpret_cast<CVerifiedUserCmd**>(
		reinterpret_cast<DWORD>(interfaces::Input) + 0xE0))[sequence_number % 150]);
	
	// 玩家输入
	CUserCmd* cmd = &((*reinterpret_cast<CUserCmd**>(
		reinterpret_cast<DWORD>(interfaces::Input) + 0xDC))[sequence_number % 150]);
	*/
	
	// 本地玩家
	CBaseEntity* localPlayer = reinterpret_cast<CBaseEntity*>(interfaces::EntList->GetClientEntity(
		interfaces::Engine->GetLocalPlayer()));

	static CMoveData movedata;
	int flags = localPlayer->GetNetProp<int>(XorStr("DT_BasePlayer"), XorStr("m_fFlags"));
	float serverTime = interfaces::GlobalVars->interval_per_tick * localPlayer->GetNetProp<int>(XorStr("DT_BasePlayer"), XorStr("m_nTickBase"));
	float oldCurtime = interfaces::GlobalVars->curtime;
	float oldFrametime = interfaces::GlobalVars->frametime;

	if (interfaces::MoveHelper != nullptr)
	{
		// TODO: 设置随机数的种子为 MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF
		// 目前还不知道随机数种子用哪个设置

		// 设置需要预测的时间（帧）
		interfaces::GlobalVars->curtime = serverTime;
		interfaces::GlobalVars->frametime = interfaces::GlobalVars->interval_per_tick;

		// 启动错误检查
		interfaces::GameMovement->StartTrackPredictionErrors(localPlayer);

		// 清空预测结果的数据
		ZeroMemory(&movedata, sizeof(CMoveData));

		// 设置需要预测的玩家
		interfaces::MoveHelper->SetHost(localPlayer);

		// 开始预测
		interfaces::Prediction->SetupMove(localPlayer, cmd, interfaces::MoveHelper, &movedata);
		interfaces::GameMovement->ProcessMovement(localPlayer, &movedata);
		interfaces::Prediction->FinishMove(localPlayer, cmd, &movedata);
	}

	for (const FnHookCreateMove& func : _CreateMove)
		func(localPlayer, cmd, bSendPacket);

	for (const auto& inst : _GameHook)
		inst->OnCreateMove(cmd, bSendPacket);

	if (interfaces::MoveHelper != nullptr)
	{
		// 结束预测
		interfaces::GameMovement->FinishTrackPredictionErrors(localPlayer);
		interfaces::MoveHelper->SetHost(nullptr);

		// TODO: 设置随机数种子为 -1
		// 目前还不知道随机数种子用哪个设置

		// 还原备份
		interfaces::GlobalVars->curtime = oldCurtime;
		interfaces::GlobalVars->frametime = oldFrametime;

		// 修复一些错误
		localPlayer->GetNetProp<int>(XorStr("DT_BasePlayer"), XorStr("m_fFlags")) = flags;
		localPlayer->GetNetPropLocal<int>(XorStr("DT_BasePlayer"), XorStr("m_iHideHUD")) = flags;
	}

	// 手动进行 CRC 验证
	// 如果是在 Hooked_CreateMoveShared 则不需要手动验证
	verified->m_cmd = *cmd;
	verified->m_crc = cmd->GetChecksum();
}

void hook::InstallClientModeHook(IClientMode * pointer)
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

bool __fastcall hook::Hooked_CreateMoveShared(IClientMode* _ecx, LPVOID _edx, float flInputSampleTime, CUserCmd* cmd)
{
	InstallClientModeHook(_ecx);
	
	oCreateMoveShared(_ecx, flInputSampleTime, cmd);
	bCreateMoveFinish = true;

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CreateMoveShared Success."));
	}
#endif

	// 本地玩家
	CBaseEntity* localPlayer = reinterpret_cast<CBaseEntity*>(interfaces::EntList->GetClientEntity(
		interfaces::Engine->GetLocalPlayer()));

	for (const FnHookCreateMoveShared& func : _CreateMoveShared)
		func(localPlayer, cmd, bSendPacket);

	for (const auto& inst : _GameHook)
		inst->OnCreateMove(cmd, bSendPacket);

	// 必须要返回 false 否则会出现 bug
	return false;
}

bool __fastcall hook::Hooked_DispatchUserMessage(IBaseClientDll* _ecx, LPVOID _edx, int msgid, bf_read* data)
{
	bool blockMessage = false;
	for (const FnHookUserMessage& func : _UserMessage)
	{
		// 由于 bf_read 是不可逆的，所以使用拷贝来防止读取越界
		if (!func(msgid, *data))
			blockMessage = true;
	}

	for (const auto& inst : _GameHook)
	{
		if(!inst->OnUserMessage(msgid, *data))
			blockMessage = true;
	}

	if(!blockMessage)
		oDispatchUserMessage(_ecx, msgid, data);

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

void __fastcall hook::Hooked_FrameStageNotify(IBaseClientDll* _ecx, LPVOID _edx, ClientFrameStage_t stage)
{
	oFrameStageNotify(_ecx, stage);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	for (auto func : _FrameStageNotify)
		func(stage);

	for (const auto& inst : _GameHook)
		inst->OnFrameStageNotify(stage);
}

void hook::InstallClientStateHook(CBaseClientState* pointer)
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

bool __fastcall hook::Hooked_ProcessGetCvarValue(CBaseClientState* _ecx, LPVOID _edx, SVC_GetCvarValue* gcv)
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
	for (auto func : _ProcessGetCvarValue)
	{
		tmpValue = func(gcv);
		if (!tmpValue.empty())
			newResult = std::move(tmpValue);
	}
	
	bool blockQuery = false;
	for (const auto& inst : _GameHook)
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

	ConVar* cvar = interfaces::Cvar->FindVar(gcv->m_szCvarName);
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

		auto it = mGotConVar.find(gcv->m_szCvarName);
		if (it != mGotConVar.end())
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

bool __fastcall hook::Hooked_ProcessSetConVar(CBaseClientState* _ecx, LPVOID _edx, NET_SetConVar* scv)
{
	// oProcessSetConVar(_ecx, scv);
	InstallClientStateHook(_ecx);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	bool blockSetting = false;
	for (auto func : _ProcessSetConVar)
	{
		if (!func(scv))
			blockSetting = true;
	}

	for (const auto& inst : _GameHook)
	{
		if (!inst->OnProcessSetConVar(scv))
			blockSetting = true;
	}

	if(!blockSetting)
		oProcessSetConVar(_ecx, scv);

	for (const auto& cvar : scv->m_ConVars)
	{
		// 纪录从服务器发送的 ConVar
		// 等服务器查询时把它返回
		if (cvar.name[0] != '\0')
			mGotConVar[cvar.name] = cvar.value;
	}

	return true;
}

bool __fastcall hook::Hooked_ProcessStringCmd(CBaseClientState* _ecx, LPVOID _edx, NET_StringCmd* sc)
{
	oProccessStringCmd(_ecx, sc);

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FrameStageNotify Success."));
	}
#endif

	bool blockExecute = false;
	for (auto func : _ProcessStringCmd)
	{
		if (!func(sc))
			blockExecute = true;
	}

	for (const auto& inst : _GameHook)
	{
		if (!inst->OnProcessClientCommand(sc))
			blockExecute = true;
	}

	if (!blockExecute)
		oProccessStringCmd(_ecx, sc);

	return true;
}

bool __fastcall hook::Hooked_WriteUsercmdDeltaToBuffer(IBaseClientDll* _ecx, LPVOID _edx, bf_write* buf, int from, int to, bool isnewcommand)
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
	WriteUserCmd(buf, interfaces::Input->GetUserCmd(to), interfaces::Input->GetUserCmd(from));
	return !(buf->IsOverflowed());
}
