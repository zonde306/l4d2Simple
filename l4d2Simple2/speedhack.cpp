#include "speedhack.h"
#include "xorstr.h"
#include <imgui.h>

#pragma comment(lib, "Winmm")

std::unique_ptr<CSpeedModifier> g_pSpeedModifier;
#define GET_ORIGINAL_FUNC(_fn)		reinterpret_cast<Fn##_fn>(g_pSpeedModifier->m_p##_fn->trampoline);

// http://blog.csdn.net/Anton8801/article/details/78726786
static DWORD WINAPI Hooked_GetTickCount()
{
	DWORD nowTick = reinterpret_cast<FnGetTickCount>(g_pSpeedModifier->m_pGetTickCount->trampoline)();
	if (g_pSpeedModifier->m_fSpeed <= 0.0f || g_pSpeedModifier->m_fSpeed == 1.0f)
		return nowTick;

	DWORD result = 0;
	static DWORD iLastFakeTick = 0;
	static DWORD iLastRealTick = 0;

	if (iLastRealTick == 0)
	{
		result = iLastRealTick = iLastFakeTick = nowTick;
	}
	else
	{
		result = iLastFakeTick + static_cast<DWORD>(g_pSpeedModifier->m_fSpeed * (nowTick - iLastRealTick));
		iLastFakeTick = result;
		iLastRealTick = nowTick;
	}

	return result;
}

static ULONGLONG WINAPI Hooked_GetTickCount64()
{
	ULONGLONG nowTick = reinterpret_cast<FnGetTickCount64>(g_pSpeedModifier->m_pGetTickCount64->trampoline)();
	if (g_pSpeedModifier->m_fSpeed <= 0.0f || g_pSpeedModifier->m_fSpeed == 1.0f)
		return nowTick;

	ULONGLONG result = 0;
	static ULONGLONG iLastFakeTick = 0;
	static ULONGLONG iLastRealTick = 0;

	if (iLastRealTick == 0)
	{
		result = iLastRealTick = iLastFakeTick = nowTick;
	}
	else
	{
		result = iLastFakeTick + static_cast<ULONGLONG>(g_pSpeedModifier->m_fSpeed * (nowTick - iLastRealTick));
		iLastFakeTick = result;
		iLastRealTick = nowTick;
	}

	return result;
}

static DWORD WINAPI Hooked_TimeGetTime()
{
	DWORD nowTick = reinterpret_cast<FnTimeGetTime>(g_pSpeedModifier->m_pTimeGetTime->trampoline)();
	if (g_pSpeedModifier->m_fSpeed <= 0.0f || g_pSpeedModifier->m_fSpeed == 1.0f)
		return nowTick;

	DWORD result = 0;
	static DWORD iLastFakeTick = 0;
	static DWORD iLastRealTick = 0;

	if (iLastRealTick == 0)
	{
		result = iLastRealTick = iLastFakeTick = nowTick;
	}
	else
	{
		result = iLastFakeTick + static_cast<DWORD>(g_pSpeedModifier->m_fSpeed * (nowTick - iLastRealTick));
		iLastFakeTick = result;
		iLastRealTick = nowTick;
	}

	return result;
}

static BOOL WINAPI Hooked_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	if (!reinterpret_cast<FnQueryPerformanceCounter>(g_pSpeedModifier->m_pQueryPerformanceCounter->trampoline)(lpPerformanceCount))
		return FALSE;

	if (g_pSpeedModifier->m_fSpeed <= 0.0f || g_pSpeedModifier->m_fSpeed == 1.0f)
		return TRUE;

	static LARGE_INTEGER iLastFakeTick = { 0 };
	static LARGE_INTEGER iLastRealTick = { 0 };
	LARGE_INTEGER nowTick = *lpPerformanceCount;

	if (iLastRealTick.QuadPart == 0)
	{
		iLastFakeTick = iLastRealTick = nowTick;
	}
	else
	{
		lpPerformanceCount->QuadPart = iLastFakeTick.QuadPart + static_cast<LONGLONG>(g_pSpeedModifier->m_fSpeed * (nowTick.QuadPart - iLastRealTick.QuadPart));
		iLastFakeTick = *lpPerformanceCount;
		iLastRealTick = nowTick;
	}

	return TRUE;
}

CSpeedModifier::~CSpeedModifier()
{
	Shutdown();
}

void CSpeedModifier::Init()
{
	m_fSpeed = 1.0f;
	m_pGetTickCount = SpliceHookFunction(GetTickCount, Hooked_GetTickCount);
	m_pGetTickCount64 = SpliceHookFunction(GetTickCount64, Hooked_GetTickCount64);
	m_pTimeGetTime = SpliceHookFunction(timeGetTime, Hooked_TimeGetTime);
	m_pQueryPerformanceCounter = SpliceHookFunction(QueryPerformanceCounter, Hooked_QueryPerformanceCounter);
}

#define DECL_DESTORY_SPLICE(_name)		if(_name != nullptr)\
{\
	SpliceUnHookFunction(_name);\
	_name = nullptr;\
}

void CSpeedModifier::Shutdown()
{
	DECL_DESTORY_SPLICE(m_pGetTickCount);
	DECL_DESTORY_SPLICE(m_pGetTickCount64);
	DECL_DESTORY_SPLICE(m_pTimeGetTime);
	DECL_DESTORY_SPLICE(m_pQueryPerformanceCounter);
}
