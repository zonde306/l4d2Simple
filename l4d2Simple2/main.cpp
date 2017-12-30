#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "peb.h"
#include "dx9hook.h"
#include "utils.h"
#include "xorstr.h"
#include "../imgui/examples/directx9_example/imgui_impl_dx9.h"

// 需要在 StartCheats 里把它设置成游戏窗口
// 例如：g_hGameWindow = FindWindowA("Valve001", "Left 4 Dead 2");
HWND g_hGameWindow = nullptr;
DWORD WINAPI StartCheats(LPVOID);

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
		// HideDll(module);

		CreateThread(NULL, NULL, StartCheats, module, NULL, NULL);
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		FreeConsole();
		ImGui_ImplDX9_Shutdown();
	}

	return TRUE;
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

DWORD WINAPI StartCheats(LPVOID module)
{
	CreateDebugConsole();
	
	Utils::FindWindowByProccess();
	while ((g_hGameWindow = FindWindowA(XorStr("Valve001"), XorStr("Left 4 Dead 2"))) == NULL)
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	Utils::g_hCurrentWindow = g_hGameWindow;

	Utils::init(reinterpret_cast<HINSTANCE>(module));
	
	g_pDirextXHook = std::make_unique<CDirectX9Hook>();
	g_pDirextXHook->Init();

	return EXIT_SUCCESS;
}
