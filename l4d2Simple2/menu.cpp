#include <sstream>
#include "menu.h"
#include "xorstr.h"
#include "utils.h"
#include "dx9hook.h"
#include "../imgui/examples/directx9_example/imgui_impl_dx9.h"
#include "../hl2sdk/hook.h"

std::unique_ptr<CBaseMenu> g_pBaseMenu;

void _OnMenuStateChanged(bool visible)
{
	g_pInterface->Surface->SetCursorAlwaysVisible(visible);
}

void CBaseMenu::Init()
{
	// g_pDirextXHook->m_vfnDrawIndexedPrimitive.emplace_back(Hooked_DrawIndexedPrimitive);
	g_pDirextXHook->AddHook_DrawIndexedPrimitive(Hooked_DrawIndexedPrimitive);
}

void CBaseMenu::OnPresent()
{
	if (!g_bHasShowMenu)
		return;

#ifdef _DEBUG
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// 1. Show a simple window.
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");                           // Some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float as a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats as a color
		if (ImGui::Button("Demo Window"))                       // Use buttons to toggle our bools. We could use Checkbox() as well.
			show_demo_window ^= 1;
		if (ImGui::Button("Another Window"))
			show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name the window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		ImGui::End();
	}

	// 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow().
	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowDemoWindow(&show_demo_window);
	}
#endif

	if (!ImGui::Begin(XorStr("l4d2Simple | by zonde306"), &g_bHasShowMenu))
	{
		ImGui::End();
		_OnMenuStateChanged(false);
		return;
	}

	// ImGui::Checkbox(XorStr("DrawIndexedPrimitive"), &m_bShowStride);
	DrawStrideMenu();

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnMenuDrawing();

	ImGui::End();

	/*
	if (m_bShowStride)
		DrawStrideMenu();
	*/
}

HRESULT WINAPI CBaseMenu::Hooked_DrawIndexedPrimitive(IDirect3DDevice9 * device, D3DPRIMITIVETYPE type,
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
		g_pDirextXHook->m_pfnDrawIndexedPrimitive(device, type, baseIndex,
			minIndex, numVertices, startIndex, primitiveCount);
		*/

		g_pDirextXHook->CallOriginal_DrawIndexedPrimitive(type, baseIndex, minIndex,
			numVertices, startIndex, primitiveCount);

		device->SetRenderState(D3DRS_ZENABLE, oldZEnable);
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

		break;
	}

	return S_OK;
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
