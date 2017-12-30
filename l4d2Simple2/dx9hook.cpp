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

HRESULT WINAPI Hooked_DrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE type,
	INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_DrawIndexedPrimitive"));

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

HRESULT WINAPI Hooked_EndScene(IDirect3DDevice9* device)
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

HRESULT WINAPI Hooked_CreateQuery(IDirect3DDevice9* device, D3DQUERYTYPE type, IDirect3DQuery9** query)
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

HRESULT WINAPI Hooked_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
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

HRESULT WINAPI Hooked_Present(IDirect3DDevice9* device, const RECT* source,
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

template<typename Fn>
bool CDirectX9Hook::HookFunction(Fn function)
{
	const std::vector<Fn>& list = GetHookList(function);
	for (std::find(list.cbegin(), list.cend(), function) != list.cend())
		return false;

	list.push_back(function);
	return true;
}

template<typename Fn>
bool CDirectX9Hook::UnhookFunction(Fn function)
{
	std::vector<Fn>& list = GetHookList(function);
	std::vector<Fn>::iterator iter = std::find(list.begin(), list.end(), function);
	if (iter != list.end())
	{
		list.erase(iter);
		return true;
	}

	return false;
}
