#pragma once
#include <vector>
#include <memory>
#include <d3d9.h>
#include <d3dx9.h>
#include <detourxs.h>
#include "vmt.h"

#pragma comment(lib, "d3d9")
#pragma comment(lib, "d3dx9")

typedef HRESULT(WINAPI* FnDrawIndexedPrimitive)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
typedef HRESULT(WINAPI* FnEndScene)(IDirect3DDevice9*);
typedef HRESULT(WINAPI* FnCreateQuery)(IDirect3DDevice9*, D3DQUERYTYPE, IDirect3DQuery9**);
typedef HRESULT(WINAPI* FnReset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef HRESULT(WINAPI* FnPresent)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);

class CDirectX9Hook
{
public:
	CDirectX9Hook();
	// CDirectX9Hook(IDirect3DDevice9* device);
	~CDirectX9Hook();
	
	void Init();
	void Shutdown();
	inline IDirect3DDevice9* GetDevice() { return m_pOriginDevice; };

public:
	static HRESULT WINAPI Hooked_DrawIndexedPrimitive(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
	static HRESULT WINAPI Hooked_EndScene(IDirect3DDevice9*);
	static HRESULT WINAPI Hooked_CreateQuery(IDirect3DDevice9*, D3DQUERYTYPE, IDirect3DQuery9**);
	static HRESULT WINAPI Hooked_Reset(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	static HRESULT WINAPI Hooked_Present(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);

protected:
	bool CreateDevice();
	bool ReleaseDevice();
	bool SetupFirstHook();
	bool SetupSecondHook(IDirect3DDevice9* device);

private:
	bool CheckHookStatus(IDirect3DDevice9* device);
	IDirect3DDevice9* FindDevicePointer(IDirect3DDevice9* device);

public:
	FnDrawIndexedPrimitive oDrawIndexedPrimitive;
	FnEndScene oEndScene;
	FnCreateQuery oCreateQuery;
	FnReset oReset;
	FnPresent oPresent;

private:
	std::unique_ptr<CVmtHook> m_pVMTHook;

private:
	std::unique_ptr<DetourXS> m_pHookDrawIndexedPrimitive, m_pHookEndScene,
		m_pHookCreateQuery, m_pHookReset, m_pHookPresent;

private:
	IDirect3D9* m_pD3D;
	IDirect3DDevice9* m_pDevice;
	IDirect3DDevice9* m_pOriginDevice;
	bool m_bSuccessCreated;
	bool m_bIsFirstHooked;
	bool m_bIsSecondHooked;
};

extern std::unique_ptr<CDirectX9Hook> g_pDirextXHook;


