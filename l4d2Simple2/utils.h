#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace Utils
{
	// 一些可能会用到的变量
	extern HINSTANCE g_hInstance;
	extern std::string g_sModulePath;
	extern HWND g_hCurrentWindow;
	
	// 初始化
	void init(HINSTANCE inst);

	// 记录日志
	void log(const char* text, ...);
	void logToFile(const char* text, ...);
	void logError(const char* text, ...);

	template<typename T, typename ...Arg>
	void log2(const T& value, Arg ...arg);
	void log2();

	// 在左上角提示信息
	void printInfo(const char* text, ...);

	// 获取系统时间
	std::string getTime();
	std::string getDate();
	std::string getWeek();
	std::string getDateTime();

	// 获取系统信息
	int getCpuUsage();

	// 字符串格式转换
	std::string g2u(const std::string& strGBK);
	std::string u2g(const std::string& strUTF8);
	char* utg(const char* utf8);
	char* gtu(const char* gb2312);
	std::string w2c(const std::wstring& ws);
	std::wstring c2w(const std::string& s);

	// 内存搜索相关
	DWORD FindProccess(const std::string& proccessName);
	HMODULE GetModuleHandleSafe(const std::string& pszModuleName);
	DWORD GetModuleBase(const std::string& ModuleName, DWORD ProcessID = 0);
	DWORD GetModuleBase(const std::string& ModuleName, DWORD* ModuleSize, DWORD ProcessID = 0);
	template<typename T, typename ...Arg> T ReadMemory(Arg... offset);
	template<typename T, typename ...Arg> T WriteMemory(T value, Arg... offset);

	// 字符串处理
	std::vector<std::string> Split(const std::string& s, const std::string& delim);
	std::string Trim(const std::string& s, const std::string& delim);

	// 虚函数相关
	PVOID GetVirtualFunction(PVOID inst, DWORD index);
	template<typename Fn> Fn GetVTableFunction(PVOID inst, DWORD index);

	// 进程相关
	void FindWindowByProccess(DWORD ProcessID = 0);
};

template<typename T, typename ...Arg>
void Utils::log2(const T & value, Arg ...arg)
{
	std::cout << value;
	return log2(std::forward<Arg>(arg)...);
}

template<typename T, typename ...Arg>
T Utils::ReadMemory(Arg ...offset)
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
T Utils::WriteMemory(T value, Arg ...offset)
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

