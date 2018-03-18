#pragma once
#include <memory>
#include "../detours/splice.h"

using FnGetTickCount = DWORD(WINAPI*)();
using FnGetTickCount64 = ULONGLONG(WINAPI*)();
using FnTimeGetTime = DWORD(*)();
using FnQueryPerformanceCounter = BOOL(WINAPI*)(LARGE_INTEGER*);

class CSpeedModifier
{
public:
	~CSpeedModifier();
	void Init();
	void Shutdown();

public:
	inline float GetSpeed() { return m_fSpeed; };
	inline void SetSpeed(float speed) { m_fSpeed = speed; };

public:
	float m_fSpeed = 1.0f;
	PSPLICE_ENTRY m_pGetTickCount = nullptr;
	PSPLICE_ENTRY m_pGetTickCount64 = nullptr;
	PSPLICE_ENTRY m_pTimeGetTime = nullptr;
	PSPLICE_ENTRY m_pQueryPerformanceCounter = nullptr;

};

extern std::unique_ptr<CSpeedModifier> g_pSpeedModifier;
