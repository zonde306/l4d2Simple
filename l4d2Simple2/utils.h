#pragma once
#include "xorstr.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <string_view>
#include <tuple>

#define TRIM_INVALID_CHAR	XorStr(" \r\n\t")

namespace Utils
{
	// 一些可能会用到的变量
	extern HINSTANCE g_hInstance;
	extern std::string g_sModulePath;
	extern HWND g_hCurrentWindow;
	extern DWORD g_iSelfStart;
	extern DWORD g_iSelfEnd;
	
	// 初始化
	void init(HINSTANCE inst);

	// 获取当前 dll 目录
	std::string BuildPath(const char* text, ...);

	// 检查指针是否超出了当前 dll 的范围
	bool FarProc(LPVOID address);

	// 检查指针是否超出了范围
	bool FarProcEx(LPVOID address, DWORD start, DWORD end);

	// 记录日志
	void log(const char* text, ...);
	void logToFile(const char* text, ...);
	void logError(const char* text, ...);

	template<typename T, typename ...Arg>
	void log2(const T& value, Arg ...arg);
	void log2();

	// 在左上角提示信息
	void PrintInfo(const char* text, ...);

	// 获取系统时间
	std::string GetTime();
	std::string GetDate();
	std::string GetWeek();
	std::string GetDateTime();

	// 获取系统信息
	int GetCpuUsage();

	// 字符串格式转换
	std::string g2u(const std::string& strGBK);
	std::string u2g(const std::string& strUTF8);
	char* utg(const char* utf8);
	char* gtu(const char* gb2312);
	std::string w2c(const std::wstring& ws);
	std::wstring c2w(const std::string& s);

	// 内存搜索相关
	DWORD FindProccess(std::string_view proccessName);
	HMODULE GetModuleHandleSafe(const std::string& pszModuleName);
	DWORD GetModuleBase(std::string_view ModuleName, DWORD ProcessID = 0);
	DWORD GetModuleBase(std::string_view ModuleName, DWORD* ModuleSize, DWORD ProcessID = 0);

	// IDA Style
	DWORD FindPattern(const std::string& szModules, const std::string& szPattern);
	DWORD FindPattern(DWORD dwBegin, DWORD dwEnd, const std::string& szPattern);

	// Code Style
	DWORD FindPattern(const std::string & szModules, const std::string & szPattern, std::string szMask);
	DWORD FindPattern(DWORD dwBegin, DWORD dwEnd, const std::string & szPattern, std::string szMask);

	template<typename T, typename ...Arg>
	inline T ReadMemory(Arg... offset);
	template<typename T, typename ...Arg>
	inline T WriteMemory(T value, Arg... offset);

	// 字符串处理
	std::vector<std::string> StringSplit(std::string_view s, std::string_view delim = TRIM_INVALID_CHAR);
	std::string StringTrim(const std::string& s, const std::string& delim = TRIM_INVALID_CHAR);
	size_t StringFind(const std::string& s, const std::string& p, bool caseSensitive = true);
	bool StringEqual(const std::string& s, std::string p, bool caseSensitive = true);
	std::string& StringReplace(std::string& s, const std::string& p, const std::string& newText);

	//查找字符串
	int FindingString(const char* lpszSour, const char* lpszFind, int nStart = 0);
	//带通配符的字符串匹配
	bool MatchingString(const char* lpszSour, const char* lpszMatch, bool bMatchCase = true);
	//多重匹配
	bool MultiMatching(const char* lpszSour, const char* lpszMatch, int nMatchLogic = 0, bool bRetReversed = 0, bool bMatchCase = true);

	// 虚函数相关
	PVOID GetVirtualFunction(PVOID inst, DWORD index);

	template<typename Fn>
	inline Fn GetVTableFunction(PVOID inst, DWORD index);

	template<typename R, typename ...Args>
	inline R InvokeVirtualFunction(PVOID inst, DWORD index, Args... arg);

	template<typename Fn, typename ...Args>
	inline auto InvokeVTableFunction(PVOID inst, DWORD index, Args... arg);

	// 进程相关
	void FindWindowByProccess(DWORD ProcessID = 0);

	// 获取模块地址
	std::pair<DWORD, DWORD> GetModuleSize(const std::string& pszModuleName);
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
		Utils::logError(XorStr("Invalid Address Count."));
		throw std::runtime_error(XorStr("Non Address"));
	}

	try
	{
		for (int i = 0; i < len; ++i)
		{
			currentAddress += offsetList[i];
			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), PAGE_EXECUTE_READWRITE, &oldProtect) == FALSE)
			{
				Utils::logError(XorStr("VirtualProtect Failed."));
				throw std::runtime_error(XorStr("Change VirtualProtect Failed"));
			}

			if (i < len - 1)
			{
				finalAddress = *(DWORD*)currentAddress;
				// memcpy_s(&finalAddress, sizeof(finalAddress), (void*)current, sizeof(current));
			}
			else
				finalAddress = currentAddress;

#ifdef DEBUG
			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, NULL) == FALSE)
				printf("错误：恢复地址 0x%X 的保护失败。\n", currentAddress);
#else
			VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, &oldProtect);
#endif

			// 将当前地址设置为最后的地址
			currentAddress = finalAddress;
		}

		if (finalAddress != NULL)
			return *(T*)finalAddress;
	}
	catch (...)
	{
		Utils::logError(XorStr("ReadMemory Failed: 0x%X."), currentAddress);
		throw std::runtime_error(XorStr("ReadMemory Failed"));
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
		Utils::logError(XorStr("Invalid Address Count."));
		throw std::runtime_error(XorStr("Non Address"));
	}

	try
	{
		for (int i = 0; i < len; ++i)
		{
			/*
			if (i == len - 1 || offsetList[i] == NULL)
			{
			if (finalAddress != NULL)
			return (*(T*)finalAddress = value);

			printf("找不到任何东西。\n");
			return T();
			}
			*/

			currentAddress += offsetList[i];
			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), PAGE_EXECUTE_READWRITE, &oldProtect) == FALSE)
			{
				Utils::logError(XorStr("VirtualProtect Failed."));
				throw std::runtime_error(XorStr("Change VirtualProtect Failed"));
			}

			if (i < len - 1)
			{
				finalAddress = *(DWORD*)currentAddress;
				// memcpy_s(&finalAddress, sizeof(finalAddress), (void*)current, sizeof(current));
			}
			else
				finalAddress = currentAddress;

#ifdef DEBUG
			if (VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, NULL) == FALSE)
				printf("错误：恢复地址 0x%X 的保护失败。\n", currentAddress);
#else
			VirtualProtect((void*)currentAddress, sizeof(finalAddress), oldProtect, &oldProtect);
#endif

			// 将当前地址设置为最后的地址
			currentAddress = finalAddress;
		}

		if (finalAddress != NULL)
			return (*(T*)finalAddress = value);
	}
	catch (...)
	{
		Utils::logError(XorStr("ReadMemory Failed: 0x%X."), currentAddress);
		throw std::runtime_error(XorStr("ReadMemory Failed"));
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
