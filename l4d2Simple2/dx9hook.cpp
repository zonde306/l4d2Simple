#include "dx9hook.h"
#include <Windows.h>
#include <chrono>
#include <thread>
#include "utils.h"
#include "xorstr.h"
#include "drawing.h"
#include "../imgui/examples/directx9_example/imgui_impl_dx9.h"

std::unique_ptr<CDirectX9Hook> g_pDirextXHook;

CDirectX9Hook::CDirectX9Hook() : m_pVmtHook(nullptr), m_pVMT(nullptr), m_pD3D(nullptr), m_pDevice(nullptr),
	m_pOriginDevice(nullptr), m_bSuccessCreated(false), m_bIsFirstHooked(false), m_bIsSecondHooked(false)
{
}

/*
CDirectX9Hook::CDirectX9Hook(IDirect3DDevice9 * device) : CDirectX9Hook::CDirectX9Hook()
{
	if(device != nullptr)
		SetupSecondHook(device);
}
*/

CDirectX9Hook::~CDirectX9Hook()
{
	if (m_bSuccessCreated)
		ReleaseDevice();

	if (m_bIsFirstHooked)
	{
		for (DetourXS*& entity : m_vpDetourList)
		{
			if (entity != nullptr)
				delete entity;
			
			entity = nullptr;
		}

		m_vpDetourList.clear();
	}

	if (m_bIsSecondHooked && m_pVmtHook)
	{
		m_pVmtHook->UninstallHook();
		delete m_pVmtHook;
	}
}

void CDirectX9Hook::Init()
{
	if (m_bIsSecondHooked || m_bIsFirstHooked)
		return;

	while (!m_bSuccessCreated)
	{
		if (!CreateDevice())
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	SetupFirstHook();
	Utils::log(XorStr("CDirectX9Hook Initialization..."));
}

IDirect3DDevice9 * CDirectX9Hook::GetDevice()
{
	return m_pOriginDevice;
}

HRESULT WINAPI CDirectX9Hook::Hooked_DrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE type,
	INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_DrawIndexedPrimitive"));

#ifdef _DEBUG
	UINT offsetByte, stride;
	static IDirect3DVertexBuffer9* stream = nullptr;
	device->GetStreamSource(0, &stream, &offsetByte, &stride);

	if (stride == 32)
	{
		static DWORD oldZEnable;
		device->GetRenderState(D3DRS_ZENABLE, &oldZEnable);
		device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
		g_pDirextXHook->m_pfnDrawIndexedPrimitive(device, type, baseIndex, minIndex, numVertices, startIndex, primitiveCount);
		device->SetRenderState(D3DRS_ZENABLE, oldZEnable);
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}
#endif

	// 在这里使用 stride, numVertices, primitiveCount 检查是否为有用的模型
	// 然后通过修改 D3DRS_ZENABLE 和 D3DRS_ZFUNC 实现透视
	for (auto& func : g_pDirextXHook->m_vfnDrawIndexedPrimitive)
	{
		func(device, type, baseIndex, minIndex,
			numVertices, startIndex, primitiveCount);
	}

#ifdef _DEBUG
	static bool bHasFirst = true;
	if (bHasFirst)
	{
		bHasFirst = false;
		Utils::log(XorStr("Hook DrawIndexedPrimitive Success."));
	}
#endif

	return g_pDirextXHook->m_pfnDrawIndexedPrimitive(device, type, baseIndex, minIndex,
		numVertices, startIndex, primitiveCount);
}

HRESULT WINAPI CDirectX9Hook::Hooked_EndScene(IDirect3DDevice9* device)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_EndScene"));

	g_pDrawing->OnBeginEndScene();

	// 在这里使用 DrawIndexedPrimitive 或 DrawIndexedPrimitiveUp 绘制屏幕
	for (auto& func : g_pDirextXHook->m_vfnEndScene)
	{
		func(device);
	}

	g_pDrawing->OnFinishEndScene();
	g_pDrawing->OnGameFrame();

#ifdef _DEBUG
	static bool bHasFirst = true;
	if (bHasFirst)
	{
		bHasFirst = false;
		Utils::log(XorStr("Hook EndScene Success."));
	}
#endif

	return g_pDirextXHook->m_pfnEndScene(device);
}

HRESULT WINAPI CDirectX9Hook::Hooked_CreateQuery(IDirect3DDevice9* device, D3DQUERYTYPE type,
	IDirect3DQuery9** query)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_CreateQuery"));

	for (auto& func : g_pDirextXHook->m_vfnCreateQuery)
	{
		func(device, type, query);
	}

	// 简易的透视，但是会有屏幕变黑的 bug
	/*
	if (type == D3DQUERYTYPE_OCCLUSION)
		type = D3DQUERYTYPE_TIMESTAMP;
	*/

#ifdef _DEBUG
	static bool bHasFirst = true;
	if (bHasFirst)
	{
		bHasFirst = false;
		Utils::log(XorStr("Hook CreateQuery Success."));
	}
#endif

	return g_pDirextXHook->m_pfnCreateQuery(device, type, query);
}

HRESULT WINAPI CDirectX9Hook::Hooked_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_Reset"));

	g_pDrawing->OnLostDevice();

	HRESULT hr = g_pDirextXHook->m_pfnReset(device, pp);

	for (auto& func : g_pDirextXHook->m_vfnReset)
	{
		func(device, pp);
	}

	g_pDrawing->OnResetDevice();

#ifdef _DEBUG
	static bool bHasFirst = true;
	if (bHasFirst)
	{
		bHasFirst = false;
		Utils::log(XorStr("Hook Reset Success."));
	}
#endif

	return hr;
}

HRESULT WINAPI CDirectX9Hook::Hooked_Present(IDirect3DDevice9* device, const RECT* source,
	const RECT* dest, HWND window, const RGNDATA* region)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_Present"));

	g_pDrawing->OnBeginPresent();

	// 在这里使用 ImGui 绘制屏幕
	for (auto& func : g_pDirextXHook->m_vfnPresent)
	{
		func(device, source, dest, window, region);
	}

	g_pDrawing->OnFinishPresent();

#ifdef _DEBUG
	static bool bHasFirst = true;
	if (bHasFirst)
	{
		bHasFirst = false;
		Utils::log(XorStr("Hook Present Success."));
	}
#endif

	return g_pDirextXHook->m_pfnPresent(device, source, dest, window, region);
}

bool CDirectX9Hook::CreateDevice()
{
	HWND window = GetDesktopWindow();
	if (window == nullptr)
	{
		Utils::log(XorStr("GetDesktopWindow Failed."));
		return false;
	}

	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D == nullptr)
	{
		Utils::log(XorStr("Direct3DCreate9 Failed."));
		return false;
	}

	// 显示模式
	D3DDISPLAYMODE dm;
	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm)))
	{
		if (m_pD3D != nullptr)
			m_pD3D->Release();

		m_pD3D = nullptr;
		Utils::log(XorStr("GetAdapterDisplayMode Failed."));

		return false;
	}

	// 创建参数
	D3DPRESENT_PARAMETERS pp;
	ZeroMemory(&pp, sizeof(pp));
	pp.Windowed = TRUE;
	pp.hDeviceWindow = window;
	pp.BackBufferCount = 1;
	pp.BackBufferWidth = 4;
	pp.BackBufferHeight = 4;
	pp.BackBufferFormat = dm.Format;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	m_pDevice = nullptr;
	HRESULT hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &pp, &m_pDevice);

	if (FAILED(hr))
	{
		if(hr == D3DERR_INVALIDCALL)
			Utils::log(XorStr("CreateDevice Failed: D3DERR_INVALIDCALL"));
		else if(hr == D3DERR_DEVICELOST)
			Utils::log(XorStr("CreateDevice Failed: D3DERR_DEVICELOST"));
		else if(hr == D3DERR_NOTAVAILABLE)
			Utils::log(XorStr("CreateDevice Failed: D3DERR_NOTAVAILABLE"));
		else if(hr == D3DERR_OUTOFVIDEOMEMORY)
			Utils::log(XorStr("CreateDevice Failed."));

		return false;
	}

	if (m_pDevice == nullptr || m_pD3D == nullptr)
	{
		if (m_pDevice != nullptr)
			m_pDevice->Release();
		if (m_pD3D != nullptr)
			m_pD3D->Release();

		m_pDevice = nullptr;
		m_pD3D = nullptr;

		Utils::log(XorStr("Unknown Failed."));
		return false;
	}

	m_bSuccessCreated = true;
	Utils::log(XorStr("Create Fake Device."));

	return true;
}

bool CDirectX9Hook::ReleaseDevice()
{
	if (!m_bSuccessCreated)
		return false;
	
	if (m_pDevice != nullptr)
		m_pDevice->Release();
	if (m_pD3D != nullptr)
		m_pD3D->Release();

	m_pDevice = nullptr;
	m_pD3D = nullptr;
	m_bSuccessCreated = false;
	Utils::log(XorStr("Release Fake Device."));

	return true;
}

bool CDirectX9Hook::SetupFirstHook()
{
	if (!m_bSuccessCreated || m_bIsSecondHooked || m_bIsFirstHooked || m_pOriginDevice != nullptr)
		return false;

	m_pVMT = *(reinterpret_cast<PDWORD*>(m_pDevice));
	DetourXS* detour = new DetourXS(reinterpret_cast<LPVOID>(m_pVMT[16]), Hooked_Reset);
	m_pfnReset = reinterpret_cast<FnReset>(detour->GetTrampoline());
	m_vpDetourList.push_back(std::move(detour));

	detour = new DetourXS(reinterpret_cast<LPVOID>(m_pVMT[17]), Hooked_Present);
	m_pfnPresent = reinterpret_cast<FnPresent>(detour->GetTrampoline());
	m_vpDetourList.push_back(std::move(detour));

	detour = new DetourXS(reinterpret_cast<LPVOID>(m_pVMT[42]), Hooked_EndScene);
	m_pfnEndScene = reinterpret_cast<FnEndScene>(detour->GetTrampoline());
	m_vpDetourList.push_back(std::move(detour));

	detour = new DetourXS(reinterpret_cast<LPVOID>(m_pVMT[82]), Hooked_DrawIndexedPrimitive);
	m_pfnDrawIndexedPrimitive = reinterpret_cast<FnDrawIndexedPrimitive>(detour->GetTrampoline());
	m_vpDetourList.push_back(std::move(detour));

	detour = new DetourXS(reinterpret_cast<LPVOID>(m_pVMT[118]), Hooked_CreateQuery);
	m_pfnCreateQuery = reinterpret_cast<FnCreateQuery>(detour->GetTrampoline());
	m_vpDetourList.push_back(std::move(detour));

	m_bIsFirstHooked = true;
	Utils::log(XorStr("Setup DirectX Hook 1st."));

	return true;
}

bool CDirectX9Hook::SetupSecondHook(IDirect3DDevice9* device)
{
	if (m_bIsSecondHooked)
		return false;

	if (m_bSuccessCreated)
		ReleaseDevice();

	if (m_bIsFirstHooked)
	{
		for (DetourXS*& entity : m_vpDetourList)
		{
			if (entity != nullptr)
				delete entity;

			entity = nullptr;
		}

		m_vpDetourList.clear();
		m_bIsFirstHooked = false;
	}

	m_pOriginDevice = device;
	m_pVmtHook = new CVmtHook(device);
	m_pfnReset = reinterpret_cast<FnReset>(m_pVmtHook->HookFunction(16, Hooked_Reset));
	m_pfnPresent = reinterpret_cast<FnPresent>(m_pVmtHook->HookFunction(17, Hooked_Present));
	m_pfnEndScene = reinterpret_cast<FnEndScene>(m_pVmtHook->HookFunction(42, Hooked_EndScene));
	m_pfnDrawIndexedPrimitive = reinterpret_cast<FnDrawIndexedPrimitive>(m_pVmtHook->HookFunction(82, Hooked_DrawIndexedPrimitive));
	m_pfnCreateQuery = reinterpret_cast<FnCreateQuery>(m_pVmtHook->HookFunction(118, Hooked_CreateQuery));
	m_pVmtHook->InstallHook();

	m_bIsSecondHooked = true;
	Utils::log(XorStr("Setup DirectX Hook 2nd."));

	return true;
}

bool CDirectX9Hook::CheckHookStatus(IDirect3DDevice9 * device)
{
	bool doInit = false;
	
	if (!m_bIsSecondHooked)
	{
		SetupSecondHook(device);
		doInit = true;
	}

	if (!g_pDrawing)
	{
		g_pDrawing = std::make_unique<CDrawing>();
		g_pDrawing->Init(device);
		doInit = true;
	}

	return doInit;
}

std::vector<FnDrawIndexedPrimitive>& CDirectX9Hook::GetHookList(FnDrawIndexedPrimitive)
{
	return m_vfnDrawIndexedPrimitive;
}

std::vector<FnEndScene>& CDirectX9Hook::GetHookList(FnEndScene)
{
	return m_vfnEndScene;
}

std::vector<FnCreateQuery>& CDirectX9Hook::GetHookList(FnCreateQuery)
{
	return m_vfnCreateQuery;
}

std::vector<FnReset>& CDirectX9Hook::GetHookList(FnReset)
{
	return m_vfnReset;
}

std::vector<FnPresent>& CDirectX9Hook::GetHookList(FnPresent)
{
	return m_vfnPresent;
}

HRESULT CDirectX9Hook::CallOriginalFunction(FnDrawIndexedPrimitive, IDirect3DDevice9* device, D3DPRIMITIVETYPE type,
	INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount)
{
	return m_pfnDrawIndexedPrimitive(device, type, baseIndex, minIndex, numVertices, startIndex, primitiveCount);
}

HRESULT CDirectX9Hook::CallOriginalFunction(FnEndScene, IDirect3DDevice9* device)
{
	return m_pfnEndScene(device);
}

HRESULT CDirectX9Hook::CallOriginalFunction(FnCreateQuery, IDirect3DDevice9* device, D3DQUERYTYPE type,
	IDirect3DQuery9** query)
{
	return m_pfnCreateQuery(device, type, query);
}

HRESULT CDirectX9Hook::CallOriginalFunction(FnReset, IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
{
	return m_pfnReset(device, pp);
}

HRESULT CDirectX9Hook::CallOriginalFunction(FnPresent, IDirect3DDevice9* device, const RECT* source,
	const RECT* dest, HWND window, const RGNDATA* region)
{
	return m_pfnPresent(device, source, dest, window, region);
}

bool CDirectX9Hook::AddHook_DrawIndexedPrimitive(FnDrawIndexedPrimitive function)
{
	return _FunctionHook(m_vfnDrawIndexedPrimitive, function);
}

bool CDirectX9Hook::AddHook_EndScene(FnEndScene function)
{
	return _FunctionHook(m_vfnEndScene, function);
}

bool CDirectX9Hook::AddHook_CreateQuery(FnCreateQuery function)
{
	return _FunctionHook(m_vfnCreateQuery, function);
}

bool CDirectX9Hook::AddHook_Reset(FnReset function)
{
	return _FunctionHook(m_vfnReset, function);
}

bool CDirectX9Hook::AddHook_Present(FnPresent function)
{
	return _FunctionHook(m_vfnPresent, function);
}

bool CDirectX9Hook::RemoveHook_DrawIndexedPrimitive(FnDrawIndexedPrimitive function)
{
	return _FunctionUnhook(m_vfnDrawIndexedPrimitive, function);
}

bool CDirectX9Hook::RemoveHook_EndScene(FnEndScene function)
{
	return _FunctionUnhook(m_vfnEndScene, function);
}

bool CDirectX9Hook::RemoveHook_CreateQuery(FnCreateQuery function)
{
	return _FunctionUnhook(m_vfnCreateQuery, function);
}

bool CDirectX9Hook::RemoveHook_Reset(FnReset function)
{
	return _FunctionUnhook(m_vfnReset, function);
}

bool CDirectX9Hook::RemoveHook_Present(FnPresent function)
{
	return _FunctionUnhook(m_vfnPresent, function);
}

HRESULT CDirectX9Hook::CallOriginal_DrawIndexedPrimitive(D3DPRIMITIVETYPE type,
	INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount)
{
	return m_pfnDrawIndexedPrimitive(m_pOriginDevice, type, baseIndex, minIndex, numVertices,
		startIndex, primitiveCount);
}

HRESULT CDirectX9Hook::CallOriginal_EndScene()
{
	return m_pfnEndScene(m_pOriginDevice);
}

HRESULT CDirectX9Hook::CallOriginal_CreateQuery(D3DQUERYTYPE type, IDirect3DQuery9** query)
{
	return m_pfnCreateQuery(m_pOriginDevice, type, query);
}

HRESULT CDirectX9Hook::CallOriginal_Reset(D3DPRESENT_PARAMETERS *pp)
{
	return m_pfnReset(m_pOriginDevice, pp);
}

HRESULT CDirectX9Hook::CallOriginal_Present(const RECT* source, const RECT* dest,
	HWND window, const RGNDATA* region)
{
	return m_pfnPresent(m_pOriginDevice, source, dest, window, region);
}

HRESULT CDirectX9Hook::CallOriginal_DrawIndexedPrimitive(IDirect3DDevice9* device,
	D3DPRIMITIVETYPE type, INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount)
{
	return m_pfnDrawIndexedPrimitive(device, type, baseIndex, minIndex, numVertices,
		startIndex, primitiveCount);
}

HRESULT CDirectX9Hook::CallOriginal_EndScene(IDirect3DDevice9* device)
{
	return m_pfnEndScene(device);
}

HRESULT CDirectX9Hook::CallOriginal_CreateQuery(IDirect3DDevice9* device, D3DQUERYTYPE type,
	IDirect3DQuery9** query)
{
	return m_pfnCreateQuery(device, type, query);
}

HRESULT CDirectX9Hook::CallOriginal_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS *pp)
{
	return m_pfnReset(device, pp);
}

HRESULT CDirectX9Hook::CallOriginal_Present(IDirect3DDevice9* device, const RECT* source,
	const RECT* dest, HWND window, const RGNDATA* region)
{
	return m_pfnPresent(device, source, dest, window, region);
}

template<typename Fn>
bool CDirectX9Hook::HookFunction(Fn function)
{
#ifdef _DEBUG
	static_assert(_DeclTypeCall<Fn>::GetHookList() == nullptr);
#endif
	
	const std::vector<Fn>& list = *_DeclTypeCall<Fn>::GetHookList();;
	if (std::find(list.cbegin(), list.cend(), function) != list.cend())
		return false;

	list.push_back(function);
	return true;
}

template<typename Fn>
bool CDirectX9Hook::UnhookFunction(Fn function)
{
#ifdef _DEBUG
	static_assert(_DeclTypeCall<Fn>::GetHookList() == nullptr);
#endif
	
	std::vector<Fn>& list = *_DeclTypeCall<Fn>::GetHookList();
	std::vector<Fn>::iterator iter = std::find(list.begin(), list.end(), function);
	if (iter != list.end())
	{
		list.erase(iter);
		return true;
	}

	return false;
}

template<typename Fn, typename ...Arg>
HRESULT CDirectX9Hook::InvokeOriginal(Fn, Arg ...arg)
{
#ifdef _DEBUG
	static_assert(_DeclTypeCall<Fn>::GetFunction() == nullptr);
#endif

	return _DeclTypeCall<Fn>::Invoke(std::forward<Arg>(arg)...);
}

template<typename Fn>
bool CDirectX9Hook::_FunctionHook(std::vector<Fn>& list, Fn function)
{
#ifdef _DEBUG
	static_assert(function == nullptr);
#endif

	if (!list.empty() && std::find(list.cbegin(), list.cend(), function) != list.cend())
		return false;

	list.emplace_back(function);
	return true;
}

template<typename Fn>
bool CDirectX9Hook::_FunctionUnhook(std::vector<Fn>& list, Fn function)
{
#ifdef _DEBUG
	static_assert(function == nullptr);
#endif
	if (list.empty())
		return false;

	auto it = std::find(list.begin(), list.end(), function);
	if (it == list.end())
		return false;

	list.erase(it);
	return true;
}

template<typename Fn>
template<typename ...Arg>
HRESULT _DeclTypeCall<Fn>::Invoke(Arg ...arg)
{
	return E_NOTIMPL;
}

template<typename Fn>
Fn _DeclTypeCall<Fn>::GetFunction()
{
	return NULL;
}

template<typename Fn>
std::vector<Fn>* _DeclTypeCall<Fn>::GetHookList()
{
	return nullptr;
}

template<>
template<typename ...Arg>
HRESULT _DeclTypeCall<FnEndScene>::Invoke(Arg ...arg)
{
	return g_pDirextXHook->m_pfnEndScene(std::forward<Arg>(arg)...);
}

template<>
FnEndScene _DeclTypeCall<FnEndScene>::GetFunction()
{
	return g_pDirextXHook->m_pfnEndScene;
}

template<>
std::vector<FnEndScene>* _DeclTypeCall<FnEndScene>::GetHookList()
{
	return &g_pDirextXHook->m_vfnEndScene;
}

template<>
template<typename ...Arg>
HRESULT _DeclTypeCall<FnPresent>::Invoke(Arg ...arg)
{
	return g_pDirextXHook->m_pfnPresent(std::forward<Arg>(arg)...);
}

template<>
FnPresent _DeclTypeCall<FnPresent>::GetFunction()
{
	return g_pDirextXHook->m_pfnPresent;
}

template<>
std::vector<FnPresent>* _DeclTypeCall<FnPresent>::GetHookList()
{
	return &g_pDirextXHook->m_vfnPresent;
}

template<>
template<typename ...Arg>
HRESULT _DeclTypeCall<FnCreateQuery>::Invoke(Arg ...arg)
{
	return g_pDirextXHook->m_pfnCreateQuery(std::forward<Arg>(arg)...);
}

template<>
FnCreateQuery _DeclTypeCall<FnCreateQuery>::GetFunction()
{
	return g_pDirextXHook->m_pfnCreateQuery;
}

template<>
std::vector<FnCreateQuery>* _DeclTypeCall<FnCreateQuery>::GetHookList()
{
	return &g_pDirextXHook->m_vfnCreateQuery;
}

template<>
template<typename ...Arg>
HRESULT _DeclTypeCall<FnReset>::Invoke(Arg ...arg)
{
	return g_pDirextXHook->m_pfnReset(std::forward<Arg>(arg)...);
}

template<>
FnReset _DeclTypeCall<FnReset>::GetFunction()
{
	return g_pDirextXHook->m_pfnReset;
}

template<>
std::vector<FnReset>* _DeclTypeCall<FnReset>::GetHookList()
{
	return &g_pDirextXHook->m_vfnReset;
}

template<>
template<typename ...Arg>
HRESULT _DeclTypeCall<FnDrawIndexedPrimitive>::Invoke(Arg ...arg)
{
	return g_pDirextXHook->m_pfnDrawIndexedPrimitive(std::forward<Arg>(arg)...);
}

template<>
FnDrawIndexedPrimitive _DeclTypeCall<FnDrawIndexedPrimitive>::GetFunction()
{
	return g_pDirextXHook->m_pfnDrawIndexedPrimitive;
}

template<>
std::vector<FnDrawIndexedPrimitive>* _DeclTypeCall<FnDrawIndexedPrimitive>::GetHookList()
{
	return &g_pDirextXHook->m_vfnDrawIndexedPrimitive;
}
