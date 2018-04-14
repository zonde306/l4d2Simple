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
#include "./Interfaces/INetChannelInfo.h"
#include "./Interfaces/IBaseFileSystem.h"
#include "./Interfaces/ILocalize.h"
#include "./Interfaces/INetworkStringTable.h"
#include "./Interfaces/IUniformRandomStream.h"
#include "./Structs/netprop.h"
#include <Windows.h>
#include <string>
#include <sstream>
#include <memory>

class CClientInterface
{
public:
	IBaseClientDll* Client;
	IEngineClient* Engine;
	IEngineTrace* Trace;
	IClientEntityList* EntList;
	ICvar* Cvar;
	IGameEventManager2* GameEvent;
	CGlobalVarsBase* GlobalVars;
	IInput* Input;
	ISurface* Surface;
	IVDebugOverlay* DebugOverlay;
	IVPanel* Panel;
	IModelInfo* ModelInfo;
	IGameMovement* GameMovement;
	IPrediction* Prediction;
	IMoveHelper* MoveHelper;
	IPlayerInfoManager* PlayerInfo;
	IInputSystem* InputSystem;
	IEngineVGui* EngineVGui;
	IVModelRender* ModelRender;
	INetChannelInfo* NetChannel;
	IBaseFileSystem* FileSystem;
	ILocalize* Localize;
	INetworkStringTableContainer* StringTable;
	IClientMode* ClientMode;
	IVRenderView* RenderView;
	IMaterialSystem* MaterialSystem;

	std::unique_ptr<CNetVars> NetProp;

	// 初始化
	void Init();

protected:
	CGlobalVarsBase * FindGlobalVars();

	template<typename T>
	T* GetPointer(const std::string& modules, const std::string& factory);
};

extern std::unique_ptr<CClientInterface> g_pInterface;
