#include "hook.h"
#include "indexes.h"
#include "./Utils/math.h"
#include "./Utils/checksum_md5.h"
#include "./Structs/convar.h"
#include "./Features/BunnyHop.h"
#include "./Features/SpeedHacker.h"
#include "./Features/TriggerBot.h"
#include "./Features/Aimbot.h"
#include "./Features/NoRecoilSpread.h"
#include "./Features/Knifebot.h"
#include "./Features/Visual.h"
#include "../l4d2Simple2/vmt.h"
#include "../l4d2Simple2/xorstr.h"
#include "../detours/detourxs.h"
#include <memory>

std::unique_ptr<CClientHook> g_pClientHook;
std::unique_ptr<CClientPrediction> g_pClientPrediction;
extern const VMatrix* g_pWorldToScreenMatrix;

#define SIG_CL_MOVE					XorStr("55 8B EC 83 EC 40 A1 ? ? ? ? 33 C5 89 45 FC 56 E8")
#define SIG_CL_SENDMOVE				XorStr("55 8B EC B8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 53 56 57 E8")
#define SIG_WRITE_USERCMD			XorStr("55 8B EC A1 ? ? ? ? 83 78 30 00 53 8B 5D 10")
#define SIG_START_DRAWING			XorStr("55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 14 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F9 80 3D")
#define SIG_FINISH_DRAWING			XorStr("55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 51 56 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 6A 00")
#define SIG_FX_FIREBULLET			XorStr("55 8B EC 8B 0D ? ? ? ? 83 EC 10 53")
#define SIG_WEAPON_ID_TO_ALIAS		XorStr("55 8B EC 8B 45 08 83 F8 37")
#define SIG_LOOKUP_WEAPON_INFO		XorStr("55 8B EC 8B 45 08 83 EC 08 85 C0")
#define SIG_INVALOID_WEAPON_INFO	XorStr("B8 ? ? ? ? C3")
#define SIG_GET_WEAPON_FILE_INFO	XorStr("55 8B EC 66 8B 45 08 66 3B 05 ? ? ? ? 73 1A")
#define SIG_PROCCESS_SET_CONVAR		XorStr("55 8B EC 8B 49 08 8B 01 8B 50 18")
#define SIG_PROCCESS_GET_CONVAR		XorStr("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 53 56 57 8B 7D 08 8B 47 10")
#define SIG_CREATEMOVESHARED		XorStr("55 8B EC 6A FF E8 ? ? ? ? 83 C4 04 85 C0 75 06 B0 01")
#define SIG_SHARED_RANDOM_FLOAT		XorStr("55 8B EC 83 EC 08 A1 ? ? ? ? 53 56 57 8B 7D 14 8D 4D 14 51 89 7D F8 89 45 FC E8 ? ? ? ? 6A 04 8D 55 FC 52 8D 45 14 50 E8 ? ? ? ? 6A 04 8D 4D F8 51 8D 55 14 52 E8 ? ? ? ? 8B 75 08 56 E8 ? ? ? ? 50 8D 45 14 56 50 E8 ? ? ? ? 8D 4D 14 51 E8 ? ? ? ? 8B 15 ? ? ? ? 8B 5D 14 83 C4 30 83 7A 30 00 74 26 57 53 56 68 ? ? ? ? 68 ? ? ? ? 8D 45 14 68 ? ? ? ? 50 C7 45 ? ? ? ? ? FF 15 ? ? ? ? 83 C4 1C 53 B9 ? ? ? ? FF 15 ? ? ? ? D9 45 10")
#define SIG_SET_RANDOM_SEED			XorStr("55 8B EC 8B 45 08 85 C0 75 0C")
#define SIG_GET_WEAPON_INFO			XorStr("55 8B EC 66 8B 45 08 66 3B 05")

#define PRINT_OFFSET(_name,_ptr)	{ss.str("");\
	ss << _name << XorStr(" - Found: 0x") << std::hex << std::uppercase << _ptr << std::oct << std::nouppercase;\
	Utils::log(ss.str().c_str());}

static std::unique_ptr<DetourXS> g_pDetourCL_SendMove, g_pDetourProcessSetConVar, g_pDetourCreateMove;
static std::unique_ptr<CVmtHook> g_pHookClient, g_pHookClientState, g_pHookVGui, g_pHookClientMode,
	g_pHookPanel, g_pHookPrediction, g_pHookRenderView, g_pHookMaterialSystem;

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

	oCL_Move = reinterpret_cast<FnCL_Move>(Utils::FindPattern(XorStr("engine.dll"), SIG_CL_MOVE));
	PRINT_OFFSET(XorStr("oCL_SendMove"), oCL_Move);

	WriteUserCmd = reinterpret_cast<FnWriteUsercmd>(Utils::FindPattern(XorStr("client.dll"), SIG_WRITE_USERCMD));
	PRINT_OFFSET(XorStr("WriteUserCmd"), WriteUserCmd);

	StartDrawing = reinterpret_cast<FnStartDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_START_DRAWING));
	PRINT_OFFSET(XorStr("StartDrawing"), StartDrawing);

	FinishDrawing = reinterpret_cast<FnFinishDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_FINISH_DRAWING));
	PRINT_OFFSET(XorStr("FinishDrawing"), FinishDrawing);

	SharedRandomFloat = reinterpret_cast<FnSharedRandomFloat>(Utils::FindPattern(XorStr("client.dll"), SIG_SHARED_RANDOM_FLOAT));
	PRINT_OFFSET(XorStr("SharedRandomFloat"), SharedRandomFloat);

	SetPredictionRandomSeed = reinterpret_cast<FnSetPredictionRandomSeed>(Utils::FindPattern(XorStr("client.dll"), SIG_SET_RANDOM_SEED));
	PRINT_OFFSET(XorStr("SetPredictionRandomSeed"), SetPredictionRandomSeed);

	g_pClientPrediction = std::make_unique<CClientPrediction>();
	g_pClientPrediction->Init();

	if (oCL_SendMove == nullptr || !g_pDetourCL_SendMove)
	{
		oCL_SendMove = reinterpret_cast<FnCL_SendMove>(Utils::FindPattern(XorStr("engine.dll"), SIG_CL_SENDMOVE));

		PRINT_OFFSET(XorStr("CL_Move"), oCL_SendMove);

		if (oCL_SendMove != nullptr)
		{
			g_pDetourCL_SendMove = std::make_unique<DetourXS>(oCL_SendMove, Hooked_CL_SendMove);
			oCL_SendMove = reinterpret_cast<FnCL_SendMove>(g_pDetourCL_SendMove->GetTrampoline());
		}
	}

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
		oFindMaterial = reinterpret_cast<FnFindMaterial>(indexes::FindMaterial, Hooked_FindMaterial);
		g_pHookMaterialSystem->InstallHook();
	}

	// 初始化功能
	{
		if (!g_pBunnyHop)
			g_pBunnyHop = new CBunnyHop();
		if (!g_pSpeedHacker)
			g_pSpeedHacker = new CSpeedHacker();
		if (!g_pTriggerBot)
			g_pTriggerBot = new CTriggerBot();
		if (!g_pAimbot)
			g_pAimbot = new CAimBot();
		if (!g_pKnifeBot)
			g_pKnifeBot = new CKnifeBot();
		if (!g_pVisualPlayer)
			g_pVisualPlayer = new CVisualPlayer();

		// 这个要排在最后，否则没有效果
		if (!g_pViewManager)
			g_pViewManager = new CViewManager();
	}

	return (g_pHookClient && g_pHookPanel && g_pHookVGui && g_pHookPrediction && g_pHookRenderView && g_pHookMaterialSystem);
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
	for (const auto& inst : g_pClientHook->_GameHook)
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

void CClientHook::Hooked_CL_SendMove()
{
	bool blockSendMovement = false;
	for (const auto& inst : g_pClientHook->_GameHook)
	{
		if (!inst->OnSendMove())
			blockSendMovement = true;
	}

	if(!blockSendMovement)
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

	if (panel == FocusOverlayPanel || panel == MatSystemTopPanel)
	{
		// 在这里获取不会出错
		g_pWorldToScreenMatrix = &g_pInterface->Engine->WorldToScreenMatrix();
		
		for (const auto& inst : g_pClientHook->_GameHook)
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
		g_pClientHook->StartDrawing(g_pInterface->Surface);

		if(g_pWorldToScreenMatrix == nullptr)
			g_pWorldToScreenMatrix = &g_pInterface->Engine->WorldToScreenMatrix();

		for (const auto& inst : g_pClientHook->_GameHook)
			inst->OnEnginePaint(mode);

		/*
#ifdef _DEBUG
		g_pInterface->Surface->DrawSetColor(0, 0, 255, 255);
		g_pInterface->Surface->DrawFilledRect(70, 70, 80, 80);
#endif
		*/

		g_pClientHook->FinishDrawing(g_pInterface->Surface);
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

	if(g_pInterface->MoveHelper == nullptr)
		g_pInterface->MoveHelper = movehelper;
}

#undef GetLocalPlayer
#define GET_INPUT_CMD(_type,_off)		(&((*reinterpret_cast<_type**>(reinterpret_cast<DWORD>(g_pInterface->Input) + _off))[sequence_number % MULTIPLAYER_BACKUP]))

void __fastcall CClientHook::Hooked_CreateMove(IBaseClientDll *_ecx, LPVOID _edx, int sequence_number, float input_sample_frametime, bool active)
{
	DWORD _ebp;
	__asm mov _ebp, ebp;

	// .text:100BBA20			bSendPacket = byte ptr -1
	g_pClientHook->bSendPacket = reinterpret_cast<bool*>(*reinterpret_cast<byte**>(_ebp) - 0x21);

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

	CVerifiedUserCmd* verified = GET_INPUT_CMD(CVerifiedUserCmd, 0xE0);
	CUserCmd* cmd = GET_INPUT_CMD(CUserCmd, 0xDC);
	if (cmd == nullptr || verified == nullptr)
		return;

	/*
	// 验证 CRC 用的
	CVerifiedUserCmd* verified = &((*reinterpret_cast<CVerifiedUserCmd**>(
		reinterpret_cast<DWORD>(g_pInterface->Input) + 0xE0))[sequence_number % 150]);
	
	// 玩家输入
	CUserCmd* cmd = &((*reinterpret_cast<CUserCmd**>(
		reinterpret_cast<DWORD>(g_pInterface->Input) + 0xDC))[sequence_number % 150]);
	*/
	
	g_pClientPrediction->StartPrediction(cmd);

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnCreateMove(cmd, g_pClientHook->bSendPacket);

	g_pClientPrediction->FinishPrediction();

	// 修复移动不正确
	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);
	math::CorrectMovement(viewAngles, cmd, cmd->forwardmove, cmd->sidemove);

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
		oKeyInput = reinterpret_cast<FnKeyInput>(g_pHookClientMode->HookFunction(indexes::KeyInput, Hooked_KeyInput));
		g_pHookClientMode->InstallHook();
	}
}

bool __fastcall CClientHook::Hooked_CreateMoveShared(IClientMode* _ecx, LPVOID _edx, float flInputSampleTime, CUserCmd* cmd)
{
	g_pClientHook->InstallClientModeHook(_ecx);
	
	g_pClientHook->oCreateMoveShared(_ecx, flInputSampleTime, cmd);

	/*
	g_pClientHook->bCreateMoveFinish = true;

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook CreateMoveShared Success."));
	}
#endif

	if (cmd == nullptr || cmd->command_number == 0)
		return false;

	g_pClientPrediction->StartPrediction(cmd);

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnCreateMove(cmd, g_pClientHook->bSendPacket);

	g_pClientPrediction->FinishPrediction();

	// 修复移动不正确
	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);
	math::CorrectMovement(viewAngles, cmd, cmd->forwardmove, cmd->sidemove);
	*/

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

					for (const auto& inst : g_pClientHook->_GameHook)
						inst->OnConnect();
				}
			}
			else if (!g_pInterface->Engine->IsInGame())
			{
				if (isConnected)
				{
					isConnected = false;

					for (const auto& inst : g_pClientHook->_GameHook)
						inst->OnDisconnect();
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

	std::string newResult, tmpValue;
	bool blockQuery = false;
	for (const auto& inst : g_pClientHook->_GameHook)
	{
		tmpValue.clear();
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

	ConVar* cvar = g_pInterface->Cvar->FindVar(gcv->m_szCvarName);
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
		Utils::log(XorStr("Hook ProcessSetConVar Success."));
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
		Utils::log(XorStr("Hook ProcessStringCmd Success."));
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
	g_pClientHook->WriteUserCmd(buf, g_pInterface->Input->GetUserCmd(solt, to), g_pInterface->Input->GetUserCmd(solt, from));
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

IMaterial* __fastcall CClientHook::Hooked_FindMaterial(IMaterialSystem* _ecx, LPVOID _edx, char const* pMaterialName,
	const char* pTextureGroupName, bool complain, const char* pComplainPrefix)
{
	std::string copyMaterialName, copyTextureGroupName;
	std::string newMaterialName, newTextureGroupName;
	for (const auto& inst : g_pClientHook->_GameHook)
	{
		copyMaterialName = pMaterialName;
		copyTextureGroupName = pTextureGroupName;
		if (inst->OnFindMaterial(copyMaterialName, copyTextureGroupName))
		{
			newMaterialName = copyMaterialName;
			newTextureGroupName = copyTextureGroupName;
		}
	}

#ifdef _DEBUG
	static bool hasFirstEnter = true;
	if (hasFirstEnter)
	{
		hasFirstEnter = false;
		Utils::log(XorStr("Hook FindMaterial Success."));
	}
#endif
	
	if (!newMaterialName.empty())
	{
		return g_pClientHook->oFindMaterial(_ecx, newMaterialName.c_str(),
			(newTextureGroupName.empty() ? nullptr : newTextureGroupName.c_str()),
			complain, pComplainPrefix);
	}

	return g_pClientHook->oFindMaterial(_ecx, pMaterialName, pTextureGroupName, complain, pComplainPrefix);
}

int CClientHook::Hooked_KeyInput(IClientMode* _ecx, LPVOID _edx, int down, ButtonCode_t keynum, const char* pszCurrentBinding)
{
	int result = g_pClientHook->oKeyInput(_ecx, down, keynum, pszCurrentBinding);
	
	for (const auto& inst : g_pClientHook->_GameHook)
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

void CClientPrediction::Init()
{
	m_pRandomSeed = *reinterpret_cast<int**>(reinterpret_cast<DWORD>(g_pClientHook->SharedRandomFloat) + 0x1A);
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
	*m_pRandomSeed = (MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF);

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
	*m_pRandomSeed = -1;

	// 还原时间
	g_pInterface->GlobalVars->curtime = m_fCurTime;
	g_pInterface->GlobalVars->frametime = m_fFrameTime;

	// 修复预测后产生的错误
	player->GetFlags() = m_iFlags;
	player->GetNetPropLocal<int>(XorStr("DT_BasePlayer"), XorStr("m_iHideHUD")) = 0;

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

std::pair<float, float> CClientPrediction::GetWeaponSpread(int seed, float spread)
{
	int oldSeed = *m_pRandomSeed;
	*m_pRandomSeed = seed;

	float horizontal = 0.0f, vertical = 0.0f;
	g_pClientHook->SharedRandomFloat(XorStr("CTerrorGun::FireBullet HorizSpread"), -spread, spread, 0);
	__asm fstp horizontal;

	g_pClientHook->SharedRandomFloat(XorStr("CTerrorGun::FireBullet VertSpread"), -spread, spread, 0);
	__asm fstp vertical;

	*m_pRandomSeed = oldSeed;
	return std::make_pair(horizontal, vertical);
}
