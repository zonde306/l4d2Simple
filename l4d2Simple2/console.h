#pragma once

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <functional>
#include <memory>
#include <imgui.h>

class CConsole
{
public:
	CConsole();
	~CConsole();

	void Init();
	void DrawConsole();

	void Print(const char* text, ...);
	void PrintLine(const char* text, ...);

public:
	bool ExecuteCommand(const std::string& cmd, bool history = false);
	int Callback_TextEdit(ImGuiTextEditCallbackData* data);

	using FnCommandCallback = std::function<bool(const std::string&, const std::vector<std::string>&)>;

	bool AddConCmd(const std::string& cmd, FnCommandCallback callback);
	bool RemoveConCmd(const std::string& cmd);
	bool FindConCmd(const std::string& cmd);

	std::pair<std::string, std::string> ParseCmdArgs(const std::string& cmdargs);
	std::vector<std::string> ParseArgList(const std::string& args);

private:
	void CheckConsoleLength();

private:
	bool ConCmd_Clear(const std::string& cmd, const std::vector<std::string>& arg);
	bool ConCmd_Help(const std::string& cmd, const std::vector<std::string>& arg);
	bool ConCmd_Echo(const std::string& cmd, const std::vector<std::string>& arg);
	bool ConCmd_Dump(const std::string& cmd, const std::vector<std::string>& arg);

private:
	bool m_bShowConsole = false;
	bool m_bAllowEditMode = false;
	char m_szCommandLine[255];
	char m_szConsoleString[4096];
	char m_szConsoleFilter[255];

	size_t m_iCurrentHistory = UINT_MAX;

	std::vector<std::string> m_sHistoryLine;
	std::map<std::string, FnCommandCallback> m_CommandList;
	std::deque<std::string> m_ConsoleString;

	size_t m_iMaxStringLength;
};

extern std::unique_ptr<CConsole> g_pConsole;