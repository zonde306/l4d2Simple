#include "interfaces.h"
#include "indexes.h"
#include "../l4d2Simple2/xorstr.h"

std::unique_ptr<CClientInterface> g_pInterface;

#define PRINT_OFFSET(_name,_ptr)	{ss.str("");\
	ss << _name << XorStr(" - Found: 0x") << std::hex << std::uppercase << _ptr << std::oct << std::nouppercase;\
	Utils::log(ss.str().c_str());}

#define GET_VFUNC(_ptr,_off)	((*reinterpret_cast<PDWORD*>(_ptr))[_off])

#define SIG_GET_CLIENTMODE		XorStr("8B 0D ? ? ? ? 8B 01 8B 90 ? ? ? ? FF D2 8B 04 85 ? ? ? ? C3")
#define SIG_GET_CLIENTSTATE		XorStr("A1 ? ? ? ? 83 C0 08 C3")
#define SIG_MOVE_HELPER			XorStr("A1 ? ? ? ? 8B 10 8B 52 ? 81 C1")
#define SIG_GLOBAL_VARS			XorStr("8B 0D ? ? ? ? D9 41 ? 8B 55 ? 8B 45")

typedef IClientMode*(__cdecl *FnGetClientMode)();
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
	FileSystem = GetPointer<IBaseFileSystem>(XorStr("engine.dll"), XorStr("VBaseFileSystem"));
	Localize = GetPointer<ILocalize>(XorStr("localize.dll"), XorStr("Localize_"));
	StringTable = GetPointer<INetworkStringTableContainer>(XorStr("engine.dll"), XorStr("VEngineClientStringTable"));
	RenderView = GetPointer<IVRenderView>(XorStr("engine.dll"), XorStr("VEngineRenderView"));
	MaterialSystem = GetPointer<IMaterialSystem>(XorStr("materialsystem.dll"), XorStr("VMaterialSystem"));
	Sound = GetPointer<IEngineSound>(XorStr("engine.dll"), XorStr("IEngineSoundClient"));

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

	GlobalVars = FindGlobalVars();
	if (GlobalVars == nullptr)
		GlobalVars = **reinterpret_cast<CGlobalVarsBase***>(Utils::FindPattern(XorStr("client.dll"), SIG_GLOBAL_VARS) + 1);

	if (GlobalVars != nullptr)
	{
		PRINT_OFFSET(XorStr("CGlobalVarsBase"), GlobalVars);
	}
	
	if (Client != nullptr)
	{
		DWORD funcStart = GET_VFUNC(Client, indexes::CreateMove);
		Input = **reinterpret_cast<IInput***>(funcStart + 0x28);
		PRINT_OFFSET(XorStr("CInput"), Input);

		ClientMode = GetClientMode();
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
}

CGlobalVarsBase * CClientInterface::FindGlobalVars()
{
	if (Client == nullptr)
		return nullptr;

	DWORD initAddr = GET_VFUNC(Client, 0);
	for (DWORD i = 0; i <= 0xFF; ++i)
	{
		if (*reinterpret_cast<PBYTE>(initAddr + i) == 0xA3)
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

