#include "utils.h"
#include "xorstr.h"
#include "drawing.h"
#include "console.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <thread>
#include <TlHelp32.h>
#include <cctype>
#include <regex>

#define INRANGE( x, a, b ) ( x >= a && x <= b )
#define getBits( x ) ( INRANGE( ( x & ( ~0x20 ) ), 'A', 'F' ) ? ( ( x & ( ~0x20 ) ) - 'A' + 0xa ) : ( INRANGE( x, '0', '9' ) ? x - '0' : 0 ) )
#define getByte( x ) ( getBits( x[ 0 ] ) << 4 | getBits( x[ 1 ] ) )
#define ThreadBasicInformation 0

HINSTANCE Utils::g_hInstance = NULL;
std::string Utils::g_sModulePath = "";
DWORD Utils::g_iSelfStart;
DWORD Utils::g_iSelfEnd;

void Utils::init(HINSTANCE inst)
{
	g_hInstance = inst;

	char buffer[MAX_PATH];
	if (GetModuleFileNameA(inst, buffer, MAX_PATH) == 0)
		GetModuleFileNameA(GetModuleHandleA(NULL), buffer, MAX_PATH);

	std::string tmp = buffer;
	g_sModulePath = tmp.substr(0, tmp.rfind('\\'));

	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)inst;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)inst) + pDOSHeader->e_lfanew);
	g_iSelfStart = pNTHeaders->OptionalHeader.ImageBase;
	g_iSelfEnd = pNTHeaders->OptionalHeader.ImageBase + pNTHeaders->OptionalHeader.SizeOfImage;
}

std::string Utils::BuildPath(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[MAX_PATH];
	buffer[0] = '\\';
	buffer[1] = '\0';

	if(text[0] != '\\')
		vsprintf_s(&(buffer[1]), MAX_PATH - 1, text, ap);
	else
		vsprintf_s(buffer, text, ap);

	va_end(ap);

	return g_sModulePath + buffer;
}

bool Utils::FarProc(LPVOID address)
{
	return FarProcEx(address, g_iSelfStart, g_iSelfEnd);
}

bool Utils::FarProcEx(LPVOID address, DWORD start, DWORD end)
{
	if (address == nullptr)
		return false;

	return (((DWORD)address < start) || ((DWORD)address > end));
}

void Utils::log(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	strcat_s(buffer, "\r\n");

	std::cout << buffer;
	OutputDebugStringA(buffer);
	g_pConsole->Print(buffer);
}

void Utils::logToFile(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	strcat_s(buffer, "\r\n");

	std::cout << '[' << GetDateTime() << ']' << buffer;
	OutputDebugStringA(buffer);
	g_pConsole->Print(buffer);
}

void Utils::logError(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	strcat_s(buffer, "\r\n");

	std::cout << '[' << GetDateTime() << ']' << buffer;
	OutputDebugStringA(buffer);
}

void Utils::log2()
{
	std::cout << std::endl;
}

void Utils::PrintInfo(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	g_pDrawing->PrintInfo(CDrawing::WHITE, buffer);
}

std::string Utils::GetTime()
{
	tm timeInfo;
	time_t t = time(nullptr);
	localtime_s(&timeInfo, &t);

	std::stringstream ss;
	ss << timeInfo.tm_hour << ':' << timeInfo.tm_min << ':' << timeInfo.tm_sec;
	return ss.str();
}

std::string Utils::GetDate()
{
	tm timeInfo;
	time_t t = time(nullptr);
	localtime_s(&timeInfo, &t);

	std::stringstream ss;
	ss << timeInfo.tm_year + 1900 << '/' << timeInfo.tm_mon + 1 << '/' << timeInfo.tm_mday;
	return ss.str();
}

std::string Utils::GetWeek()
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

std::string Utils::GetDateTime()
{
	tm timeInfo;
	time_t t = time(nullptr);
	localtime_s(&timeInfo, &t);

	std::stringstream ss;
	ss << timeInfo.tm_year + 1900 << '/' << timeInfo.tm_mon + 1 << '/' << timeInfo.tm_mday << ' ' <<
		timeInfo.tm_hour << ':' << timeInfo.tm_min << ':' << timeInfo.tm_sec;
	return ss.str();
}

int Utils::GetCpuUsage()
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

DWORD Utils::FindProccess(std::string_view proccessName)
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

DWORD Utils::GetModuleBase(std::string_view ModuleName, DWORD ProcessID)
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

DWORD Utils::GetModuleBase(std::string_view ModuleName, DWORD * ModuleSize, DWORD ProcessID)
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

DWORD Utils::FindPattern(const std::string & szModules, const std::string & szPattern)
{
	auto info = GetModuleSize(szModules);
	return FindPattern(info.first, info.second, szPattern);
}

DWORD Utils::FindPattern(DWORD dwBegin, DWORD dwEnd, const std::string & szPattern)
{
	const char *pat = szPattern.c_str();
	DWORD firstMatch = NULL;
	for (DWORD pCur = dwBegin; pCur < dwEnd; pCur++)
	{
		if (!*pat)
			return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE *)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;
			if (!pat[2])
				return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;
			else
				pat += 2;
		}
		else
		{
			pat = szPattern.c_str();
			firstMatch = 0;
		}
	}

	return NULL;
}

DWORD Utils::FindPattern(const std::string & szModules, const std::string & szPattern, std::string szMask)
{
	auto info = GetModuleSize(szModules);
	return FindPattern(info.first, info.second, szPattern, szMask);
}

DWORD Utils::FindPattern(DWORD dwBegin, DWORD dwEnd, const std::string & szPattern, std::string szMask)
{
	BYTE First = szPattern[0];
	PBYTE BaseAddress = (PBYTE)(dwBegin);
	PBYTE Max = (PBYTE)(dwEnd - szPattern.length());

	const static auto CompareByteArray = [](PBYTE Data, const char* Signature, const char* Mask) -> bool
	{
		for (; *Signature; ++Signature, ++Data, ++Mask)
		{
			if (*Mask == '?' || *Signature == '\x00' || *Signature == '\x2A')
			{
				continue;
			}
			if (*Data != *Signature)
			{
				return false;
			}
		}
		return true;
	};

	if (szMask.length() < szPattern.length())
	{
		for (auto i = szPattern.begin() + (szPattern.length() - 1); i != szPattern.end(); ++i)
			szMask.push_back(*i == '\x2A' || *i == '\x00' ? '?' : 'x');
	}

	for (; BaseAddress < Max; ++BaseAddress)
	{
		if (*BaseAddress != First)
		{
			continue;
		}
		if (CompareByteArray(BaseAddress, szPattern.c_str(), szMask.c_str()))
		{
			return (DWORD)BaseAddress;
		}
	}

	return NULL;
}

std::vector<std::string> Utils::StringSplit(std::string_view s, std::string_view delim)
{
	std::vector<std::string> result;
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		result.emplace_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		result.emplace_back(s.substr(last, index - last));
	}

	return result;
}

std::string Utils::StringTrim(const std::string & s, const std::string & delim)
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

// 搜索字符串，支持通配符 ? 和 *
// s = 被搜索的字符串 (source)
// p = 用来搜索的字符串 (pattern)
// caseSensitive = 是否检查大小写
size_t Utils::StringFind(const std::string & s, const std::string & p, bool caseSensitive)
{
	// 比较字符
	auto equal = [&caseSensitive](char s, char p) -> bool
	{
		if (caseSensitive)
			return (s == p);

		return (std::tolower(s) == std::tolower(p));
	};

	// 当前比较位置
	// sp = sourcePosition
	// pp = patternPosition
	size_t sp = 0, pp = 0;

	// 上次 * 命中位置
	// lsp = lastStarPosition
	size_t lsp = std::string::npos;

	// 上次 * 的偏移
	// lspo = lastStarPositionOffset
	size_t lspo = std::string::npos;

	// 当前命中开始位置
	// msp = matchStartPosition
	size_t msp = std::string::npos;

	// 字符串长度
	// sm = sourceMax
	// pm = patternMax
	size_t sm = s.length(), pm = p.length();

	while (sp < sm && pp < pm)
	{
		if (p[pp] == '?')
		{
			// 获取开始位置
			if (msp == std::string::npos)
				msp = sp;
			
			// 单个任意字符匹配
			++sp;
			++pp;
		}
		else if (p[pp] == '*')
		{
			lspo = 0;
			
			// 合并连续的 * 和 ?
			while (++pp < pm && (p[pp] == '*' || p[pp] == '?'))
				++lspo;
			lsp = --pp;

			// 获取开始位置
			if (msp == std::string::npos)
				msp = sp;

			// p 最后一个字符是 * 则不需要进行匹配了
			if (pp + 1 >= pm || sp + 1 >= sm)
				return msp;

			// 匹配预测，跳出 * 的匹配
			if (equal(p[pp + 1], s[sp + 1]))
			{
				++pp;
				++sp;
			}

			// 进行下一次的匹配
			++sp;
		}
		else if (!equal(s[sp], p[pp]))
		{
			if (lsp < pm)
			{
				// 之前的预测失败了
				// 退回上一个 * 处再次预测
				msp = sp;
				// msp = sp - lsp;
				pp = lsp;
			}
			else
			{
				// 模式搜索完全失败时才需要前进
				// 不然只需要重新开始匹配模式
				if (pp == 0)
				{
					// 一般的字符串匹配
					++sp;
				}

				pp = 0;
				msp = std::string::npos;
			}
		}
		else
		{
			// 获取开始位置
			if (msp == std::string::npos)
				msp = sp;

			++sp;
			++pp;
		}
	}

	return msp;
}

// 比较字符串，支持通配符 ? 和 *
bool Utils::StringEqual(const std::string& s, std::string p, bool caseSensitive)
{
	// 正则转义
	StringReplace(p, u8"\\", u8"\\\\");
	StringReplace(p, u8".", u8"\\.");
	StringReplace(p, u8"+", u8"\\+");
	StringReplace(p, u8"(", u8"\\(");
	StringReplace(p, u8")", u8"\\)");
	StringReplace(p, u8"[", u8"\\[");
	StringReplace(p, u8"]", u8"\\]");
	StringReplace(p, u8"{", u8"\\{");
	StringReplace(p, u8"}", u8"\\}");
	StringReplace(p, u8"|", u8"\\|");
	StringReplace(p, u8"^", u8"\\^");
	StringReplace(p, u8"$", u8"\\$");
	StringReplace(p, u8"*", u8".*");
	StringReplace(p, u8"?", u8".");

	auto flags = (std::regex::optimize|std::regex::ECMAScript);
	if (!caseSensitive)
		flags |= std::regex::icase;

	return std::regex_match(s, std::regex(p, flags));
}

std::string & Utils::StringReplace(std::string & s, const std::string & p, const std::string & newText)
{
	for (std::string::size_type pos(0); pos != std::string::npos; pos += newText.length())
	{
		pos = s.find(p, pos);
		if (pos != std::string::npos)
			s.replace(pos, p.length(), newText);
		else
			break;
	}
	return s;
}

//功  能：在lpszSour中查找字符串lpszFind，lpszFind中可以包含通配字符‘?’
//参  数：nStart为在lpszSour中的起始查找位置
//返回值：成功返回匹配位置，否则返回-1
//注  意：Called by “bool MatchingString()”
int Utils::FindingString(const char* lpszSour, const char* lpszFind, int nStart /* = 0 */)
{
	//  ASSERT(lpszSour && lpszFind && nStart >= 0);
	if (lpszSour == NULL || lpszFind == NULL || nStart < 0)
		return -1;

	int m = strlen(lpszSour);
	int n = strlen(lpszFind);

	if (nStart + n > m)
		return -1;

	if (n == 0)
		return nStart;

	//KMP算法
	int* next = new int[n];
	//得到查找字符串的next数组
	{   n--;

	int j, k;
	j = 0;
	k = -1;
	next[0] = -1;

	while (j < n)
	{
		if (k == -1 || lpszFind[k] == '?' || lpszFind[j] == lpszFind[k])
		{
			j++;
			k++;
			next[j] = k;
		}
		else
			k = next[k];
	}

	n++;
	}

	int i = nStart, j = 0;
	while (i < m && j < n)
	{
		if (j == -1 || lpszFind[j] == '?' || lpszSour[i] == lpszFind[j])
		{
			i++;
			j++;
		}
		else
			j = next[j];
	}

	delete[] next;

	if (j >= n)
		return i - n;
	else
		return -1;
}

//功   能：带通配符的字符串匹配
//参   数：lpszSour是一个普通字符串；
//            lpszMatch是一可以包含通配符的字符串；
//            bMatchCase为0，不区分大小写，否则区分大小写。
//返  回  值：匹配，返回1；否则返回0。
//通配符意义：
//      ‘*’ 代表任意字符串，包括空字符串；
//      ‘?’ 代表任意一个字符，不能为空；
//时   间：    2001.11.02  13:00
bool Utils::MatchingString(const char* lpszSour, const char* lpszMatch, bool bMatchCase /*  = true */)
{
	//  ASSERT(AfxIsValidString(lpszSour) && AfxIsValidString(lpszMatch));
	if (lpszSour == NULL || lpszMatch == NULL)
		return false;

	if (lpszMatch[0] == 0)//Is a empty string
	{
		if (lpszSour[0] == 0)
			return true;
		else
			return false;
	}

	int i = 0, j = 0;

	//生成比较用临时源字符串'szSource'
	char* szSource = new char[(j = strlen(lpszSour) + 1)];

	if (bMatchCase)
	{   //memcpy(szSource, lpszSour, j);
		while (*(szSource + i) = *(lpszSour + i++));
	}
	else
	{   //Lowercase 'lpszSour' to 'szSource'
		i = 0;
		while (lpszSour[i])
		{
			if (lpszSour[i] >= 'A' && lpszSour[i] <= 'Z')
				szSource[i] = lpszSour[i] - 'A' + 'a';
			else
				szSource[i] = lpszSour[i];

			i++;
		}
		szSource[i] = 0;
	}

	//生成比较用临时匹配字符串'szMatcher'
	char* szMatcher = new char[strlen(lpszMatch) + 1];

	//把lpszMatch里面连续的“*”并成一个“*”后复制到szMatcher中
	i = j = 0;
	while (lpszMatch[i])
	{
		szMatcher[j++] = (!bMatchCase) ?
			((lpszMatch[i] >= 'A' && lpszMatch[i] <= 'Z') ?//Lowercase lpszMatch[i] to szMatcher[j]
				lpszMatch[i] - 'A' + 'a' :
				lpszMatch[i]
				) :
			lpszMatch[i];        //Copy lpszMatch[i] to szMatcher[j]
								 //Merge '*'
		if (lpszMatch[i] == '*')
			while (lpszMatch[++i] == '*');
		else
			i++;
	}
	szMatcher[j] = 0;

	//开始进行匹配检查

	int nMatchOffset, nSourOffset;

	bool bIsMatched = true;
	nMatchOffset = nSourOffset = 0;
	while (szMatcher[nMatchOffset])
	{
		if (szMatcher[nMatchOffset] == '*')
		{
			if (szMatcher[nMatchOffset + 1] == 0)
			{   //szMatcher[nMatchOffset]是最后一个字符

				bIsMatched = true;
				break;
			}
			else
			{   //szMatcher[nMatchOffset+1]只能是'?'或普通字符

				int nSubOffset = nMatchOffset + 1;

				while (szMatcher[nSubOffset])
				{
					if (szMatcher[nSubOffset] == '*')
						break;
					nSubOffset++;
				}

				if (strlen(szSource + nSourOffset) <
					size_t(nSubOffset - nMatchOffset - 1))
				{   //源字符串剩下的长度小于匹配串剩下要求长度
					bIsMatched = false; //判定不匹配
					break;          //退出
				}

				if (!szMatcher[nSubOffset])//nSubOffset is point to ender of 'szMatcher'
				{   //检查剩下部分字符是否一一匹配

					nSubOffset--;
					int nTempSourOffset = strlen(szSource) - 1;
					//从后向前进行匹配
					while (szMatcher[nSubOffset] != '*')
					{
						if (szMatcher[nSubOffset] == '?')
							;
						else
						{
							if (szMatcher[nSubOffset] != szSource[nTempSourOffset])
							{
								bIsMatched = false;
								break;
							}
						}
						nSubOffset--;
						nTempSourOffset--;
					}
					break;
				}
				else//szMatcher[nSubOffset] == '*'
				{
					nSubOffset -= nMatchOffset;

					char* szTempFinder = new char[nSubOffset];
					nSubOffset--;
					memcpy(szTempFinder, szMatcher + nMatchOffset + 1, nSubOffset);
					szTempFinder[nSubOffset] = 0;

					int nPos = FindingString(szSource + nSourOffset, szTempFinder, 0);
					delete[] szTempFinder;

					if (nPos != -1)//在'szSource+nSourOffset'中找到szTempFinder
					{
						nMatchOffset += nSubOffset;
						nSourOffset += (nPos + nSubOffset - 1);
					}
					else
					{
						bIsMatched = false;
						break;
					}
				}
			}
		}       //end of "if(szMatcher[nMatchOffset] == '*')"
		else if (szMatcher[nMatchOffset] == '?')
		{
			if (!szSource[nSourOffset])
			{
				bIsMatched = false;
				break;
			}
			if (!szMatcher[nMatchOffset + 1] && szSource[nSourOffset + 1])
			{   //如果szMatcher[nMatchOffset]是最后一个字符，
				//且szSource[nSourOffset]不是最后一个字符
				bIsMatched = false;
				break;
			}
			nMatchOffset++;
			nSourOffset++;
		}
		else//szMatcher[nMatchOffset]为常规字符
		{
			if (szSource[nSourOffset] != szMatcher[nMatchOffset])
			{
				bIsMatched = false;
				break;
			}
			if (!szMatcher[nMatchOffset + 1] && szSource[nSourOffset + 1])
			{
				bIsMatched = false;
				break;
			}
			nMatchOffset++;
			nSourOffset++;
		}
	}

	delete[] szSource;
	delete[] szMatcher;
	return bIsMatched;
}

//功  能：多重匹配，不同匹配字符串之间用‘,’隔开
//          如：“*.h,*.cpp”将依次匹配“*.h”和“*.cpp”
//参  数：nMatchLogic = 0, 不同匹配求或，else求与；bMatchCase, 是否大小敏感
//返回值：如果bRetReversed = 0, 匹配返回true；否则不匹配返回true
//时  间：2001.11.02  17:00
bool Utils::MultiMatching(const char* lpszSour, const char* lpszMatch, int nMatchLogic /* = 0 */, bool bRetReversed /* = 0 */, bool bMatchCase /* = true */)
{
	//  ASSERT(AfxIsValidString(lpszSour) && AfxIsValidString(lpszMatch));
	if (lpszSour == NULL || lpszMatch == NULL)
		return false;

	char* szSubMatch = new char[strlen(lpszMatch) + 1];
	bool bIsMatch;

	if (nMatchLogic == 0)//求或
	{
		bIsMatch = 0;
		int i = 0;
		int j = 0;
		while (1)
		{
			if (lpszMatch[i] != 0 && lpszMatch[i] != ',')
				szSubMatch[j++] = lpszMatch[i];
			else
			{
				szSubMatch[j] = 0;
				if (j != 0)
				{
					bIsMatch = MatchingString(lpszSour, szSubMatch, bMatchCase);
					if (bIsMatch)
						break;
				}
				j = 0;
			}

			if (lpszMatch[i] == 0)
				break;
			i++;
		}
	}
	else//求与
	{
		bIsMatch = 1;
		int i = 0;
		int j = 0;
		while (1)
		{
			if (lpszMatch[i] != 0 && lpszMatch[i] != ',')
				szSubMatch[j++] = lpszMatch[i];
			else
			{
				szSubMatch[j] = 0;

				bIsMatch = MatchingString(lpszSour, szSubMatch, bMatchCase);
				if (!bIsMatch)
					break;

				j = 0;
			}

			if (lpszMatch[i] == 0)
				break;
			i++;
		}
	}

	delete[]szSubMatch;

	if (bRetReversed)
		return !bIsMatch;
	else
		return bIsMatch;
}

PVOID Utils::GetVirtualFunction(PVOID inst, DWORD index)
{
	if (inst == nullptr)
		return nullptr;

	PDWORD table = *(reinterpret_cast<PDWORD*>(inst));
	if (table == nullptr)
		return nullptr;

	return reinterpret_cast<PVOID>(table[index]);
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

std::pair<DWORD, DWORD> Utils::GetModuleSize(const std::string& pszModuleName)
{
	HMODULE hmModule = GetModuleHandleSafe(pszModuleName);
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hmModule;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hmModule) + pDOSHeader->e_lfanew);

	DWORD dwAddress = ((DWORD)hmModule) + pNTHeaders->OptionalHeader.BaseOfCode;
	DWORD dwLength = ((DWORD)hmModule) + pNTHeaders->OptionalHeader.SizeOfCode;
	return { dwAddress, dwLength };
}
