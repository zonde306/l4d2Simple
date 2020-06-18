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
#include "./Interfaces/IEngineSound.h"
#include "./interfaces/IBaseClientState.h"
#include "./Interfaces/IKeyValuesSystem.h"
#include "./Structs/baseplayerresource.h"
#include "./Structs/netprop.h"
#include "./Structs/viewsetup.h"
#include <Windows.h>
#include <string>
#include <sstream>
#include <memory>

// 搜索到的函数
using FnStartDrawing = void(__thiscall*)(ISurface* _this);
using FnFinishDrawing = void(__thiscall*)(ISurface* _this);
using FnWriteUsercmd = void(__cdecl*)(bf_write* buf, CUserCmd* to, CUserCmd* from);
using FnSetPredictionRandomSeed = int(__cdecl*)(int seed);
using FnSharedRandomFloat = void(__cdecl*)(const char* name, float min, float max, int slots);
using FnTraceLine2 = void(__cdecl*)(const Vector& start, const Vector& end, unsigned int mask, const IHandleEntity* ignore, int collisionGroup, trace_t* result);
using FnTraceLine = void(__cdecl*)(const Vector& start, const Vector& end, unsigned int mask, ITraceFilter* filter, trace_t* result);
using FnClipTraceToPlayers = void(__cdecl*)(const Vector& start, const Vector& end, unsigned int mask, ITraceFilter* filter, trace_t* result);

// 导出函数
using FnRandomSeed = void(__cdecl*)(int iSeed);
using FnRandomFloat = float(__cdecl*)(float flMinVal, float flMaxVal);
using FnRandomFloatExp = float(__cdecl*)(float flMinVal, float flMaxVal, float flExponent);
using FnRandomInt = int(__cdecl*)(int iMinVal, int iMaxVal);
using FnRandomGaussianFloat = float(__cdecl*)(float flMean, float flStdDev);
using FnInstallUniformRandomStream = void(__cdecl*)(IUniformRandomStream* pStream);
using FnKeyValuesSystem = IKeyValuesSystem*(__cdecl*)();

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
	IFileSystem* FileSystem;
	ILocalize* Localize;
	INetworkStringTableContainer* StringTable;
	IVRenderView* RenderView;
	IMaterialSystem* MaterialSystem;
	IEngineSound* Sound;
	IKeyValuesSystem* KeyValueSystem;
	CBasePlayerResource* PlayerResource;

	// 以下功能可能会随着连接服务器而改变，最好每次加入游戏后都重新获取一遍
	// 最好不要直接使用这个，而是调用 IEngineClient::GetNetChannelInfo 然后转换类型
	INetChannel* NetChannel;
	CBaseClientState* ClientState;
	IClientMode* ClientMode;

	std::unique_ptr<CNetVars> NetProp;

	// 初始化
	void Init();

public:
	// 搜索特征码得到的函数
	FnStartDrawing StartDrawing;
	FnFinishDrawing FinishDrawing;
	FnWriteUsercmd WriteUserCmd;
	FnSharedRandomFloat SharedRandomFloat;
	FnSetPredictionRandomSeed SetPredictionRandomSeed;
	FnTraceLine2 TraceLine2;
	FnTraceLine TraceLine;
	FnClipTraceToPlayers ClipTraceToPlayers;

	// 通过导出表得到的函数
	FnRandomSeed RandomSeed;
	FnRandomFloat RandomFloat;
	FnRandomFloatExp RandomFloatExp;
	FnRandomInt RandomInt;
	FnRandomGaussianFloat RandomGaussianFloat;
	FnInstallUniformRandomStream InstallUniformRandomStream;
	FnKeyValuesSystem GetKeyValuesSystem;

protected:
	CGlobalVarsBase * FindGlobalVars();

	template<typename T>
	T* GetPointer(const std::string& modules, const std::string& factory);
};

extern std::unique_ptr<CClientInterface> g_pInterface;
