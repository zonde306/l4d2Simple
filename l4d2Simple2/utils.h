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
	LPVOID GetVirtualFunction(LPVOID inst, DWORD index);
	template<typename Fn> Fn GetVTableFunction(LPVOID inst, DWORD index);

	// 进程相关
	void FindWindowByProccess(DWORD ProcessID = 0);
};
