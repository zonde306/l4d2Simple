#pragma once

#include <imgui.h>
#include <memory>
#include <vector>
#include <functional>
#include <map>

#include "drawing.h"

class CBaseMenu
{
public:
	void Init();
	void OnPresent();

	void OnDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE type,
		INT baseIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primitiveCount);

protected:

	struct StrideObject
	{
		std::string title;
		int stride, vertices, primitive;
		float color[4];
		bool disabled, selected;

		StrideObject(const std::string& title, int stride = 32, int vertices = 0, int primitive = 0);
		StrideObject(const std::string& title, int stride, int vertices, int primitive, float color[4]);
	};

	std::vector<StrideObject> m_vStride;
	bool m_bShowStride;

protected:
	void DrawStrideMenu();
	void DrawMiscMenu();
};

extern std::unique_ptr<CBaseMenu> g_pBaseMenu;