#include "dx9hook.h"
#include <Windows.h>
#include <chrono>
#include <thread>
#include "utils.h"
#include "xorstr.h"
#include "drawing.h"
#include "menu.h"
#include "../imgui/examples/imgui_impl_dx9.h"

std::unique_ptr<CDirectX9Hook> g_pDirextXHook;

// .text:10005D5C A1 88 39 17 10              mov     eax, g_pD3DDevice
#define SIG_MOV_DIRECT_PTR		XorStr("A1 ? ? ? ? 50 8B CE E8 ? ? ? ? 84 DB 75 0F")

#define DECL_DESTORY_DETOURXS(_name)	if(_name && _name->Created())\
	_name->Destroy()

#pragma comment(lib, "d3d9")
#pragma comment(lib, "d3dx9")

CDirectX9Hook::CDirectX9Hook() : m_pVMTHook(nullptr), m_pD3D(nullptr), m_pDevice(nullptr),
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
		DECL_DESTORY_DETOURXS(m_pHookDrawIndexedPrimitive);
		DECL_DESTORY_DETOURXS(m_pHookEndScene);
		DECL_DESTORY_DETOURXS(m_pHookCreateQuery);
		DECL_DESTORY_DETOURXS(m_pHookReset);
		DECL_DESTORY_DETOURXS(m_pHookPresent);
	}

	if (m_bIsSecondHooked && m_pVMTHook)
	{
		m_pVMTHook->UninstallHook();
		m_pVMTHook.reset();
	}

	// ImGui::DestroyContext();
}

void CDirectX9Hook::Init()
{
	if (m_bIsSecondHooked || m_bIsFirstHooked)
		return;

	D3DCAPS9 caps;
	m_pOriginDevice = **reinterpret_cast<IDirect3DDevice9***>(Utils::FindPattern(XorStr("shaderapidx9.dll"), SIG_MOV_DIRECT_PTR) + 1);
	
	if (m_pOriginDevice != nullptr && m_pOriginDevice->GetDeviceCaps(&caps) == D3D_OK)
	{
		Utils::log(XorStr("CDirectX9Hook.Init -> m_pOriginDevice = 0x%X"), m_pOriginDevice);
		SetupSecondHook(m_pOriginDevice);
	}
	else
	{
		while (!m_bSuccessCreated)
		{
			if (!CreateDevice())
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		SetupFirstHook();
	}

	if(ImGui::GetCurrentContext() == NULL)
		ImGui::CreateContext();

	Utils::log(XorStr("CDirectX9Hook Initialization..."));
}

void CDirectX9Hook::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
}

void CDirectX9Hook::Rehook()
{
	if (m_bSuccessCreated)
		ReleaseDevice();

	if (m_bIsFirstHooked)
	{
		DECL_DESTORY_DETOURXS(m_pHookDrawIndexedPrimitive);
		DECL_DESTORY_DETOURXS(m_pHookEndScene);
		DECL_DESTORY_DETOURXS(m_pHookCreateQuery);
		DECL_DESTORY_DETOURXS(m_pHookReset);
		DECL_DESTORY_DETOURXS(m_pHookPresent);
	}

	if (m_bIsSecondHooked && m_pVMTHook)
	{
		m_pVMTHook->UninstallHook();
		m_pVMTHook.reset();
	}

	Init();
}

HRESULT WINAPI CDirectX9Hook::Hooked_DrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE type,
	INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_DrawIndexedPrimitive"));

	/*
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
		g_pDirextXHook->oDrawIndexedPrimitive(device, type, baseIndex, minIndex, numVertices, startIndex, primitiveCount);
		device->SetRenderState(D3DRS_ZENABLE, oldZEnable);
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}
#endif
	*/

	g_pBaseMenu->OnDrawIndexedPrimitive(device, type, baseIndex, minIndex, numVertices, startIndex, primitiveCount);

#ifdef _DEBUG
	static bool bHasFirst = true;
	if (bHasFirst)
	{
		bHasFirst = false;
		Utils::log(XorStr("Hook DrawIndexedPrimitive Success."));
	}
#endif

	return g_pDirextXHook->oDrawIndexedPrimitive(device, type, baseIndex, minIndex,
		numVertices, startIndex, primitiveCount);
}

HRESULT WINAPI CDirectX9Hook::Hooked_EndScene(IDirect3DDevice9* device)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_EndScene"));
	if(g_pDrawing->CheckDeviceStatus(device))
		Utils::log(XorStr("Device Updated with Hooked_EndScene"));

	g_pDrawing->OnBeginEndScene();

	// 在这里使用 CDrawing::Render 开头的函数进行绘制

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

	return g_pDirextXHook->oEndScene(device);
}

HRESULT WINAPI CDirectX9Hook::Hooked_CreateQuery(IDirect3DDevice9* device, D3DQUERYTYPE type,
	IDirect3DQuery9** query)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_CreateQuery"));

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

	return g_pDirextXHook->oCreateQuery(device, type, query);
}

HRESULT WINAPI CDirectX9Hook::Hooked_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
{
	if (g_pDirextXHook->CheckHookStatus(device))
		Utils::log(XorStr("Initialization with Hooked_Reset"));

	g_pDrawing->OnLostDevice();

	HRESULT hr = g_pDirextXHook->oReset(device, pp);

#ifdef _DEBUG
	if (hr == D3DERR_INVALIDCALL)
		Utils::log(XorStr("IDirect3DDevice9::Reset returns a D3DERR_INVALIDCALL!"));
#endif

	if (hr != D3DERR_INVALIDCALL)
		g_pDrawing->OnResetDevice();
	/*
	else
		g_pDirextXHook->Rehook();
	*/

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
	if (g_pDrawing->CheckDeviceStatus(device))
		Utils::log(XorStr("Device Updated with Hooked_Present"));

	g_pDrawing->OnBeginPresent();

	// 在这里使用 CDrawing::Draw 开头的函数进行绘制

	g_pDrawing->OnFinishPresent();

#ifdef _DEBUG
	static bool bHasFirst = true;
	if (bHasFirst)
	{
		bHasFirst = false;
		Utils::log(XorStr("Hook Present Success."));
	}
#endif

	return g_pDirextXHook->oPresent(device, source, dest, window, region);
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

		m_pDevice = nullptr;
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
	if (!m_bSuccessCreated || m_bIsSecondHooked || m_bIsFirstHooked || m_pDevice == nullptr)
		return false;

	m_pHookReset = std::make_unique<DetourXS>(Utils::GetVirtualFunction(m_pDevice, 16), Hooked_Reset);
	oReset = reinterpret_cast<FnReset>(m_pHookReset->GetTrampoline());

	m_pHookPresent = std::make_unique<DetourXS>(Utils::GetVirtualFunction(m_pDevice, 17), Hooked_Present);
	oPresent = reinterpret_cast<FnPresent>(m_pHookPresent->GetTrampoline());

	m_pHookEndScene = std::make_unique<DetourXS>(Utils::GetVirtualFunction(m_pDevice, 42), Hooked_EndScene);
	oEndScene = reinterpret_cast<FnEndScene>(m_pHookEndScene->GetTrampoline());

	/*
	m_pHookDrawIndexedPrimitive = std::make_unique<DetourXS>(Utils::GetVirtualFunction(m_pDevice, 82), Hooked_DrawIndexedPrimitive);
	oDrawIndexedPrimitive = reinterpret_cast<FnDrawIndexedPrimitive>(m_pHookDrawIndexedPrimitive->GetTrampoline());

	m_pHookCreateQuery = std::make_unique<DetourXS>(Utils::GetVirtualFunction(m_pDevice, 118), Hooked_CreateQuery);
	oCreateQuery = reinterpret_cast<FnCreateQuery>(m_pHookCreateQuery->GetTrampoline());
	*/

	m_bIsFirstHooked = true;
	// ReleaseDevice();

	Utils::log(XorStr("Setup DirectX Hook 1st."));
	return true;
}

bool CDirectX9Hook::SetupSecondHook(IDirect3DDevice9* device)
{
	if (m_bIsSecondHooked)
		return false;

	if (device != nullptr)
		Utils::log(XorStr("CDirectX9Hook.SetupSecondHook -> device = 0x%X"), device);

	if (m_bSuccessCreated)
		ReleaseDevice();

	if (m_bIsFirstHooked)
	{
		DECL_DESTORY_DETOURXS(m_pHookDrawIndexedPrimitive);
		DECL_DESTORY_DETOURXS(m_pHookEndScene);
		DECL_DESTORY_DETOURXS(m_pHookCreateQuery);
		DECL_DESTORY_DETOURXS(m_pHookReset);
		DECL_DESTORY_DETOURXS(m_pHookPresent);
	}

	m_pOriginDevice = device;
	m_pVMTHook = std::make_unique<CVmtHook>(device);
	oReset = reinterpret_cast<FnReset>(m_pVMTHook->HookFunction(16, Hooked_Reset));
	oPresent = reinterpret_cast<FnPresent>(m_pVMTHook->HookFunction(17, Hooked_Present));
	oEndScene = reinterpret_cast<FnEndScene>(m_pVMTHook->HookFunction(42, Hooked_EndScene));
	oDrawIndexedPrimitive = reinterpret_cast<FnDrawIndexedPrimitive>(m_pVMTHook->HookFunction(82, Hooked_DrawIndexedPrimitive));
	oCreateQuery = reinterpret_cast<FnCreateQuery>(m_pVMTHook->HookFunction(118, Hooked_CreateQuery));
	m_pVMTHook->InstallHook();

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

#define GUARD (PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE)
#define READABLE (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOPY)

IDirect3DDevice9* CDirectX9Hook::FindDevicePointer(IDirect3DDevice9* device)
{
	DWORD* pTable = *reinterpret_cast<DWORD**>(device);

	D3DCAPS9 caps;

	LPDIRECT3DDEVICE9 pDevice;

	DWORD * pTempTable,
		dwAddress,
		dwMatchCount,
		pMatchedTables[150];

	MEMORY_BASIC_INFORMATION info;

	pDevice = NULL;
	dwMatchCount = 0;

	for (int iLoop = 0; iLoop < 2; iLoop++)
	{
		dwAddress = 0;

		while (pDevice == NULL)
		{
			if (VirtualQuery((LPCVOID)dwAddress, &info, sizeof(info)) == 0)
			{
				if (GetLastError() == ERROR_INVALID_PARAMETER)
					break;

				continue;
			}
			
			if (!(info.AllocationProtect & READABLE) && info.State == MEM_COMMIT && !(info.Protect & GUARD))
			{
				// If this is the first loop, store all copies of the vtable
				if (iLoop == 0)
				{
					for (DWORD i = dwAddress; i < dwAddress + info.RegionSize - 4; i += 2)
					{
						pTempTable = (DWORD*)i;

						for (DWORD k = 0; k < 42; k++)
						{
							if (pTempTable[k] == pTable[k])
							{
								if (k == 41 && dwMatchCount < 150)
									pMatchedTables[dwMatchCount++] = i;
							}
							else
								break;
						}
					}
				}
				else
				{
					// Search for all pointers to the collected vtables
					for (DWORD i = dwAddress; i < dwAddress + info.RegionSize - 4; i += 2)
					{
						for (DWORD k = 0; k < dwMatchCount; k++)
						{
							// If this a pointer to the table
							if (pMatchedTables[k] == *(DWORD*)i)
							{
								pDevice = (LPDIRECT3DDEVICE9)i;

								// If it is a valid device and not a random pointer
								if (pDevice->GetDeviceCaps(&caps) == D3D_OK)
								{
									i = dwAddress + info.RegionSize;
									break;
								}
								else
									pDevice = NULL;
							}
						}
					}

				}
			}

			dwAddress += info.RegionSize;
		}
	}

	return pDevice;
}
