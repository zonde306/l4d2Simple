#include "utils.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <thread>
#include <TlHelp32.h>
#include "xorstr.h"
#include "drawing.h"

HINSTANCE Utils::g_hInstance = NULL;
std::string Utils::g_sModulePath = "";

void Utils::init(HINSTANCE inst)
{
	g_hInstance = inst;

	char buffer[MAX_PATH];
	GetModuleFileNameA(g_hInstance, buffer, MAX_PATH);
	std::string tmp = buffer;
	g_sModulePath = tmp.substr(0, tmp.find('\\'));
}

void Utils::log(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	std::cout << text << std::endl;
	OutputDebugStringA(text);
}

void Utils::logToFile(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	std::cout << '[' << getDateTime() << ']' << text << std::endl;
	OutputDebugStringA(text);
}

void Utils::logError(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	std::cout << '[' << getDateTime() << ']' << text << std::endl;
	OutputDebugStringA(text);
}

void Utils::log2()
{
	std::cout << std::endl;
}

void Utils::printInfo(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	g_pDrawing->PrintInfo(CDrawing::WHITE, text);
}

std::string Utils::getTime()
{
	tm timeInfo;
	time_t t = time(nullptr);
	localtime_s(&timeInfo, &t);

	std::stringstream ss;
	ss << timeInfo.tm_hour << ':' << timeInfo.tm_min << ':' << timeInfo.tm_sec;
	return ss.str();
}

std::string Utils::getDate()
{
	tm timeInfo;
	time_t t = time(nullptr);
	localtime_s(&timeInfo, &t);

	std::stringstream ss;
	ss << timeInfo.tm_year + 1900 << '/' << timeInfo.tm_mon + 1 << '/' << timeInfo.tm_mday;
	return ss.str();
}

std::string Utils::getWeek()
{
	tm timeInfo;
	time_t t = time(nullptr);
	localtime_s(&timeInfo, &t);

	int week = (timeInfo.tm_mday + 2 * (timeInfo.tm_mon + 1) + 3 *
		((timeInfo.tm_mon + 1) + 1) / 5 + (timeInfo.tm_year + 1900) +
		(timeInfo.tm_year + 1900) / 4 - (timeInfo.tm_year + 1900) / 100 +
		(timeInfo.tm_year + 1900) / 400) % 7;

	switch (week)
	{
	case 0:
		return XorStr("Monday");
	case 1:
		return XorStr("Tuesday");
	case 2:
		return XorStr("Wednesday");
	case 3:
		return XorStr("Thursday");
	case 4:
		return XorStr("Friday");
	case 5:
		return XorStr("Saturday");
	case 6:
		return XorStr("Sunday");
	}

	return XorStr("Unknown");
}

std::string Utils::getDateTime()
{
	tm timeInfo;
	time_t t = time(nullptr);
	localtime_s(&timeInfo, &t);

	std::stringstream ss;
	ss << timeInfo.tm_year + 1900 << '/' << timeInfo.tm_mon + 1 << '/' << timeInfo.tm_mday << ' ' <<
		timeInfo.tm_hour << ':' << timeInfo.tm_min << ':' << timeInfo.tm_sec;
	return ss.str();
}

int Utils::getCpuUsage()
{
	//cpu数量
	static int processor_count_ = -1;

	//上一次的时间
	static int64_t last_time_ = 0;
	static int64_t last_system_time_ = 0;

	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;
	int64_t system_time;
	int64_t time;
	int64_t system_time_delta;
	int64_t time_delta;

	int cpu = -1;
	if (processor_count_ == -1)
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		processor_count_ = (int)info.dwNumberOfProcessors;
	}

	GetSystemTimeAsFileTime(&now);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		return -1;
	}

	static auto file_time_2_utc = [](const FILETIME* ftime) -> uint64_t
	{
		LARGE_INTEGER li;
		li.LowPart = ftime->dwLowDateTime;
		li.HighPart = ftime->dwHighDateTime;
		return li.QuadPart;
	};

	system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / processor_count_;
	time = file_time_2_utc(&now);

	if ((last_system_time_ == 0) || (last_time_ == 0))
	{
		last_system_time_ = system_time;
		last_time_ = time;
		return -1;
	}

	system_time_delta = system_time - last_system_time_;
	time_delta = time - last_time_;
	if (time_delta == 0)
		return -1;

	cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
	last_system_time_ = system_time;
	last_time_ = time;

	return cpu;
}

std::string Utils::g2u(const std::string & strGBK)
{
	std::string strOutUTF8 = "";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	strOutUTF8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutUTF8;
}

std::string Utils::u2g(const std::string & strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	WCHAR * wszGBK = new WCHAR[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	//strUTF8 = szGBK;  
	std::string strTemp(szGBK);
	delete[]szGBK;
	delete[]wszGBK;
	return strTemp;
}

char * Utils::utg(const char * utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

char * Utils::gtu(const char * gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

std::string Utils::w2c(const std::wstring & ws)
{
	size_t convertedChars = 0;
	std::string curLocale = setlocale(LC_ALL, NULL); //curLocale="C"
	setlocale(LC_ALL, "chs");
	const wchar_t* wcs = ws.c_str();
	size_t dByteNum = sizeof(wchar_t)*ws.size() + 1;
	// cout << "ws.size():" << ws.size() << endl;            //5

	char* dest = new char[dByteNum];
	wcstombs_s(&convertedChars, dest, dByteNum, wcs, _TRUNCATE);
	// cout << "convertedChars:" << convertedChars << endl; //8
	std::string result = dest;
	delete[] dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

std::wstring Utils::c2w(const std::string & s)
{
	size_t convertedChars = 0;
	std::string curLocale = setlocale(LC_ALL, NULL);   //curLocale="C"
	setlocale(LC_ALL, "chs");
	const char* source = s.c_str();
	size_t charNum = sizeof(char)*s.size() + 1;
	// cout << "s.size():" << s.size() << endl;   //7

	wchar_t* dest = new wchar_t[charNum];
	mbstowcs_s(&convertedChars, dest, charNum, source, _TRUNCATE);
	// cout << "s2ws_convertedChars:" << convertedChars << endl; //6
	std::wstring result = dest;
	delete[] dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

DWORD Utils::FindProccess(const std::string & proccessName)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);

	if (!Process32First(snapshot, &pe))
		return 0;

	while (Process32Next(snapshot, &pe))
	{
		if (pe.szExeFile == proccessName)
			return pe.th32ProcessID;
	}

	return 0;
}

HMODULE Utils::GetModuleHandleSafe(const std::string & pszModuleName)
{
	HMODULE hmModuleHandle = NULL;

	do
	{
		if (pszModuleName.empty())
			hmModuleHandle = GetModuleHandleA(NULL);
		else
			hmModuleHandle = GetModuleHandleA(pszModuleName.c_str());

		std::this_thread::sleep_for(std::chrono::microseconds(100));
	} while (hmModuleHandle == NULL);

	return hmModuleHandle;
}

DWORD Utils::GetModuleBase(const std::string & ModuleName, DWORD ProcessID)
{
	if (ProcessID == 0)
		ProcessID = GetCurrentProcessId();

	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (!hSnapShot)
		return NULL;

	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL __RunModule = Module32First(hSnapShot, &lpModuleEntry);
	while (__RunModule)
	{
		if (lpModuleEntry.szModule == ModuleName)
		{
			CloseHandle(hSnapShot);
			return (DWORD)lpModuleEntry.modBaseAddr;
		}
		__RunModule = Module32Next(hSnapShot, &lpModuleEntry);
	}
	CloseHandle(hSnapShot);
	return NULL;
}

DWORD Utils::GetModuleBase(const std::string & ModuleName, DWORD * ModuleSize, DWORD ProcessID)
{
	if (ProcessID == 0)
		ProcessID = GetCurrentProcessId();

	MODULEENTRY32 lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	if (!hSnapShot)
		return NULL;

	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL __RunModule = Module32First(hSnapShot, &lpModuleEntry);
	while (__RunModule)
	{
		if (lpModuleEntry.szModule == ModuleName)
		{
			CloseHandle(hSnapShot);
			*ModuleSize = lpModuleEntry.dwSize;
			return (DWORD)lpModuleEntry.modBaseAddr;
		}
		__RunModule = Module32Next(hSnapShot, &lpModuleEntry);
	}
	CloseHandle(hSnapShot);
	return NULL;
}

std::vector<std::string> Utils::Split(const std::string & s, const std::string & delim)
{
	std::vector<std::string> result;
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		result.push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		result.push_back(s.substr(last, index - last));
	}

	return result;
}

std::string Utils::Trim(const std::string & s, const std::string & delim)
{
	if (s.empty())
		return s;

	std::string result = s;
	for (char c : delim)
	{
		result.erase(0, result.find_first_not_of(c));
		result.erase(result.find_last_not_of(c) + 1);
	}

	return result;
}

LPVOID Utils::GetVirtualFunction(LPVOID inst, DWORD index)
{
	if (inst == nullptr)
		return nullptr;

	PDWORD table = *(reinterpret_cast<PDWORD*>(inst));
	if (table == nullptr)
		return nullptr;

	return reinterpret_cast<LPVOID>(table[index]);
}

HWND Utils::g_hCurrentWindow = NULL;
static BOOL CALLBACK __EnumChildWindowsCallback(HWND hWnd, LPARAM lParam)
{
	DWORD pid = 0;
	GetWindowThreadProcessId(hWnd, &pid);

	if (pid == lParam && IsWindowVisible(hWnd))
	{
		char title[255], classname[64];
		title[0] = '\0';
		GetWindowTextA(hWnd, title, 255);
		RealGetWindowClassA(hWnd, classname, 64);

		std::cout << std::endl << classname << XorStr(" - ") << title << std::endl;
		std::cout << XorStr("Is this the correct window?") << std::endl;
		std::cout << XorStr("Enter y to determine, other negative.") << std::endl;

		char c;
		std::cin >> c;
		std::cout << std::endl;

		if (c == 'y' || c == 'Y')
		{
			Utils::g_hCurrentWindow = hWnd;
			return FALSE;
		}
	}

	return TRUE;
}

static BOOL CALLBACK __EnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
	DWORD pid = 0;
	GetWindowThreadProcessId(hWnd, &pid);

	if (pid == lParam && IsWindowVisible(hWnd))
		EnumChildWindows(hWnd, __EnumChildWindowsCallback, lParam);
	
	if (Utils::g_hCurrentWindow != NULL)
		return FALSE;

	return TRUE;
}

void Utils::FindWindowByProccess(DWORD ProcessID)
{
	if (ProcessID == 0)
		ProcessID = GetCurrentProcessId();
	
	g_hCurrentWindow = NULL;
	EnumWindows(__EnumWindowsCallback, ProcessID);
}

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
Fn Utils::GetVTableFunction(LPVOID inst, DWORD index)
{
	if (inst == nullptr)
		return nullptr;

	PDWORD table = *(reinterpret_cast<PDWORD*>(inst));
	if (table == nullptr)
		return nullptr;

	return reinterpret_cast<Fn>(table[index]);
}
