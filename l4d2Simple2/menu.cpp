#include <sstream>
#include "menu.h"
#include "xorstr.h"
#include "utils.h"
#include "dx9hook.h"
#include "../imgui/examples/directx9_example/imgui_impl_dx9.h"
#include "../hl2sdk/hook.h"
#include "../l4d2Simple2/config.h"

std::unique_ptr<CBaseMenu> g_pBaseMenu;
time_t g_tpPlayingTimer = 0;
time_t g_tpGameTimer = 0;

void _OnMenuStateChanged(bool visible)
{
	g_pInterface->Surface->SetCursorAlwaysVisible(visible);
	ImGui::GetIO().MouseDrawCursor = visible;

	static bool isFirstEnter = true;

	if (isFirstEnter && visible)
	{
		isFirstEnter = false;
		g_pClientHook->LoadConfig();
	}
	else if (!visible)
	{
		g_pClientHook->SaveConfig();
	}
}

void CBaseMenu::Init()
{
	// g_pDirextXHook->m_vfnDrawIndexedPrimitive.emplace_back(Hooked_DrawIndexedPrimitive);
	// g_pDirextXHook->AddHook_DrawIndexedPrimitive(Hooked_DrawIndexedPrimitive);
	ImGui::StyleColorsDark();
	g_tpGameTimer = time(nullptr);
	g_tpPlayingTimer = 0;
}

void CBaseMenu::OnPresent()
{
	if (!g_bHasShowMenu)
		return;

	if (!ImGui::Begin(XorStr(u8"l4d2Simple2 (oﾟvﾟ)ノ"), &g_bHasShowMenu))
	{
		ImGui::End();
		_OnMenuStateChanged(false);
		return;
	}

	static bool isFirstEnter = true;
	const std::string mainKeys = XorStr("Windows");

	if (isFirstEnter)
	{
		isFirstEnter = false;
		
		ImGui::SetWindowPos(ImVec2(g_pConfig->GetFloat(mainKeys, XorStr("mainwindow_x"), 400.0f),
			g_pConfig->GetFloat(mainKeys, XorStr("mainwindow_y"), 300.0f)));

		ImGui::SetWindowSize(ImVec2(g_pConfig->GetFloat(mainKeys, XorStr("mainwindow_w"), 400.0f),
			g_pConfig->GetFloat(mainKeys, XorStr("mainwindow_h"), 300.0f)));
	}

	ImGui::PushFont(g_pDrawing->m_imFonts.Fonts.back());
	ImGui::Text(XorStr("Version: 1.0 | Created by zonde306"));
	ImGui::Text(XorStr(u8"此辅助免费且开源，如果你是通过购买获得，说明你被骗了。"));
	// ImGui::GetIO().MouseDrawCursor = true;

	const static auto GetWeakName = [](int weak) -> std::string
	{
		switch (weak)
		{
		case 0:
		case 7:
			return XorStr(u8"星期日");
		case 1:
			return XorStr(u8"星期一");
		case 2:
			return XorStr(u8"星期二");
		case 3:
			return XorStr(u8"星期三");
		case 4:
			return XorStr(u8"星期四");
		case 5:
			return XorStr(u8"星期五");
		case 6:
			return XorStr(u8"星期六");
		}

		return "";
	};

	const static auto GetTimeDuration = [](time_t duration) -> std::string
	{
		const auto SECONDS_IN_HOUR = 3600;
		const auto SECONDS_IN_MINUTE = 60;

		time_t bh = 0, bm = 0, bs = 0;

		if (duration >= SECONDS_IN_HOUR)
		{
			bh = duration / SECONDS_IN_HOUR;
			duration = duration % SECONDS_IN_HOUR;
		}

		if (duration >= SECONDS_IN_MINUTE)
		{
			bm = duration / SECONDS_IN_MINUTE;
			duration = duration % SECONDS_IN_MINUTE;
		}

		bs = duration;

		char buffer[16];
		buffer[0] = '\0';

		if (bh > 0)
			sprintf_s(buffer, 16, "%I64dh %I64dm %I64ds", bh, bm, bs);
		else if (bm > 0)
			sprintf_s(buffer, 16, "%I64dm %I64ds", bm, bs);
		else
			sprintf_s(buffer, 16, "%I64ds", bs);

		return buffer;
	};

	// 显示系统时间
	{
		ImGui::Separator();

		tm timeInfo;
		time_t t = time(nullptr);
		localtime_s(&timeInfo, &t);

		/*
		int week = (timeInfo.tm_mday + 2 * (timeInfo.tm_mon + 1) + 3 *
		((timeInfo.tm_mon + 1) + 1) / 5 + (timeInfo.tm_year + 1900) +
		(timeInfo.tm_year + 1900) / 4 - (timeInfo.tm_year + 1900) / 100 +
		(timeInfo.tm_year + 1900) / 400) % 7;
		*/

		ImGui::Text(XorStr("%4d/%2d/%2d %2d:%2d:%2d %s"),
			timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
			timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, GetWeakName(timeInfo.tm_wday).c_str());

		if (g_tpPlayingTimer > 0)
			ImGui::Text(XorStr(u8"游戏时间：%s丨在线时间：%s"), GetTimeDuration(t - g_tpGameTimer).c_str(), GetTimeDuration(t - g_tpPlayingTimer).c_str());
		else
			ImGui::Text(XorStr(u8"游戏时间：%s"), GetTimeDuration(t - g_tpGameTimer).c_str());

		ImGui::Separator();
	}


	// ImGui::Checkbox(XorStr("DrawIndexedPrimitive"), &m_bShowStride);
	DrawStrideMenu();

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnMenuDrawing();

	ImVec2 window = ImGui::GetWindowPos();
	g_pConfig->SetValue(mainKeys, XorStr("mainwindow_x"), window.x);
	g_pConfig->SetValue(mainKeys, XorStr("mainwindow_y"), window.y);
	window = ImGui::GetWindowSize();
	g_pConfig->SetValue(mainKeys, XorStr("mainwindow_w"), window.x);
	g_pConfig->SetValue(mainKeys, XorStr("mainwindow_h"), window.y);

	ImGui::PopFont();
	ImGui::End();

	/*
	if (m_bShowStride)
		DrawStrideMenu();
	*/
}

void CBaseMenu::OnDrawIndexedPrimitive(IDirect3DDevice9 * device, D3DPRIMITIVETYPE type,
	INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount)
{
	UINT offsetByte, stride;
	static IDirect3DVertexBuffer9* stream = nullptr;
	device->GetStreamSource(0, &stream, &offsetByte, &stride);

	for (auto item : g_pBaseMenu->m_vStride)
	{
		if (item.disabled || item.stride != stride)
			continue;

		if (item.vertices > 0 && item.vertices != numVertices)
			continue;

		if (item.primitive > 0 && item.vertices != primitiveCount)
			continue;
		
		static DWORD oldZEnable;
		device->GetRenderState(D3DRS_ZENABLE, &oldZEnable);
		device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);

		/*
		g_pDirextXHook->oDrawIndexedPrimitive(device, type, baseIndex,
			minIndex, numVertices, startIndex, primitiveCount);
		*/

		g_pDirextXHook->oDrawIndexedPrimitive(device, type, baseIndex, minIndex,
			numVertices, startIndex, primitiveCount);

		device->SetRenderState(D3DRS_ZENABLE, oldZEnable);
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		break;
	}
}

void CBaseMenu::DrawStrideMenu()
{
	/*
	if (!m_bShowStride)
		return;
	
	if (!ImGui::Begin(XorStr("DrawIndexedPrimitive Setting"), &m_bShowStride, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}
	*/

	if (!ImGui::TreeNode(XorStr("DrawIndexedPrimitive")))
		return;

	static char buffer[255]{ '\0' }, rename[255]{ '\0' };
	
	if (ImGui::Button(XorStr("Add")))
	{
		if (buffer[0] != '\0')
		{
			for (StrideObject& i : m_vStride)
			{
				if (i.title == buffer)
					goto dnot_push_new;
			}
			
			m_vStride.emplace_back(buffer);
		}

	dnot_push_new:
		buffer[0] = '\0';
	}

	ImGui::SameLine();
	if (ImGui::Button(XorStr("AddEx")))
	{
		ImGui::OpenPopup(XorStr("Add Items"));
		if (ImGui::BeginPopupModal(XorStr("Add Items")))
		{
			ImGui::InputText(XorStr("Name"), buffer, 255);

			static int stride = 32, vertices = 1, primitive = 1;
			ImGui::DragInt(XorStr("Stride"), &stride, 1.0f, 0, 255);
			ImGui::DragInt(XorStr("Vertices"), &vertices, 1.0f, 0);
			ImGui::DragInt(XorStr("Primitive"), &primitive, 1.0f, 0);

			static float color[4];
			ImGui::ColorEdit4(XorStr("Color"), color);

			ImGui::Separator();

			if (ImGui::Button(XorStr("OK")) && buffer[0] != '\0')
			{
				m_vStride.emplace_back(buffer, stride, vertices, primitive, color);
				ImGui::CloseCurrentPopup();
				buffer[0] = '\0';
			}

			ImGui::SameLine();
			if (ImGui::Button(XorStr("Cancel")))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(XorStr("Remove")))
	{
		for (auto i = m_vStride.begin(); i != m_vStride.end(); )
		{
			if (i->selected)
				i = m_vStride.erase(i);
			else
				++i;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(XorStr("Disable/Enable")))
	{
		for (auto& i : m_vStride)
		{
			if (i.selected)
				i.disabled = !i.disabled;
		}
	}

	if (ImGui::InputText(XorStr("Name"), buffer, 255, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (buffer[0] != '\0')
		{
			for (StrideObject& i : m_vStride)
			{
				if (i.title == buffer)
					goto dnot_push_new2;
			}

			m_vStride.emplace_back(buffer);
		}

	dnot_push_new2:
		buffer[0] = '\0';
	}
	ImGui::Separator();

	int state = 0;
	static StrideObject clipboard("");

	for(auto i = m_vStride.begin(); i != m_vStride.end(); )
	{
		bool opening = ImGui::TreeNodeEx((i->disabled ? i->title + XorStr(" - disabled") : i->title).c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow|ImGuiTreeNodeFlags_OpenOnDoubleClick|(i->selected ? ImGuiTreeNodeFlags_Selected : 0));
		
		state = 0;
		if (ImGui::IsItemClicked())
			i->selected = !i->selected;

		if (opening)
		{
			// 右键菜单
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::Selectable(XorStr("delete")))
					state = 1;
				else if (ImGui::Selectable(XorStr("reset")))
					state = 2;
				else if (ImGui::Selectable(XorStr("toggle")))
					state = 3;
				else if (ImGui::Selectable(XorStr("copy")))
					state = 4;
				else if (ImGui::Selectable(XorStr("paste")))
					state = 5;
				else if (ImGui::Selectable(XorStr("copynew")))
					state = 6;

				strcpy_s(rename, i->title.c_str());
				if (ImGui::InputText(XorStr("Name"), rename, 255, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					for (StrideObject& it : m_vStride)
					{
						if (it.title == rename)
							goto dnot_change_name;
					}

					if (rename[0] != '\0')
						i->title = rename;
				}

			dnot_change_name:
				ImGui::EndPopup();
				rename[0] = '\0';
			}
			
			ImGui::DragInt(XorStr("Stride"), &(i->stride), 1.0f, 0, 255);
			ImGui::DragInt(XorStr("Vertices"), &(i->vertices), 1.0f, 0);
			ImGui::DragInt(XorStr("Primitive"), &(i->primitive), 1.0f, 0);
			ImGui::ColorEdit4(XorStr("Color"), i->color);

			ImGui::TreePop();
		}

		ptrdiff_t diff;

		switch (state)
		{
		case 1:
			i = m_vStride.erase(i);
			break;

		case 2:
			i->disabled = false;
			i->stride = 32;
			i->primitive = i->vertices = 0;
			i->color[0] = i->color[1] = i->color[2] = i->color[3] = 1.0f;
			++i;
			break;

		case 3:
			i->disabled = !i->disabled;
			++i;
			break;

		case 4:
			clipboard.stride = i->stride;
			clipboard.vertices = i->vertices;
			clipboard.primitive = i->primitive;
			clipboard.color[0] = i->color[0];
			clipboard.color[1] = i->color[1];
			clipboard.color[2] = i->color[2];
			clipboard.color[3] = i->color[3];
			++i;
			break;

		case 5:
			i->stride = clipboard.stride;
			i->vertices = clipboard.vertices;
			i->primitive = clipboard.primitive;
			i->color[0] = clipboard.color[0];
			i->color[1] = clipboard.color[1];
			i->color[2] = clipboard.color[2];
			i->color[3] = clipboard.color[3];
			++i;
			break;

		case 6:
			diff = std::distance(i, m_vStride.end());
			
			clipboard.stride = i->stride;
			clipboard.vertices = i->vertices;
			clipboard.primitive = i->primitive;
			clipboard.color[0] = i->color[0];
			clipboard.color[1] = i->color[1];
			clipboard.color[2] = i->color[2];
			clipboard.color[3] = i->color[3];
			clipboard.title = i->title;

		get_new_name:
			clipboard.title += '_';
			for (StrideObject& it : m_vStride)
			{
				if (it.title == clipboard.title)
					goto get_new_name;
			}

			m_vStride.push_back(StrideObject(clipboard));

			i = m_vStride.begin() + diff;
			break;

		default:
			++i;
		}
	}

	// ImGui::End();
	ImGui::TreePop();
}

CBaseMenu::StrideObject::StrideObject(const std::string & title, int stride, int vertices, int primitive) :
	title(title), stride(stride), vertices(vertices), primitive(primitive), disabled(false),
	color{1.0f, 1.0f, 1.0f, 1.0f}, selected(false)
{
}

CBaseMenu::StrideObject::StrideObject(const std::string & title, int stride, int vertices, int primitive,
	float color[4]) : title(title), stride(stride), vertices(vertices), primitive(primitive),
	disabled(false), color{ color[0], color[1], color[2], color[3] }, selected(false)
{
}
