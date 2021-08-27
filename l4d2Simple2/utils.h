#pragma once

#include "xorstr.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <string_view>
#include <tuple>

#define TRIM_INVALID_CHAR XorStr(" \r\n\t")

namespace Utils
{
	extern HINSTANCE g_hInstance;
	extern std::string g_sModulePath;
	extern HWND g_hCurrentWindow;
	extern DWORD g_iSelfStart;
	extern DWORD g_iSelfEnd;

	void init(HINSTANCE inst);

	std::string BuildPath(const char* text, ...);

	bool FarProc(LPVOID address);
	bool FarProcEx(LPVOID address, DWORD start, DWORD end);

	void log(const char* text, ...);
	void logToFile(const char* text, ...);
	void logError(const char* text, ...);

	template<typename T, typename ...Arg>
	void log2(const T& value, Arg ...arg);
	void log2();

	void PrintInfo(const char* text, ...);

	std::string GetTime();
	std::string GetDate();
	std::string GetWeek();
	std::string GetDateTime();

	int GetCpuUsage();

	std::string g2u(const std::string& strGBK);
	std::string u2g(const std::string& strUTF8);
	char* utg(const char* utf8);
	char* gtu(const char* gb2312);
	std::string w2c(const std::wstring& ws);
	std::wstring c2w(const std::string& s);

	DWORD FindProccess(std::string_view proccessName);
	HMODULE GetModuleHandleSafe(const std::string& pszModuleName);
	DWORD GetModuleBase(std::string_view ModuleName, DWORD ProcessID = 0);
	DWORD GetModuleBase(std::string_view ModuleName, DWORD* ModuleSize, DWORD ProcessID = 0);

	DWORD FindPattern(const std::string& szModules, const std::string& szPattern);
	DWORD FindPattern(DWORD dwBegin, DWORD dwEnd, const std::string& szPattern);
	DWORD FindPattern(const std::string& szModules, const std::string& szPattern, std::string szMask);
	DWORD FindPattern(DWORD dwBegin, DWORD dwEnd, const std::string& szPattern, std::string szMask);

	template<typename T, typename ...Arg>
	inline T ReadMemory(Arg... offset);
	template<typename T, typename ...Arg>
	inline T WriteMemory(T value, Arg... offset);

	std::vector<std::string> StringSplit(std::string_view s, std::string_view delim = TRIM_INVALID_CHAR);
	std::string StringTrim(std::string_view s, std::string_view delim = TRIM_INVALID_CHAR);
	size_t StringFind(std::string_view s, std::string_view p, bool caseSensitive = true);
	bool StringEqual(const std::string& s, std::string p, bool caseSensitive = true);
	std::string& StringReplace(std::string& s, const std::string& p, const std::string& newText);

	int FindingString(const char* lpszSour, const char* lpszFind, int nStart = 0);
	bool MatchingString(const char* lpszSour, const char* lpszMatch, bool bMatchCase = true);
	bool MultiMatching(const char* lpszSour, const char* lpszMatch, int nMatchLogic = 0, bool bRetReversed = 0, bool bMatchCase = true);

	PVOID GetVirtualFunction(PVOID inst, DWORD index);

	template<typename Fn>
	inline Fn GetVTableFunction(PVOID inst, DWORD index);

	template<typename R, typename ...Args>
	inline R InvokeVirtualFunction(PVOID inst, DWORD index, Args... arg);

	template<typename Fn, typename ...Args>
	inline auto InvokeVTableFunction(PVOID inst, DWORD index, Args... arg);

	void FindWindowByProccess(DWORD ProcessID = 0);
	std::pair<DWORD, DWORD> GetModuleSize(const std::string& pszModuleName);
	DWORD CalcInstAddress(DWORD inst);
};

template<typename T, typename ...Arg>
inline void Utils::log2(const T & value, Arg ...arg)
{
	std::cout << value;
	return log2(std::forward<Arg>(arg)...);
}

template<typename T, typename ...Arg>
inline T Utils::ReadMemory(Arg ...offset)
{
	DWORD offsetList[] = { (DWORD)offset... };
	DWORD currentAddress = 0, finalAddress = 0, oldProtect = 0;

	int len = ARRAYSIZE(offsetList);

	if (len <= 0)
	{
		Utils::logError(XorStr("Invalid address count"));
		throw std::runtime_error(XorStr("Invalid address count"));
	}

	try
	{
		for (int i = 0; i < len; ++i)
		{
			currentAddress += offsetList[i];

			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), PAGE_EXECUTE_READWRITE, &oldProtect) == FALSE)
			{
				Utils::logError(XorStr("VirtualProtect failed"));
				throw std::runtime_error(XorStr("VirtualProtect failed"));
			}

			if (i < len - 1)
			{
				finalAddress = *(DWORD*)currentAddress;
			}
			else
				finalAddress = currentAddress;

#ifdef DEBUG
			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, NULL) == FALSE)
				printf("Error: Failed to restore protection of address 0x%X\n", currentAddress);
#else
			VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, &oldProtect);
#endif
			currentAddress = finalAddress;
		}

		if (finalAddress != NULL)
			return *(T*)finalAddress;
	}
	catch (...)
	{
		Utils::logError(XorStr("ReadMemory failed: 0x%X"), currentAddress);
		throw std::runtime_error(XorStr("ReadMemory failed"));
	}

	return T();
}

template<typename T, typename ...Arg>
inline T Utils::WriteMemory(T value, Arg ...offset)
{
	DWORD offsetList[] = { (DWORD)offset... };
	DWORD currentAddress = 0, finalAddress = 0, oldProtect = 0;

	int len = ARRAYSIZE(offsetList);

	if (len <= 0)
	{
		Utils::logError(XorStr("Invalid address count"));
		throw std::runtime_error(XorStr("Invalid address count"));
	}

	try
	{
		for (int i = 0; i < len; ++i)
		{
			currentAddress += offsetList[i];

			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), PAGE_EXECUTE_READWRITE, &oldProtect) == FALSE)
			{
				Utils::logError(XorStr("VirtualProtect failed"));
				throw std::runtime_error(XorStr("VirtualProtect failed"));
			}

			if (i < len - 1)
			{
				finalAddress = *(DWORD*)currentAddress;
			}
			else
				finalAddress = currentAddress;

#ifdef DEBUG
			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, NULL) == FALSE)
				printf("Error: Failed to restore protection of address 0x%X\n", currentAddress);
#else
			VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, &oldProtect);
#endif
			currentAddress = finalAddress;
		}

		if (finalAddress != NULL)
			return (*(T*)finalAddress = value);
	}
	catch (...)
	{
		Utils::logError(XorStr("ReadMemory failed: 0x%X"), currentAddress);
		throw std::runtime_error(XorStr("ReadMemory failed"));
	}

	return T();
}

template<typename Fn>
inline Fn Utils::GetVTableFunction(PVOID inst, DWORD index)
{
	if (inst == nullptr)
		return nullptr;

	PDWORD table = *(reinterpret_cast<PDWORD*>(inst));

	if (table == nullptr)
		return nullptr;

	return reinterpret_cast<Fn>(table[index]);
}

template<typename R, typename ...Args>
inline R Utils::InvokeVirtualFunction(PVOID inst, DWORD index, Args ...arg)
{
	using Fn = R(__thiscall*)(PVOID, Args...);
	return std::forward<R>(reinterpret_cast<Fn>(GetVirtualFunction(inst, index))(inst, std::forward<Args>(arg)...));
}

template<typename Fn, typename ...Args>
inline auto Utils::InvokeVTableFunction(PVOID inst, DWORD index, Args ...arg)
{
	return std::forward<auto>(GetVTableFunction<Fn>(inst, index)(inst, std::forward<Args>(arg)...));
}