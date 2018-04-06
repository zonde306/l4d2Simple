#include "console.h"
#include "drawing.h"
#include "config.h"
#include "xorstr.h"
#include "utils.h"
#include <cctype>

std::unique_ptr<CConsole> g_pConsole;
#define BIND_CONCMD(_fn)	std::bind(&CConsole::_fn, this, std::placeholders::_1, std::placeholders::_2)

CConsole::CConsole()
{
}

CConsole::~CConsole()
{
}

void CConsole::Init()
{
	// 一些简易的命令
	AddConCmd(XorStr("clear"), BIND_CONCMD(ConCmd_Clear));
	AddConCmd(XorStr("help"), BIND_CONCMD(ConCmd_Help));
	AddConCmd(XorStr("echo"), BIND_CONCMD(ConCmd_Echo));
	AddConCmd(XorStr("dump"), BIND_CONCMD(ConCmd_Dump));
}

void CConsole::DrawConsole()
{
	if (!ImGui::Begin(XorStr("Console"), &m_bShowConsole))
	{
		ImGui::End();
		return;
	}

	static bool isFirstEnter = true;
	const std::string mainKeys = XorStr("Windows");

	if (isFirstEnter)
	{
		isFirstEnter = false;

		ImGui::SetWindowPos(ImVec2(g_pConfig->GetFloat(mainKeys, XorStr("console_x"), 500.0f),
			g_pConfig->GetFloat(mainKeys, XorStr("console_y"), 400.0f)));

		ImGui::SetWindowSize(ImVec2(g_pConfig->GetFloat(mainKeys, XorStr("console_w"), 500.0f),
			g_pConfig->GetFloat(mainKeys, XorStr("console_h"), 400.0f)));
	}

	ImGui::PushFont(g_pDrawing->m_imFonts.Fonts.back());
	ImGui::InputText(XorStr("ConFilter"), m_szConsoleFilter, 255);
	ImGui::SameLine();
	ImGui::Checkbox(XorStr("ViewOnly"), &m_bAllowEditMode);

	ImGui::Separator();

	if (m_bAllowEditMode)
	{
		m_szConsoleString[0] = '\0';
		for (const auto& it : m_ConsoleString)
		{
			if (m_szConsoleFilter[0] != '\0' && it.find(m_szConsoleFilter) == std::string::npos)
				continue;
			
			strcat_s(m_szConsoleString, it.c_str());
		}
		
		ImGui::InputTextMultiline(XorStr("##console"), m_szConsoleString, ARRAYSIZE(m_szConsoleString),
			ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AllowTabInput);
	}
	else
	{
		ImGui::BeginChild(XorStr("##console"), ImVec2(-1.0f, -1.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

		for (const auto& it : m_ConsoleString)
		{
			if (m_szConsoleFilter[0] != '\0' && it.find(m_szConsoleFilter) == std::string::npos)
				continue;
			
			char color = it[0];
			switch (color)
			{
			case '\x00':	// 禁用
				ImGui::TextDisabled(it.substr(1).c_str());
				break;
			case '\x01':	// 红色
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), it.substr(1).c_str());
				break;
			case '\x02':	// 橙色
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.25f, 1.0f), it.substr(1).c_str());
				break;
			case '\x03':	// 黄色
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), it.substr(1).c_str());
				break;
			case '\x04':	// 绿色
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), it.substr(1).c_str());
				break;
			case '\x05':	// 青色
				ImGui::TextColored(ImVec4(0.0f, 0.5f, 0.25f, 1.0f), it.substr(1).c_str());
				break;
			case '\x06':	// 蓝色
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), it.substr(1).c_str());
				break;
			case '\x07':	// 紫色
				ImGui::TextColored(ImVec4(0.5f, 0.0f, 1.0f, 1.0f), it.substr(1).c_str());
				break;
			default:
				// ImGui::Text(it.c_str());
				ImGui::TextWrapped(it.c_str());
				break;
			}
			
			if (ImGui::BeginPopupContextItem(XorStr("ConStrPopup")))
			{
				if (ImGui::Selectable(XorStr("copy")))
					ImGui::SetClipboardText(it.c_str());
				
				ImGui::EndPopup();
			}
		}

		ImGui::EndChild();
	}

	ImGui::Separator();
	if (ImGui::InputText(XorStr("##cmdinput"), m_szCommandLine, 255, ImGuiInputTextFlags_EnterReturnsTrue|
		ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory,
		[](ImGuiTextEditCallbackData* data) -> int
		{
			CConsole* ptr = reinterpret_cast<CConsole*>(data->UserData);
			return ptr->Callback_TextEdit(data);
		}, this))
	{
		ExecuteCommand(m_szCommandLine);
		m_szCommandLine[0] = '\0';
		m_iCurrentHistory = UINT_MAX;
	}

	ImGui::PopFont();

	ImVec2 window = ImGui::GetWindowPos();
	g_pConfig->SetValue(mainKeys, XorStr("console_x"), window.x);
	g_pConfig->SetValue(mainKeys, XorStr("console_y"), window.y);
	window = ImGui::GetWindowSize();
	g_pConfig->SetValue(mainKeys, XorStr("console_w"), window.x);
	g_pConfig->SetValue(mainKeys, XorStr("console_h"), window.y);

	ImGui::End();
}

void CConsole::Print(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	m_ConsoleString.push_back(buffer);
	CheckConsoleLength();
}

void CConsole::PrintLine(const char * text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	m_ConsoleString.push_back(buffer + '\n');
	CheckConsoleLength();
}

bool CConsole::ExecuteCommand(const std::string & cmd, bool history)
{
	auto it = m_CommandList.find(cmd);
	if (it == m_CommandList.end())
	{
		PrintLine(XorStr("Unknown command: %s"), cmd.c_str());
		return true;
	}

	auto ca = ParseCmdArgs(cmd);
	size_t offset = m_ConsoleString.size() - 1;

	if (it->second(ca.first, ParseArgList(ca.second)))
		m_ConsoleString.insert(m_ConsoleString.begin() + offset, "] " + cmd);

	CheckConsoleLength();

	if (history)
		m_sHistoryLine.push_back(cmd);

	return false;
}

int CConsole::Callback_TextEdit(ImGuiTextEditCallbackData * data)
{
	switch (data->EventFlag)
	{
		// 按下了 Tab 键，尝试自动补全
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			if (data->BufTextLen <= 0 || data->Buf == nullptr)
				break;

			using match_type = std::pair<std::string, FnCommandCallback>;
			std::vector<match_type> match;

			std::copy_if(m_CommandList.begin(), m_CommandList.end(), std::back_inserter(match),
				[data](const match_type& it) -> bool
			{
				// 范围比较
				return std::equal(it.first.begin(), it.first.end(), data->Buf, data->Buf + data->BufTextLen,
					[](char c1, char c2) -> bool
				{
					// 不区分大小写进行比较
					return (std::tolower(c1) == std::tolower(c2));
				});
			});
			
			if (match.empty())
				break;

			if (match.size() == 1)
			{
				// 唯一的选项，不需要手动选择
				strcpy_s(data->Buf, data->BufSize, match.front().first.c_str());
				data->SelectionStart = data->SelectionEnd = 0;
				data->BufTextLen = static_cast<int>(match.front().first.length());
				data->CursorPos = max(data->BufTextLen - 1, 0);
				break;
			}

			const std::string popup = XorStr("selectOnceCommand");
			ImGui::OpenPopup(popup.c_str());

			// 弹出一个列表框，可惜不能用键盘控制
			// 如果自己画一个会被菜单挡住的，效果更差
			if (ImGui::BeginPopup(popup.c_str()))
			{
				for(auto it = match.begin(); it != match.end(); ++it)
				{
					if (!ImGui::Selectable(it->first.c_str()))
						continue;

					// 选择了一个匹配项
					strcpy_s(data->Buf, data->BufSize, it->first.c_str());
					data->SelectionStart = data->SelectionEnd = 0;
					data->BufTextLen = static_cast<int>(it->first.length());
					data->CursorPos = max(data->BufTextLen - 1, 0);



					break;
				}
				
				ImGui::EndPopup();
			}

			break;
		}

		// 按下了 ↑ 或 ↓ 键，尝试回溯历史记录
		case ImGuiInputTextFlags_CallbackHistory:
		{
			if (m_sHistoryLine.empty())
				break;

			const size_t lastHistory = m_iCurrentHistory;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				// 上一个
				if (m_iCurrentHistory <= 0)
					m_iCurrentHistory = 0;
				else if (m_iCurrentHistory > m_sHistoryLine.size())
					m_iCurrentHistory = m_sHistoryLine.size() - 1;
				else
					--m_iCurrentHistory;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (m_iCurrentHistory < 0)
					m_iCurrentHistory = 0;
				else if (m_iCurrentHistory >= m_sHistoryLine.size())
					m_iCurrentHistory = m_sHistoryLine.size() - 1;
				else
					++m_iCurrentHistory;
			}
			
			if (lastHistory == m_iCurrentHistory)
				break;

			strcpy_s(data->Buf, data->BufSize, m_sHistoryLine[m_iCurrentHistory].c_str());
			data->SelectionStart = data->SelectionEnd = 0;
			data->BufTextLen = static_cast<int>(m_sHistoryLine[m_iCurrentHistory].length());
			data->CursorPos = max(data->BufTextLen - 1, 0);
			break;
		}
	}
	
	return 0;
}

bool CConsole::AddConCmd(const std::string & cmd, FnCommandCallback callback)
{
	auto it = m_CommandList.find(cmd);
	bool result = true;
	if (it != m_CommandList.end())
		result = false;

	m_CommandList[cmd] = callback;
	return result;
}

bool CConsole::RemoveConCmd(const std::string & cmd)
{
	auto it = m_CommandList.find(cmd);
	if (it == m_CommandList.end())
		return false;

	m_CommandList.erase(it);
	return true;
}

std::pair<std::string, std::string> CConsole::ParseCmdArgs(const std::string & cmdargs)
{
	std::string cmd = Utils::Trim(cmdargs);

	bool inGroup = false;
	for (size_t i = 0; i < cmd.length(); ++i)
	{
		if (cmd[i] == '"')
			inGroup = !inGroup;

		if (inGroup)
			continue;

		if (cmd[i] != ' ')
			continue;

		return std::make_pair(Utils::Trim(cmd.substr(0, i), XorStr(" \r\n\t\"'")),
			Utils::Trim(cmd.substr(i + 1), XorStr(" \r\n\t\"'")));
	}

	return std::make_pair(cmd, "");
}

std::vector<std::string> CConsole::ParseArgList(const std::string & args)
{
	std::string cmd = Utils::Trim(args);

	std::vector<std::string> result;
	size_t groupBegin = std::string::npos;
	for (size_t i = 0; i < cmd.size(); ++i)
	{
		if (cmd[i] == '"')
		{
			if (groupBegin != std::string::npos)
			{
				result.push_back(cmd.substr(groupBegin + 1, i));
				groupBegin = std::string::npos;
			}
			else
			{
				groupBegin = i;
			}

			continue;
		}

		if (cmd[i] == ' ' && groupBegin == std::string::npos)
		{
			size_t begin = cmd.rfind(' ', i - 1);
			if(begin != std::string::npos)
				result.push_back(cmd.substr(begin + 1, i));
			else
				result.push_back(cmd.substr(0, i));
		}
	}

	return result;
}

void CConsole::CheckConsoleLength()
{
	if (m_iMaxStringLength > 0 && m_ConsoleString.size() > m_iMaxStringLength)
		m_ConsoleString.erase(m_ConsoleString.begin(), m_ConsoleString.begin() + (m_ConsoleString.size() - m_iMaxStringLength));
}

bool CConsole::ConCmd_Clear(const std::string & cmd, const std::vector<std::string>& arg)
{
	m_ConsoleString.clear();
	return false;
}

bool CConsole::ConCmd_Help(const std::string & cmd, const std::vector<std::string>& arg)
{
	if (arg.size() < 1)
	{
		PrintLine(XorStr("usag: help <command>"));
		return true;
	}

	PrintLine(XorStr("This command has no description."));
	return true;
}

bool CConsole::ConCmd_Echo(const std::string & cmd, const std::vector<std::string>& arg)
{
	if (arg.size() < 1)
	{
		PrintLine(XorStr("usag: echo <message>"));
		return true;
	}

	PrintLine(arg.back().c_str());
	return false;
}

bool CConsole::ConCmd_Dump(const std::string & cmd, const std::vector<std::string>& arg)
{
	std::fstream file(Utils::BuildPath(XorStr("condump.txt")), std::ios::out|std::ios::ate);
	if (file.bad() || !file.is_open())
	{
		PrintLine(XorStr("dump failed."));
		return true;
	}

	for (const auto& it : m_ConsoleString)
		file << it;

	file.close();
	PrintLine(XorStr("dump success."));
	return true;
}
