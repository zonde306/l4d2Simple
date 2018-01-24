#pragma once
#include "./Interfaces/IBaseClientDLL.h"
#include "./Interfaces/IEngineClient.h"
#include "./Interfaces/IEngineTrace.h"
#include "./Interfaces/IClientEntityList.h"
#include "./Interfaces/IClientMode.h"
#include "./Interfaces/ICvar.h"
#include "./Interfaces/IGameEvent.h"
#include "./Interfaces/IGlobalVarsBase.h"
#include "./Interfaces/IInputSystem.h"
#include "./Interfaces/ISurface.h"
#include "./Interfaces/IVDebugOverlay.h"
#include "./Interfaces/IVPanel.h"
#include "./Interfaces/IModelInfo.h"
#include "./Interfaces/IGameMovement.h"
#include "./Interfaces/IVModelRender.h"
#include "./Interfaces/IMaterialSystem.h"
#include "./Interfaces/IMoveHelper.h"
#include "./Interfaces/IPrediction.h"
#include "./Interfaces/IRenderView.h"
#include "./Interfaces/IEngineVGui.h"
#include "./Interfaces/IVModelRender.h"
#include "./Interfaces/INetChannelInfo.h"
#include "./Interfaces/IBaseFileSystem.h"
#include "./Interfaces/ILocalize.h"
#include "./Interfaces/INetworkStringTable.h"
#include "./Structs/netprop.h"
#include <Windows.h>
#include <string>
#include <sstream>
#include <memory>

namespace interfaces
{
	extern IBaseClientDll* Client;
	extern IEngineClient* Engine;
	extern IEngineTrace* Trace;
	extern IClientEntityList* EntList;
	extern ICvar* Cvar;
	extern IGameEvent* GameEvent;
	extern CGlobalVarsBase* GlobalVars;
	extern IInput* Input;
	extern ISurface* Surface;
	extern IVDebugOverlay* DebugOverlay;
	extern IVPanel* Panel;
	extern IModelInfo* ModelInfo;
	extern IGameMovement* GameMovement;
	extern IPrediction* Prediction;
	extern IMoveHelper* MoveHelper;
	extern IPlayerInfoManager* PlayerInfo;
	extern IInputSystem* InputSystem;
	extern IEngineVGui* EngineVGui;
	extern IVModelRender* ModelRender;
	extern INetChannelInfo* NetChannel;
	extern IBaseFileSystem* FileSystem;
	extern ILocalize* Localize;
	extern INetworkStringTableContainer* StringTable;
	extern IClientMode* ClientMode;

	extern std::unique_ptr<CNetVars> NetProp;

	// 初始化
	void InitAllInterfaces();
};
