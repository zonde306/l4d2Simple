#include "vmt.h"

CVmtHook::CVmtHook() : m_pOriginTable(nullptr), m_pCopyTable(nullptr), m_iHeightIndex(0), m_bHasHooked(false)
{
}

CVmtHook::~CVmtHook()
{
	if (m_bHasHooked)
		UninstallHook();
	
	delete[] m_pCopyTable;
}

CVmtHook::CVmtHook(LPVOID pointer) : CVmtHook::CVmtHook()
{
	Init(pointer);
}

void CVmtHook::Init(LPVOID pointer)
{
	m_pOriginTable = *(reinterpret_cast<PDWORD*>(pointer));
	m_iHeightIndex = GetCount();

	m_pCopyTable = new DWORD[m_iHeightIndex + 1];
	m_pCopyTable[m_iHeightIndex] = 0;
	memcpy_s(m_pCopyTable, sizeof(LPVOID) * m_iHeightIndex, m_pOriginTable, sizeof(LPVOID) * m_iHeightIndex);
	m_pInstance = reinterpret_cast<PDWORD>(pointer);
}

LPVOID CVmtHook::HookFunction(DWORD index, LPVOID function, bool update)
{
	if (index >= m_iHeightIndex)
		return nullptr;

	m_pCopyTable[index] = reinterpret_cast<DWORD>(function);
	if (update)
		InstallHook();

	return reinterpret_cast<LPVOID>(m_pOriginTable[index]);
}

LPVOID CVmtHook::UnhookFunction(DWORD index, bool update)
{
	if (index >= m_iHeightIndex)
		return nullptr;

	LPVOID hookedFunction = reinterpret_cast<LPVOID>(m_pCopyTable[index]);
	m_pCopyTable[index] = m_pOriginTable[index];

	if (update)
		InstallHook();

	return hookedFunction;
}

LPVOID CVmtHook::GetOriginalFunction(DWORD index)
{
	if (index >= m_iHeightIndex)
		return nullptr;

	return reinterpret_cast<LPVOID>(m_pOriginTable[index]);
}

bool CVmtHook::InstallHook()
{
	try
	{
		*m_pInstance = reinterpret_cast<DWORD>(m_pCopyTable);
		m_bHasHooked = true;
		return true;
	}
	catch(...)
	{
		
	}

	return false;
}

bool CVmtHook::UninstallHook()
{
	try
	{
		*m_pInstance = reinterpret_cast<DWORD>(m_pOriginTable);
		m_bHasHooked = false;
		return true;
	}
	catch (...)
	{
		
	}

	return false;
}

DWORD CVmtHook::GetCount()
{
	if (m_pOriginTable == nullptr)
		return 0;
	
	DWORD index = 0;
	PDWORD* table = reinterpret_cast<PDWORD*>(m_pOriginTable);
	try
	{
		for (PDWORD func = nullptr; (func = table[index]) != nullptr; ++index)
		{
			if (!CanReadPointer(func))
				break;
		}
	}
	catch (...)
	{

	}

	return index;
}

bool CVmtHook::CanReadPointer(LPVOID pointer)
{
	if (pointer == nullptr)
		return false;
	
	MEMORY_BASIC_INFORMATION mbi;
	if (!VirtualQuery(pointer, &mbi, sizeof(mbi)))
		return false;

	if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
		return false;

	return (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_READONLY |
		PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_WRITECOPY));
}

LPVOID CVmtHook::CheckHookFunction(DWORD index, LPVOID function, bool update, std::true_type)
{
	return HookFunction(index, function, update);
}

template<typename Fn>
Fn* CVmtHook::HookFunctionEx(DWORD index, Fn* function, bool update)
{
	return reinterpret_cast<Fn>(CheckHookFunction(index, function, update, std::is_function<Fn>::value));
}

template<typename R, typename ...Arg>
R CVmtHook::Invoke(DWORD index, Arg ...arg)
{
	if (index >= m_iHeightIndex)
		return nullptr;
	
	typedef R(*__thiscall Fn)(Arg...);
	return reinterpret_cast<Fn>(m_pOriginTable[index])(std::forward<Arg>(arg)...);
}
