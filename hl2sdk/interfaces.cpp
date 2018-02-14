#include "interfaces.h"
#include "indexes.h"
#include "../l4d2Simple2/xorstr.h"

std::unique_ptr<CClientInterface> g_pClientInterface;

#define PRINT_OFFSET(_name,_ptr)	{ss.str("");\
	ss << _name << XorStr(" - Found: 0x") << std::hex << std::uppercase << _ptr << std::oct << std::nouppercase;\
	Utils::log(ss.str().c_str());}

#define GET_VFUNC(_ptr,_off)	((*reinterpret_cast<PDWORD*>(_ptr))[_off])

void CClientInterface::Init()
{
	Client = GetPointer<IBaseClientDll>(XorStr("client.dll"), XorStr("VClient"));
	Engine = GetPointer<IEngineClient>(XorStr("engine.dll"), XorStr("VEngineClient"));
	Trace = GetPointer<IEngineTrace>(XorStr("engine.dll"), XorStr("EngineTraceClient"));
	EntList = GetPointer<IClientEntityList>(XorStr("client.dll"), XorStr("VClientEntityList"));
	Cvar = GetPointer<ICvar>(XorStr("vstdlib.dll"), XorStr("VEngineCvar"));
	GameEvent = GetPointer<IGameEvent>(XorStr("engine.dll"), XorStr("GAMEEVENTSMANAGER002"));
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

	GlobalVars = FindGlobalVars();
	if (GlobalVars != nullptr)
	{
		PRINT_OFFSET(XorStr("CGlobalVarsBase"), GlobalVars);
	}
	
	if (Client != nullptr)
	{
		DWORD funcStart = GET_VFUNC(Client, indexes::CreateMove);
		DWORD offsetStart = Utils::FindPattern(funcStart, funcStart + 0x51,
			XorStr("8B 0D ? ? ? ? FF 75 10 D9 45 0C"));
		if (offsetStart != NULL)
			Input = **reinterpret_cast<IInput***>(offsetStart + 0x2);
		else
			Input = **reinterpret_cast<IInput***>(funcStart + 0x20);

		PRINT_OFFSET(XorStr("CInput"), Input);

		funcStart = GET_VFUNC(Client, indexes::HudProcessInput);
		offsetStart = Utils::FindPattern(funcStart, funcStart + 0x51,
			XorStr("8B 0D ? ? ? ? FF 75 10 D9 45 0C"));
		if (offsetStart != NULL)
			ClientMode = *reinterpret_cast<IClientMode**>(offsetStart + 0x2);
		else
			ClientMode = *reinterpret_cast<IClientMode**>(funcStart + 0x5);

		PRINT_OFFSET(XorStr("ClientMode"), Input);

		NetProp = std::make_unique<CNetVars>();

		ss.clear();
		ss << std::oct << std::nouppercase;
		ss << XorStr("NetPropTable Got: ") << NetProp->GetCount();
		Utils::log(ss.str().c_str());
	}

	if (Engine != nullptr)
	{
		NetChannel = Engine->GetNetChannelInfo();

		PRINT_OFFSET(XorStr("INetChannelInfo"), NetChannel);
	}

	if (Cvar)
	{
		ConVar_Register(FCVAR_NONE, nullptr);
	}
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

