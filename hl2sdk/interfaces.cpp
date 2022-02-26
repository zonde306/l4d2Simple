#include "interfaces.h"
#include "indexes.h"
#include "../l4d2Simple2/xorstr.h"
#include <fstream>

std::unique_ptr<CClientInterface> g_pInterface;

#define PRINT_OFFSET(_name,_ptr)	{ss.str("");\
	ss << _name << XorStr(" - Found: 0x") << std::hex << std::uppercase << _ptr << std::oct << std::nouppercase;\
	Utils::log(ss.str().c_str());}

#define GET_VFUNC(_ptr,_off)	((*reinterpret_cast<PDWORD*>(_ptr))[_off])

#define SIG_GET_CLIENTMODE			XorStr("55 8B EC 8B 45 08 83 F8 FF 75 10 8B 0D ? ? ? ? 8B 01 8B 90 ? ? ? ? FF D2 8B 04 85 ? ? ? ? 5D C3")
#define SIG_GET_CLIENTSTATE			XorStr("A1 ? ? ? ? 83 C0 08 C3")
#define SIG_MOVE_HELPER				XorStr("A1 ? ? ? ? 8B 10 8B 52 ? 81 C1")
#define SIG_GLOBAL_VARS				XorStr("8B 0D ? ? ? ? D9 41 ? 8B 55 ? 8B 45")
#define SIG_WRITE_USERCMD			XorStr("55 8B EC A1 ? ? ? ? 83 78 30 00 53 8B 5D 10")
#define SIG_START_DRAWING			XorStr("55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 14 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F9 80 3D")
#define SIG_FINISH_DRAWING			XorStr("55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 51 56 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 6A 00")
#define SIG_SHARED_RANDOM_FLOAT_OLD	XorStr("55 8B EC 83 EC 08 A1 ? ? ? ? 53 56 57 8B 7D 14 8D 4D 14 51 89 7D F8 89 45 FC E8 ? ? ? ? 6A 04 8D 55 FC 52 8D 45 14 50 E8 ? ? ? ? 6A 04 8D 4D F8 51 8D 55 14 52 E8 ? ? ? ? 8B 75 08 56 E8 ? ? ? ? 50 8D 45 14 56 50 E8 ? ? ? ? 8D 4D 14 51 E8 ? ? ? ? 8B 15 ? ? ? ? 8B 5D 14 83 C4 30 83 7A 30 00 74 26 57 53 56 68 ? ? ? ? 68 ? ? ? ? 8D 45 14 68 ? ? ? ? 50 C7 45 ? ? ? ? ? FF 15 ? ? ? ? 83 C4 1C 53 B9 ? ? ? ? FF 15 ? ? ? ? D9 45 10")
#define SIG_SHARED_RANDOM_FLOAT		XorStr("E8 ? ? ? ? D9 5D A8 D9 05 ? ? ? ? 83 C4 10 57")
#define SIG_SET_RANDOM_SEED			XorStr("55 8B EC 8B 45 08 85 C0 75 0C")
#define SIG_GET_WEAPON_INFO			XorStr("55 8B EC 66 8B 45 08 66 3B 05")
#define SIG_UPDATE_WEAPON_SPREAD	XorStr("53 8B DC 83 EC ? 83 E4 ? 83 C4 ? 55 8B 6B ? 89 6C ? ? 8B EC 83 EC ? 56 57 8B F9 E8")
#define SIG_TRACE_LINE2				XorStr("53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 6C 56 8B 43 08")
#define SIG_TRACE_LINE				XorStr("53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 5C 56 8B 43 08")
#define SIG_CLIP_TRACE_PLAYER		XorStr("53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 56 57 8B 53 14")
#define SIG_RESOURCE_POINTER		XorStr("A1 ? ? ? ? 89 45 F4 85 C0")
#define SIG_CINPUT					XorStr("8B 0D ? ? ? ? 8B 11 8B 42 74 6A FF FF D0 8B 0D")

typedef IClientMode*(__cdecl *FnGetClientMode)(int);
static FnGetClientMode GetClientMode = nullptr;

typedef CBaseClientState*(__cdecl *FnGetClientState)();
static FnGetClientState GetClientState = nullptr;

void CClientInterface::Init()
{
	Client = GetPointer<IBaseClientDll>(XorStr("client.dll"), XorStr("VClient"));
	Engine = GetPointer<IEngineClient>(XorStr("engine.dll"), XorStr("VEngineClient"));
	Trace = GetPointer<IEngineTrace>(XorStr("engine.dll"), XorStr("EngineTraceClient"));
	EntList = GetPointer<IClientEntityList>(XorStr("client.dll"), XorStr("VClientEntityList"));
	Cvar = GetPointer<ICvar>(XorStr("vstdlib.dll"), XorStr("VEngineCvar"));
	GameEvent = GetPointer<IGameEventManager2>(XorStr("engine.dll"), XorStr("GAMEEVENTSMANAGER002"));
	Surface = GetPointer<ISurface>(XorStr("vguimatsurface.dll"), XorStr("VGUI_Surface"));
	DebugOverlay = GetPointer<IVDebugOverlay>(XorStr("engine.dll"), XorStr("VDebugOverlay"));
	Panel = GetPointer<IVPanel>(XorStr("vgui2.dll"), XorStr("VGUI_Panel"));
	ModelInfo = GetPointer<IModelInfo>(XorStr("engine.dll"), XorStr("VModelInfoClient"));
	GameMovement = GetPointer<IGameMovement>(XorStr("client.dll"), XorStr("GameMovement"));
	Prediction = GetPointer<IPrediction>(XorStr("client.dll"), XorStr("VClientPrediction"));
	PlayerInfo = GetPointer<IPlayerInfoManager>(XorStr("server.dll"), XorStr("PlayerInfoManager"));
	InputSystem = GetPointer<IInputSystem>(XorStr("inputsystem.dll"), XorStr("InputSystemVersion"));
	EngineVGui = GetPointer<IEngineVGui>(XorStr("engine.dll"), XorStr("VEngineVGui"));
	ModelRender = GetPointer<IVModelRender>(XorStr("engine.dll"), XorStr("VEngineModel"));
	FileSystem = GetPointer<IFileSystem>(XorStr("filesystem_stdio.dll"), XorStr("VFileSystem"));
	Localize = GetPointer<ILocalize>(XorStr("localize.dll"), XorStr("Localize_"));
	StringTable = GetPointer<INetworkStringTableContainer>(XorStr("engine.dll"), XorStr("VEngineClientStringTable"));
	RenderView = GetPointer<IVRenderView>(XorStr("engine.dll"), XorStr("VEngineRenderView"));
	MaterialSystem = GetPointer<IMaterialSystem>(XorStr("materialsystem.dll"), XorStr("VMaterialSystem"));
	Sound = GetPointer<IEngineSound>(XorStr("engine.dll"), XorStr("IEngineSoundClient"));
	KeyValueSystem = GetPointer<IKeyValuesSystem>(XorStr("engine.dll"), XorStr("KeyValuesSystem"));

	std::stringstream ss;

	/*
	ss.tie(nullptr);
	ss.setf(std::ios::fixed);
	ss.precision(0);
	*/

	/*
	if (PlayerInfo != nullptr)
	{
		try
		{
			GlobalVars = PlayerInfo->GetGlobalVars();
		}
		catch (...)
		{
			GlobalVars = nullptr;
		}

		if (GlobalVars != nullptr)
		{
			ss << XorStr("CGlobalVarsBase Got: 0x");
			ss << std::hex << std::uppercase << GlobalVars;
			Utils::log(ss.str().c_str());
		}
	}
	*/

	GetClientMode = reinterpret_cast<FnGetClientMode>(Utils::FindPattern(XorStr("client.dll"), SIG_GET_CLIENTMODE));
	PRINT_OFFSET(XorStr("GetClientMode"), GetClientMode);
	
	GetClientState = reinterpret_cast<FnGetClientState>(Utils::FindPattern(XorStr("engine.dll"), SIG_GET_CLIENTSTATE));
	PRINT_OFFSET(XorStr("GetClientState"), GetClientState);

	/*
	GlobalVars = FindGlobalVars();
	if (GlobalVars == nullptr)
	*/
	GlobalVars = **reinterpret_cast<CGlobalVarsBase***>(Utils::FindPattern(XorStr("client.dll"), SIG_GLOBAL_VARS) + 2);

	if (GlobalVars != nullptr)
	{
		PRINT_OFFSET(XorStr("CGlobalVarsBase"), GlobalVars);
	}
	
	if (Client != nullptr)
	{
		DWORD funcStart = GET_VFUNC(Client, indexes::CreateMove);
		Input = **reinterpret_cast<IInput***>(funcStart + 0x28);
		// IInput* InputOld = **reinterpret_cast<IInput***>(Utils::FindPattern(XorStr("client.dll"), SIG_CINPUT) + 2);
		PRINT_OFFSET(XorStr("CInput"), Input);
		// PRINT_OFFSET(XorStr("CInputOld"), InputOld);

		ClientMode = GetClientMode(-1);
		PRINT_OFFSET(XorStr("ClientMode"), ClientMode);

		ClientState = GetClientState();
		PRINT_OFFSET(XorStr("ClientState"), ClientState);

		/*
		funcStart = GET_VFUNC(GameMovement, indexes::PlaySwimSound);
		MoveHelper = **reinterpret_cast<IMoveHelper***>(funcStart + 0x4);
		PRINT_OFFSET(XorStr("MoveHelper"), MoveHelper);
		*/

		NetProp = std::make_unique<CNetVars>();

		ss.clear();
		ss << std::oct << std::nouppercase;
		ss << XorStr("NetPropTable Got: ") << NetProp->GetCount();
		Utils::log(ss.str().c_str());
	}

	if (Engine != nullptr)
	{
		// 这个指针会在 CBaseClientState::FullConnect 里创建，在 CBaseClientState::Disconnect 里释放
		// 创建时调用 NET_CreateNetChannel，释放时调用 INetChannel::Shutdown
		NetChannel = reinterpret_cast<INetChannel*>(Engine->GetNetChannelInfo());
		PRINT_OFFSET(XorStr("INetChannel"), NetChannel);
	}

	if (Cvar)
	{
		ConVar_Register(FCVAR_NONE, nullptr);
	}

	MoveHelper = **reinterpret_cast<IMoveHelper***>(Utils::FindPattern(XorStr("client.dll"), SIG_MOVE_HELPER) + 1);
	PRINT_OFFSET(XorStr("IMoveHelper"), MoveHelper);

	HMODULE vstdlib = GetModuleHandleA(XorStr("vstdlib.dll"));
	if (vstdlib != NULL)
	{
		RandomSeed = reinterpret_cast<FnRandomSeed>(GetProcAddress(vstdlib, XorStr("RandomSeed")));
		RandomFloat = reinterpret_cast<FnRandomFloat>(GetProcAddress(vstdlib, XorStr("RandomFloat")));
		RandomFloatExp = reinterpret_cast<FnRandomFloatExp>(GetProcAddress(vstdlib, XorStr("RandomFloatExp")));
		RandomInt = reinterpret_cast<FnRandomInt>(GetProcAddress(vstdlib, XorStr("RandomInt")));
		RandomGaussianFloat = reinterpret_cast<FnRandomGaussianFloat>(GetProcAddress(vstdlib, XorStr("RandomGaussianFloat")));
		InstallUniformRandomStream = reinterpret_cast<FnInstallUniformRandomStream>(GetProcAddress(vstdlib, XorStr("InstallUniformRandomStream")));
		GetKeyValuesSystem = reinterpret_cast<FnKeyValuesSystem>(GetProcAddress(vstdlib, XorStr("KeyValuesSystem")));

		PRINT_OFFSET(XorStr("RandomSeed"), RandomSeed);
		PRINT_OFFSET(XorStr("RandomFloat"), RandomFloat);
		PRINT_OFFSET(XorStr("RandomFloatExp"), RandomFloatExp);
		PRINT_OFFSET(XorStr("RandomInt"), RandomInt);
		PRINT_OFFSET(XorStr("RandomGaussianFloat"), RandomGaussianFloat);
		PRINT_OFFSET(XorStr("InstallUniformRandomStream"), InstallUniformRandomStream);
		PRINT_OFFSET(XorStr("KeyValuesSystem"), GetKeyValuesSystem);
	}

	WriteUserCmd = reinterpret_cast<FnWriteUsercmd>(Utils::FindPattern(XorStr("client.dll"), SIG_WRITE_USERCMD));
	PRINT_OFFSET(XorStr("WriteUserCmd"), WriteUserCmd);

	StartDrawing = reinterpret_cast<FnStartDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_START_DRAWING));
	PRINT_OFFSET(XorStr("StartDrawing"), StartDrawing);

	FinishDrawing = reinterpret_cast<FnFinishDrawing>(Utils::FindPattern(XorStr("vguimatsurface.dll"), SIG_FINISH_DRAWING));
	PRINT_OFFSET(XorStr("FinishDrawing"), FinishDrawing);

	//FnSharedRandomFloat SharedRandomFloatOld = reinterpret_cast<FnSharedRandomFloat>(Utils::FindPattern(XorStr("client.dll"), SIG_SHARED_RANDOM_FLOAT_OLD));
	SharedRandomFloat = reinterpret_cast<FnSharedRandomFloat>(Utils::CalcInstAddress(Utils::FindPattern(XorStr("client.dll"), SIG_SHARED_RANDOM_FLOAT)));
	PRINT_OFFSET(XorStr("SharedRandomFloat"), SharedRandomFloat);
	//PRINT_OFFSET(XorStr("SharedRandomFloatOld"), SharedRandomFloatOld);

	SetPredictionRandomSeed = reinterpret_cast<FnSetPredictionRandomSeed>(Utils::FindPattern(XorStr("client.dll"), SIG_SET_RANDOM_SEED));
	PRINT_OFFSET(XorStr("SetPredictionRandomSeed"), SetPredictionRandomSeed);

	TraceLine2 = reinterpret_cast<FnTraceLine2>(Utils::FindPattern(XorStr("client.dll"), SIG_TRACE_LINE2));
	PRINT_OFFSET(XorStr("TraceLine2"), TraceLine2);

	TraceLine = reinterpret_cast<FnTraceLine>(Utils::FindPattern(XorStr("client.dll"), SIG_TRACE_LINE));
	PRINT_OFFSET(XorStr("TraceLine"), TraceLine);

	ClipTraceToPlayers = reinterpret_cast<FnClipTraceToPlayers>(Utils::FindPattern(XorStr("client.dll"), SIG_CLIP_TRACE_PLAYER));
	PRINT_OFFSET(XorStr("ClipTraceToPlayers"), ClipTraceToPlayers);

	if (GetKeyValuesSystem != nullptr && KeyValueSystem == nullptr)
	{
		KeyValueSystem = GetKeyValuesSystem();
		PRINT_OFFSET(XorStr("KeyValueSystem"), KeyValueSystem);
	}

	PlayerResource = **reinterpret_cast<CBasePlayerResource***>(Utils::FindPattern(XorStr("client.dll"), SIG_RESOURCE_POINTER) + 1);
	PRINT_OFFSET(XorStr("TerrorPlayerResource"), PlayerResource);
}

// 这个用不了，匹配到了错误的地址
CGlobalVarsBase * CClientInterface::FindGlobalVars()
{
	if (Client == nullptr)
		return nullptr;

	DWORD initAddr = GET_VFUNC(Client, 0);
	for (DWORD i = 0; i <= 0xFF; ++i)
	{
		// 前面有个 E8 A3 的 call，要忽略掉
		if (*reinterpret_cast<PBYTE>(initAddr + i) == 0xA3 && *reinterpret_cast<PBYTE>(initAddr) != 0xE8)
			return **reinterpret_cast<CGlobalVarsBase***>(initAddr + i + 1);
	}

	return nullptr;
}

template<typename T>
T* CClientInterface::GetPointer(const std::string & modules, const std::string & factory)
{
	HMODULE module = GetModuleHandleA(modules.c_str());
	if (module == nullptr)
		return nullptr;

	CreateInterfaceFn CreateInterface = reinterpret_cast<CreateInterfaceFn>(
		GetProcAddress(module, XorStr("CreateInterface")));

	if (CreateInterface == nullptr)
		return nullptr;

	std::stringstream ss;
	T* pointer = reinterpret_cast<T*>(CreateInterface(factory.c_str(), NULL));

	if (pointer != nullptr)
	{
		ss << factory << XorStr(" - Found: 0x");
		ss << std::hex << std::uppercase << pointer;
		Utils::log(ss.str().c_str());

		return pointer;
	}

	char buffer[1024];

	for (int i = 1; i < 100; ++i)
	{
		sprintf_s(buffer, "%s0%i", factory.c_str(), i);
		pointer = reinterpret_cast<T*>(CreateInterface(buffer, NULL));
		if (pointer != nullptr)
		{
			ss << buffer << XorStr(" - Found: 0x");
			ss << std::hex << std::uppercase << pointer;
			Utils::log(ss.str().c_str());
			return pointer;
		}

		sprintf_s(buffer, "%s00%i", factory.c_str(), i);
		pointer = reinterpret_cast<T*>(CreateInterface(buffer, NULL));
		if (pointer != nullptr)
		{
			ss << buffer << XorStr(" - Found: 0x");
			ss << std::hex << std::uppercase << pointer;
			Utils::log(ss.str().c_str());
			return pointer;
		}
	}

	return nullptr;
}

