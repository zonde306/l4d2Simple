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
	IDirect3DDevice9* GetDevice();

protected:
	template<typename Fn>
	bool HookFunction(Fn function);

	template<typename Fn>
	bool UnhookFunction(Fn function);

	template<typename Fn, typename ...Arg>
	HRESULT InvokeOriginal(Fn, Arg ...arg);

protected:
	bool CreateDevice();
	bool ReleaseDevice();
	bool SetupFirstHook();
	bool SetupSecondHook(IDirect3DDevice9* device);

	static HRESULT WINAPI Hooked_DrawIndexedPrimitive(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
	static HRESULT WINAPI Hooked_EndScene(IDirect3DDevice9*);
	static HRESULT WINAPI Hooked_CreateQuery(IDirect3DDevice9*, D3DQUERYTYPE, IDirect3DQuery9**);
	static HRESULT WINAPI Hooked_Reset(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	static HRESULT WINAPI Hooked_Present(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);

private:
	bool CheckHookStatus(IDirect3DDevice9* device);

	std::vector<FnDrawIndexedPrimitive>& GetHookList(FnDrawIndexedPrimitive);
	std::vector<FnEndScene>& GetHookList(FnEndScene);
	std::vector<FnCreateQuery>& GetHookList(FnCreateQuery);
	std::vector<FnReset>& GetHookList(FnReset);
	std::vector<FnPresent>& GetHookList(FnPresent);

	HRESULT CallOriginalFunction(FnDrawIndexedPrimitive, IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
	HRESULT CallOriginalFunction(FnEndScene, IDirect3DDevice9*);
	HRESULT CallOriginalFunction(FnCreateQuery, IDirect3DDevice9*, D3DQUERYTYPE, IDirect3DQuery9**);
	HRESULT CallOriginalFunction(FnReset, IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	HRESULT CallOriginalFunction(FnPresent, IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);

	template<typename Fn>
	bool _FunctionHook(std::vector<Fn>& list, Fn function);

	template<typename Fn>
	bool _FunctionUnhook(std::vector<Fn>& list, Fn function);

public:
	bool AddHook_DrawIndexedPrimitive(FnDrawIndexedPrimitive function);
	bool AddHook_EndScene(FnEndScene function);
	bool AddHook_CreateQuery(FnCreateQuery function);
	bool AddHook_Reset(FnReset function);
	bool AddHook_Present(FnPresent function);

	bool RemoveHook_DrawIndexedPrimitive(FnDrawIndexedPrimitive function);
	bool RemoveHook_EndScene(FnEndScene function);
	bool RemoveHook_CreateQuery(FnCreateQuery function);
	bool RemoveHook_Reset(FnReset function);
	bool RemoveHook_Present(FnPresent function);

	HRESULT CallOriginal_DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
	HRESULT CallOriginal_EndScene();
	HRESULT CallOriginal_CreateQuery(D3DQUERYTYPE, IDirect3DQuery9**);
	HRESULT CallOriginal_Reset(D3DPRESENT_PARAMETERS*);
	HRESULT CallOriginal_Present(const RECT*, const RECT*, HWND, const RGNDATA*);

	HRESULT CallOriginal_DrawIndexedPrimitive(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
	HRESULT CallOriginal_EndScene(IDirect3DDevice9*);
	HRESULT CallOriginal_CreateQuery(IDirect3DDevice9*, D3DQUERYTYPE, IDirect3DQuery9**);
	HRESULT CallOriginal_Reset(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	HRESULT CallOriginal_Present(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	
// private:
	FnDrawIndexedPrimitive m_pfnDrawIndexedPrimitive;
	FnEndScene m_pfnEndScene;
	FnCreateQuery m_pfnCreateQuery;
	FnReset m_pfnReset;
	FnPresent m_pfnPresent;

	std::vector<FnDrawIndexedPrimitive> m_vfnDrawIndexedPrimitive;
	std::vector<FnEndScene> m_vfnEndScene;
	std::vector<FnCreateQuery> m_vfnCreateQuery;
	std::vector<FnReset> m_vfnReset;
	std::vector<FnPresent> m_vfnPresent;

private:
	std::vector<DetourXS*> m_vpDetourList;
	CVmtHook* m_pVmtHook;
	PDWORD m_pVMT;

private:
	IDirect3D9* m_pD3D;
	IDirect3DDevice9* m_pDevice;
	IDirect3DDevice9* m_pOriginDevice;
	bool m_bSuccessCreated;
	bool m_bIsFirstHooked;
	bool m_bIsSecondHooked;
};

template<typename Fn>
class _DeclTypeCall
{
	friend CDirectX9Hook;

	// 直接调用原函数
	template<typename ...Arg>
	static HRESULT Invoke(Arg... arg);

	// 获取原函数或蹦床
	static Fn GetFunction();

	// 获取 Hook 表
	static std::vector<Fn>* GetHookList();
};

extern std::unique_ptr<CDirectX9Hook> g_pDirextXHook;
