#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "peb.h"
#include "dx9hook.h"
#include "utils.h"
#include "xorstr.h"
#include "menu.h"
#include "speedhack.h"
#include "../hl2sdk/interfaces.h"
#include "../hl2sdk/hook.h"

// #include "../imgui/examples/directx9_example/imgui_impl_dx9.h"

// 需要在 StartCheats 里把它设置成游戏窗口
// 例如：g_hGameWindow = FindWindowA("Valve001", "Left 4 Dead 2");
HWND g_hGameWindow = nullptr;

DWORD WINAPI StartCheats(LPVOID);
void CreateDebugConsole();
HWND CheckTopWindow();

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);

		/*
#ifndef _DEBUG
		HideDll(module);
#endif
		*/

		CreateThread(NULL, NULL, StartCheats, module, NULL, NULL);

		/*
#ifndef _DEBUG
		HideThread(g_hStartThread);
#endif
		*/
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
#ifdef _DEBUG
		FreeConsole();
#endif

		g_pClientHook->Shutdown();
		g_pDirextXHook->Shutdown();
		g_pSpeedModifier->Shutdown();
	}

	return TRUE;
}

DWORD WINAPI StartCheats(LPVOID module)
{
#ifdef _DEBUG
	CreateDebugConsole();
#endif
	
	// Utils::FindWindowByProccess();
	while ((g_hGameWindow = FindWindowA(XorStr("Valve001"), nullptr)) == NULL)
	{
		Utils::log(XorStr("Please switch to the target window."));
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		if ((g_hGameWindow = CheckTopWindow()) != NULL)
			break;
	}

	Utils::g_hCurrentWindow = g_hGameWindow;

	Utils::init(reinterpret_cast<HINSTANCE>(module));
	
	g_pSpeedModifier = std::make_unique<CSpeedModifier>();
	g_pSpeedModifier->Init();

	g_pInterface = std::make_unique<CClientInterface>();
	g_pInterface->Init();

	g_pClientHook = std::make_unique<CClientHook>();
	g_pClientHook->Init();

	g_pDirextXHook = std::make_unique<CDirectX9Hook>();
	g_pDirextXHook->Init();

	g_pBaseMenu = std::make_unique<CBaseMenu>();
	g_pBaseMenu->Init();

	return EXIT_SUCCESS;
}

void CreateDebugConsole()
{
	AllocConsole();

	HWND hwnd = GetConsoleWindow();
	HMENU hMenu = GetSystemMenu(hwnd, FALSE);
	if (hMenu) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);

	freopen(XorStr("CONIN$"), XorStr("r"), stdin);
	freopen(XorStr("CONOUT$"), XorStr("w"), stdout);
}

HWND CheckTopWindow()
{
	static DWORD curPid = GetCurrentProcessId();
	HWND topWindow = GetForegroundWindow();
	if (topWindow == NULL)
		return NULL;

	DWORD pid = 0;
	GetWindowThreadProcessId(topWindow, &pid);
	if (pid != curPid)
		return NULL;

	char classname[64];
	RealGetWindowClassA(topWindow, classname, 64);
	if (!_stricmp(classname, XorStr("ConsoleWindowClass")))
		return NULL;

	char title[255];
	GetWindowTextA(topWindow, title, 255);

	std::cout << XorStr("Found: ") << title << ' ' << '(' << classname << ')' << std::endl;
	return topWindow;
}
