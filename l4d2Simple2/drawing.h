#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <imgui.h>
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <D3DFont.h>

#include "vector.h"

class CDrawing
{
public:
	CDrawing();
	~CDrawing();

	void Init(IDirect3DDevice9* device, int fontSize = 16);
	void OnLostDevice();
	void OnResetDevice();
	void OnBeginEndScene();
	void OnFinishEndScene();
	void OnBeginPresent();
	void OnFinishPresent();
	void OnGameFrame();

	bool WorldToScreen(const Vector& origin, Vector& output);

protected:
	void ReleaseObjects();
	void CreateObjects();
	void DrawQueueObject();

	static std::string FindFonts(const std::string& name);

public:
	enum D3DColor
	{
		WHITE = D3DCOLOR_ARGB(255, 255, 255, 255),		
		BLACK = D3DCOLOR_ARGB(255, 0, 0, 0),			
		RED = D3DCOLOR_ARGB(255, 255, 0, 0),			
		GREEN = D3DCOLOR_ARGB(255, 0, 255, 0),			
		LAWNGREEN = D3DCOLOR_ARGB(255, 128, 255, 0),	
		BLUE = D3DCOLOR_ARGB(255, 0, 128, 192),			
		DEEPSKYBLUE = D3DCOLOR_ARGB(255, 0, 0, 255),	
		SKYBLUE = D3DCOLOR_ARGB(255, 0, 255, 255),		
		YELLOW = D3DCOLOR_ARGB(255, 255, 255, 0),		
		ORANGE = D3DCOLOR_ARGB(255, 255, 128, 0),		
		DARKORANGE = D3DCOLOR_ARGB(255, 255, 100, 0),	
		PURPLE = D3DCOLOR_ARGB(255, 128, 0, 255),		
		CYAN = D3DCOLOR_ARGB(255, 0, 255, 64),			
		PINK = D3DCOLOR_ARGB(255, 255, 128, 255),		
		GRAY = D3DCOLOR_ARGB(255, 128, 128, 128),		
		DARKGRAY = D3DCOLOR_ARGB(255, 73, 73, 73),		
		DARKERGRAY = D3DCOLOR_ARGB(255, 192, 192, 192)	
	};

public:	
	void PrintInfo(D3DCOLOR color, const char* text, ...);

public:	
	void RenderPoint(int x, int y, D3DCOLOR color = D3DColor::WHITE);
	void RenderLine(int x1, int y1, int x2, int y2, D3DCOLOR color = D3DColor::WHITE);
	void RenderRect(int x, int y, int w, int h, D3DCOLOR color = D3DColor::WHITE);
	void RenderCircle(int x, int y, int r, D3DCOLOR color = D3DColor::WHITE, int resolution = 32);
	void RenderRectFilled(int x, int y, int w, int h, D3DCOLOR color = D3DColor::WHITE);
	void RenderCircleFilled(int x, int y, int r, D3DCOLOR color = D3DColor::WHITE, int resolution = 32);
	void RenderCorner(int x, int y, int w, int h, D3DCOLOR color = D3DColor::WHITE, int length = 0);
	void RenderText(int x, int y, D3DCOLOR color, bool centered, const char* text, ...);
	void RenderText(int x, int y, D3DCOLOR color, bool centered, const wchar_t* text, ...);
	void RenderText2(int x, int y, D3DCOLOR color, bool centered, const char* text, ...);

public:	
	void DrawPoint(int x, int y, D3DCOLOR color = D3DColor::WHITE);
	void DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR color = D3DColor::WHITE);
	void DrawRect(int x, int y, int w, int h, D3DCOLOR color = D3DColor::WHITE);
	void DrawCircle(int x, int y, int r, D3DCOLOR color = D3DColor::WHITE, int resolution = 32);
	void DrawRectFilled(int x, int y, int w, int h, D3DCOLOR color = D3DColor::WHITE);
	void DrawCircleFilled(int x, int y, int r, D3DCOLOR color = D3DColor::WHITE, int resolution = 32);
	void DrawCorner(int x, int y, int w, int h, D3DCOLOR color = D3DColor::WHITE, int length = 0);
	void DrawText(int x, int y, D3DCOLOR color, bool centered, const char* text, ...);

	std::pair<int, int> GetDrawTextSize(const char* text, ...);

protected:
	struct D3DVertex
	{
		D3DVertex();
		D3DVertex(float _x, float _y, float _z, D3DCOLOR _color);
		D3DVertex(float _x, float _y, float _z, float _w, D3DCOLOR _color);
		
		float x;
		float y;
		float z;
		float rhw;
		D3DCOLOR color;
	};

	struct TopStringList
	{
		TopStringList(D3DCOLOR color, const std::string& text, int second = 3);

		std::string text;
		D3DCOLOR color;
		time_t destoryTime;
	};

	struct DelayDrawList
	{
		DelayDrawList(D3DPRIMITIVETYPE type, size_t count, const std::vector<D3DVertex>& vertex);
		DelayDrawList(D3DVertex* vertex, size_t len, D3DPRIMITIVETYPE type);

		std::vector<D3DVertex> vertex;
		D3DPRIMITIVETYPE type;
		size_t vertexCount;
	};

	struct DelayStringList
	{
		DelayStringList(float _x, float _y, const std::string& _text, D3DCOLOR _color,
			DWORD _flags = 0, D3DCOLOR bgcolor = 0xFFFFFFFF);

		float x, y;
		DWORD flags;
		D3DCOLOR color, background;
		std::string text;
	};

	struct DelayStringListWide
	{
		DelayStringListWide(float _x, float _y, const std::wstring& _text, D3DCOLOR _color,
			DWORD _flags = 0, D3DCOLOR bgcolor = 0xFFFFFFFF);

		float x, y;
		DWORD flags;
		D3DCOLOR color, background;
		std::wstring text;
	};

private:
	std::vector<DelayDrawList> m_vDrawList;
	std::vector<DelayStringList> m_vStringList;
	std::vector<DelayStringListWide> m_vStringListW;
	std::vector<TopStringList> m_vTopStringList;
	std::vector<DelayStringList> m_vSimpleStringList;
	bool m_bTopStringDrawing;

private:
	bool m_bInEndScene, m_bInPresent;
	std::mutex m_hLockEndScene;
	std::mutex m_hLockPresent;
	IDirect3DDevice9* m_pDevice;
	bool m_bIsReady;

public:
	int m_iFontSize, m_iScreenWidth, m_iScreenHeight;

private:
	int m_iFramePerSecond;
	int m_iFrameCount;
	time_t m_tNextUpdateTime;

private:
	IDirect3DStateBlock9* m_pStateBlock;
	ID3DXFont* m_pDefaultFont;
	CD3DFont* m_pFont;
	ID3DXLine* m_pLine;
	ID3DXSprite* m_pTextSprite;

private:
	ImDrawData m_imDrawData;
	ImDrawList* m_imDrawList;

public:
	IDirect3DTexture9* m_imFontTexture;
	ImFontAtlas m_imFonts;
};

extern std::unique_ptr<CDrawing> g_pDrawing;