#pragma once
#include <Windows.h>
#include <functional>
#include <vector>

class CVmtHook
{
public:
	CVmtHook();
	~CVmtHook();
	CVmtHook(LPVOID pointer);
	void Init(LPVOID pointer);

	LPVOID HookFunction(DWORD index, LPVOID function, bool update = false);
	LPVOID UnhookFunction(DWORD index, bool update = false);
	LPVOID GetOriginalFunction(DWORD index);

	bool InstallHook();
	bool UninstallHook();

	DWORD GetCount();
	static bool CanReadPointer(LPVOID pointer);

	template<typename Fn>
	Fn* HookFunctionEx(DWORD index, Fn* function, bool update = false);

	template<typename R, typename ...Arg>
	R Invoke(DWORD index, Arg ...arg);

	LPVOID GetOriginalTable();
	LPVOID GetHookedTable();

protected:
	LPVOID CheckHookFunction(DWORD index, LPVOID function, bool update, std::true_type);

protected:
	PDWORD m_pOriginTable, m_pCopyTable, m_pInstance;
	DWORD m_iHeightIndex;
	bool m_bHasHooked;
};
