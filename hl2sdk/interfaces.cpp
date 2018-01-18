#include "interfaces.h"
#include "indexes.h"
#include "../l4d2Simple2/xorstr.h"

namespace interfaces
{
	IBaseClientDll* Client = nullptr;
	IEngineClient* Engine = nullptr;
	IEngineTrace* Trace = nullptr;
	IClientEntityList* EntList = nullptr;
	ICvar* Cvar = nullptr;
	IGameEvent* GameEvent = nullptr;
	CGlobalVarsBase* GlobalVars = nullptr;
	IInput* Input = nullptr;
	ISurface* Surface = nullptr;
	IVDebugOverlay* DebugOverlay = nullptr;
	IVPanel* Panel = nullptr;
	IModelInfo* ModelInfo = nullptr;
	IGameMovement* GameMovement = nullptr;
	IPrediction* Prediction = nullptr;
	IMoveHelper* MoveHelper = nullptr;
	IPlayerInfoManager* PlayerInfo = nullptr;
	IInputSystem* InputSystem = nullptr;
	IEngineVGui* EngineVGui = nullptr;
	IVModelRender* ModelRender = nullptr;
	INetChannelInfo* NetChannel = nullptr;
	IBaseFileSystem* FileSystem = nullptr;

	template<typename T>
	T* GetPointer(const std::string& modules, const std::string& factory);

	// 搜索全局变量
	CGlobalVarsBase* FindGlobalVars();
};

void interfaces::InitAllInterfaces()
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

	std::stringstream ss;

	if (PlayerInfo != nullptr)
	{
		GlobalVars = PlayerInfo->GetGlobalVars();
		if (GlobalVars != nullptr)
		{
			ss << XorStr("CGlobalVarsBase Got: 0x");
			ss << std::hex << std::uppercase << GlobalVars;
			Utils::log(ss.str().c_str());
		}
	}
	if (GlobalVars == nullptr)
	{
		GlobalVars = FindGlobalVars();
		if (GlobalVars != nullptr)
		{
			ss << XorStr("CGlobalVarsBase Found: 0x");
			ss << std::hex << std::uppercase << GlobalVars;
			Utils::log(ss.str().c_str());
		}
	}
	

	if (Client != nullptr)
	{
		Input = **reinterpret_cast<decltype(Input)**>(
			(*(reinterpret_cast<PDWORD*>(Client))[indexes::CreateMove]) + 0x20);
		
		ss.clear();
		ss << XorStr("IInput Got: 0x");
		ss << std::hex << std::uppercase << Input;
		Utils::log(ss.str().c_str());
	}

	if (Engine != nullptr)
	{
		NetChannel = Engine->GetNetChannelInfo();

		ss.clear();
		ss << XorStr("INetChannelInfo Got: 0x");
		ss << std::hex << std::uppercase << Input;
		Utils::log(ss.str().c_str());
	}
}

CGlobalVarsBase * interfaces::FindGlobalVars()
{
	if (Client == nullptr)
		return nullptr;

	DWORD initAddr = (*reinterpret_cast<PDWORD*>(Client))[0];
	for (DWORD i = 0; i <= 0xFF; ++i)
	{
		if (*reinterpret_cast<PBYTE>(initAddr + i) == 0xA3)
			return **reinterpret_cast<CGlobalVarsBase***>(initAddr + i + 1);
	}

	return nullptr;
}


template<typename T>
T* interfaces::GetPointer(const std::string & modules, const std::string & factory)
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

