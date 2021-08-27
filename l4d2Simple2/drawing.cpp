﻿#include <sstream>

#include "drawing.h"
#include "dx9hook.h"
#include "utils.h"
#include "xorstr.h"
#include "menu.h"
#include "../imgui/examples/directx9_example/imgui_impl_dx9.h"
#include "../hl2sdk/hook.h"

// Convert D3DCOLOR to ImGui color
#define D3DCOLOR_IMGUI(_clr) ((_clr & 0xFF000000) | ((_clr & 0x00FF0000) >> 16) | (_clr & 0x0000FF00) | ((_clr & 0x000000FF) << 16))

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_F ((float)M_PI)
#endif

std::unique_ptr<CDrawing> g_pDrawing;

extern HWND g_hGameWindow;

class CAutoLock
{
public:
	CAutoLock(std::mutex& mutex) : mutex(mutex), hasLocked(false)
	{
		lock();
	}

	~CAutoLock()
	{
		unlock();
	}

	void lock()
	{
		if (hasLocked)
			return;

		hasLocked = true;
		this->mutex.lock();
	}

	void unlock()
	{
		if (!hasLocked)
			return;

		hasLocked = false;
		this->mutex.unlock();
	}

private:
	std::mutex& mutex;
	bool hasLocked;
};

#define LOCK_PRESENT() CAutoLock __lock_present(m_hLockPresent)
#define LOCK_ENDSCENE() CAutoLock __lock_endscene(m_hLockEndScene)

#define UNLOCK_PRESENT() __lock_present.unlock()
#define UNLOCK_ENDSCENE() __lock_endscene.unlock()

bool CDrawing::WorldToScreen(const Vector& origin, Vector& output)
{
	D3DVIEWPORT9 viewport;
	m_pDevice->GetViewport(&viewport);

	D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix;
	m_pDevice->GetTransform(D3DTS_VIEW, &viewMatrix);
	m_pDevice->GetTransform(D3DTS_PROJECTION, &projectionMatrix);

	D3DXMatrixIdentity(&worldMatrix);
	D3DXVECTOR3 worldPosition(origin.x, origin.y, origin.z), screenPosition;
	D3DXVec3Project(&screenPosition, &worldPosition, &viewport, &projectionMatrix, &viewMatrix, &worldMatrix);

	output.x = screenPosition.x;
	output.y = screenPosition.y;
	output.z = screenPosition.z;

	return (screenPosition.z < 1.0f);
}

void CDrawing::ReleaseObjects()
{
	LOCK_ENDSCENE();

	if (m_pStateBlock)
	{
		m_pStateBlock->Release();
		m_pStateBlock = nullptr;
	}

	if (m_pDefaultFont)
	{
		m_pDefaultFont->Release();
		m_pDefaultFont = nullptr;
	}

	if (m_pLine)
	{
		m_pLine->Release();
		m_pLine = nullptr;
	}

	if (m_pTextSprite)
	{
		m_pTextSprite->Release();
		m_pTextSprite = nullptr;
	}

	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = nullptr;
	}

	UNLOCK_ENDSCENE();

	LOCK_PRESENT();

	if (m_imFontTexture)
	{
		m_imFontTexture->Release();
		m_imFontTexture = nullptr;
	}

	if (m_imDrawList)
	{
		delete m_imDrawList;
		m_imDrawList = nullptr;
	}

	m_imFonts.TexID = NULL;
	m_imFonts.Clear();
	ImGui::GetIO().Fonts->Clear();

	UNLOCK_PRESENT();

	m_bIsReady = false;
	Utils::log(XorStr("CDrawing::ReleaseObjects"));
}

void CDrawing::CreateObjects()
{
	if (m_iFontSize <= 0)
		m_iFontSize = 16;

	std::stringstream ss;

	LOCK_ENDSCENE();

	HRESULT hr = D3D_OK;

	if (FAILED(hr = m_pDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock)))
	{
		ss << XorStr("CreateStateBlock Failed: ");

		if (hr == D3DERR_INVALIDCALL)
			ss << XorStr("The method call is invalid");
		else if (hr == D3DERR_OUTOFVIDEOMEMORY)
			ss << XorStr("Direct3D does not have enough display memory to perform the operation");
		else if (hr == E_OUTOFMEMORY)
			ss << XorStr("Direct3D could not allocate sufficient memory to complete the call");
		else
			ss << XorStr("Unknown Error");

		Utils::logError(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	if (FAILED(hr = D3DXCreateFontA(m_pDevice, m_iFontSize, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		XorStr("Microsoft YaHei Light & Microsoft YaHei UI Light"), &m_pDefaultFont)))
	{
		ss << XorStr("D3DXCreateFont Failed: ");

		if (hr == D3DERR_INVALIDCALL)
			ss << XorStr("The method call is invalid");
		else if (hr == E_OUTOFMEMORY)
			ss << XorStr("Direct3D could not allocate sufficient memory to complete the call");
		else if (hr == D3DXERR_INVALIDDATA)
			ss << XorStr("The data is invalid");
		else
			ss << XorStr("Unknown Error");

		Utils::logError(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	if (FAILED(hr = D3DXCreateLine(m_pDevice, &m_pLine)))
	{
		ss << XorStr("D3DXCreateLine Failed: ");

		if (hr == D3DERR_INVALIDCALL)
			ss << XorStr("The method call is invalid");
		else if (hr == E_OUTOFMEMORY)
			ss << XorStr("Direct3D could not allocate sufficient memory to complete the call");
		else if (hr == D3DXERR_INVALIDDATA)
			ss << XorStr("The data is invalid");
		else
			ss << XorStr("Unknown Error");

		Utils::logError(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	if (FAILED(D3DXCreateSprite(m_pDevice, &m_pTextSprite)))
	{
		ss << XorStr("D3DXCreateLine Failed: ");

		if (hr == D3DERR_INVALIDCALL)
			ss << XorStr("The method call is invalid");
		else if (hr == D3DXERR_INVALIDDATA)
			ss << XorStr("The data is invalid");
		else
			ss << XorStr("Unknown Error");

		Utils::logError(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	m_pFont = new CD3DFont(XorStr("Tahoma"), m_iFontSize);
	m_pFont->InitializeDeviceObjects(m_pDevice);
	m_pFont->RestoreDeviceObjects();

	UNLOCK_ENDSCENE();

	D3DVIEWPORT9 viewport;

	m_pDevice->GetViewport(&viewport);
	m_iScreenWidth = viewport.Width;
	m_iScreenHeight = viewport.Height;

	LOCK_PRESENT();

	ImGui::GetIO().DisplaySize.x = static_cast<float>(m_iScreenWidth);
	ImGui::GetIO().DisplaySize.y = static_cast<float>(m_iScreenHeight);

	m_imDrawList = new ImDrawList(ImGui::GetDrawListSharedData());

	char systemPath[MAX_PATH];
	GetWindowsDirectoryA(systemPath, MAX_PATH);

	std::string fontPath(systemPath);
	fontPath += XorStr("\\Fonts\\msyhl.ttc");

	m_imFonts.AddFontFromFileTTF(fontPath.c_str(), static_cast<float>(m_iFontSize), nullptr, m_imFonts.GetGlyphRangesChinese());
	ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.data(), static_cast<float>(m_iFontSize), nullptr, m_imFonts.GetGlyphRangesChinese());

	uint8_t* pixel_data;
	int width, height, bytes_per_pixel;

	m_imFonts.GetTexDataAsRGBA32(&pixel_data, &width, &height, &bytes_per_pixel);

	if (FAILED(hr = m_pDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_imFontTexture, nullptr)))
	{
		ss << XorStr("IDirect3DDevice9::CreateTexture Failed: ");

		if (hr == D3DERR_INVALIDCALL)
			ss << XorStr("The method call is invalid");
		else if (hr == D3DERR_OUTOFVIDEOMEMORY)
			ss << XorStr("Direct3D does not have enough display memory to perform the operation");
		else if (hr == E_OUTOFMEMORY)
			ss << XorStr("Direct3D could not allocate sufficient memory to complete the call");
		else
			ss << XorStr("Unknown Error");

		Utils::logError(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	D3DLOCKED_RECT textureLock;

	if (FAILED(hr = m_imFontTexture->LockRect(0, &textureLock, nullptr, 0)))
	{
		ss << XorStr("IDirect3DTexture9::LockRect Failed: ");

		if (hr == D3DERR_INVALIDCALL)
			ss << XorStr("The method call is invalid");
		else
			ss << XorStr("Unknown Error");

		Utils::logError(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	for (int y = 0; y < height; y++)
	{
		memcpy_s((uint8_t*)textureLock.pBits + textureLock.Pitch * y, (width * bytes_per_pixel),
			pixel_data + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
	}

	m_imFontTexture->UnlockRect(0);
	m_imFonts.TexID = m_imFontTexture;

	UNLOCK_PRESENT();

	m_bIsReady = true;

	Utils::log(XorStr("CDrawing::CreateObjects"));
	ss << XorStr("Screen - Width: ") << m_iScreenWidth << XorStr(", Height: ") << m_iScreenHeight;
	Utils::log(ss.str().c_str());
}

void CDrawing::DrawQueueObject()
{
	{
		m_pFont->BeginDrawing();
		m_bTopStringDrawing = true;

#ifdef _DEBUG
		{
			char buffer[12];
			_itoa_s(m_iFramePerSecond, buffer, 10);
			D3DCOLOR color = RED;

			if (m_iFramePerSecond >= 60)
				color = LAWNGREEN;
			else if (m_iFramePerSecond >= 30)
				color = ORANGE;

			m_pFont->DrawText(m_iScreenWidth - 50.0f, 10.0f, color, buffer);
		}
#endif

		if (!m_vSimpleStringList.empty())
		{
			for (DelayStringList& i : m_vSimpleStringList)
			{
				m_pFont->DrawText(i.x, i.y, i.color, i.text.c_str(), i.flags, i.background);
			}
		}

		if (!m_vTopStringList.empty())
		{
			int drawQueue = -1;
			time_t currentTime = time(nullptr);
			auto i = m_vTopStringList.begin();

			while (i != m_vTopStringList.end())
			{
				m_pFont->DrawText(10.0f, m_iFontSize * ++drawQueue + 12.0f, i->color, i->text.c_str());

				if (i->destoryTime <= currentTime)
					i = m_vTopStringList.erase(i);
				else
					++i;
			}
		}

		m_pFont->EndDrawing();
		m_bTopStringDrawing = false;
	}

	if (!m_vDrawList.empty())
	{
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

		for (DelayDrawList& i : m_vDrawList)
		{
			if (i.vertexCount <= 0 || i.vertex.empty())
				continue;

			m_pDevice->DrawPrimitiveUP(i.type, i.vertexCount,
				&(i.vertex[0]), sizeof(D3DVertex));
		}
	}

	if (!m_vStringList.empty() || !m_vStringListW.empty())
	{
		RECT rect;
		m_pTextSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);

		for (DelayStringList& i : m_vStringList)
		{
			if (i.text.empty())
				continue;

			if (i.flags & D3DFONT_CENTERED)
			{
				rect = { 0, 0, 0, 0 };

				m_pDefaultFont->DrawTextA(m_pTextSprite, i.text.c_str(), i.text.length(), &rect,
					DT_NOCLIP | DT_CALCRECT, i.color);

				rect = { ((LONG)i.x) - rect.right / 2, ((LONG)i.y), 0, 0 };
			}
			else
			{
				rect = { ((LONG)i.x), ((LONG)i.y), 1000, 100 };
			}

			m_pDefaultFont->DrawTextA(m_pTextSprite, i.text.c_str(), i.text.length(), &rect,
				DT_TOP | DT_LEFT | DT_NOCLIP, i.color);
		}

		for (DelayStringListWide& i : m_vStringListW)
		{
			if (i.text.empty())
				continue;

			if (i.flags & D3DFONT_CENTERED)
			{
				rect = { 0, 0, 0, 0 };

				m_pDefaultFont->DrawTextW(m_pTextSprite, i.text.c_str(), i.text.length(), &rect,
					DT_NOCLIP | DT_CALCRECT, i.color);

				rect = { ((LONG)i.x) - rect.right / 2, ((LONG)i.y), 0, 0 };
			}
			else
			{
				rect = { ((LONG)i.x), ((LONG)i.y), 1000, 100 };
			}

			m_pDefaultFont->DrawTextW(m_pTextSprite, i.text.c_str(), i.text.length(), &rect,
				DT_TOP | DT_LEFT | DT_NOCLIP, i.color);
		}

		m_pTextSprite->End();
	}
}

std::string CDrawing::FindFonts(const std::string& name)
{
	char buffer[MAX_PATH];
	HKEY registryKey;

	GetWindowsDirectoryA(buffer, MAX_PATH);
	std::string fontsFolder = buffer + std::string(XorStr("\\Fonts\\"));

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		XorStr("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"), 0, KEY_READ, &registryKey))
	{
		return "";
	}

	uint32_t valueIndex = 0;
	char valueName[MAX_PATH];
	uint8_t valueData[MAX_PATH];
	std::wstring wsFontFile;

	for (;;)
	{
		uint32_t valueNameSize = MAX_PATH;
		uint32_t valueDataSize = MAX_PATH;
		uint32_t valueType;

		auto error = RegEnumValueA(registryKey, valueIndex, valueName, reinterpret_cast<DWORD*>(&valueNameSize), 0,
			reinterpret_cast<DWORD*>(&valueType), valueData, reinterpret_cast<DWORD*>(&valueDataSize));

		valueIndex++;

		if (error == ERROR_NO_MORE_ITEMS)
		{
			RegCloseKey(registryKey);
			break;
		}

		if (error || valueType != REG_SZ)
		{
			continue;
		}

		if (_strnicmp(name.data(), valueName, name.size()) == 0)
		{
			RegCloseKey(registryKey);
			return (fontsFolder + std::string((char*)valueData, valueDataSize));
		}
	}

	return "";
}

CDrawing::CDrawing() : m_bInEndScene(false), m_bInPresent(false), m_pDevice(nullptr),
    m_pStateBlock(nullptr), m_pDefaultFont(nullptr), m_pFont(nullptr), m_pLine(nullptr),
	m_pTextSprite(nullptr), m_imDrawList(nullptr), m_imFontTexture(nullptr), m_iFramePerSecond(0),
	m_iFrameCount(0), m_tNextUpdateTime(0), m_bTopStringDrawing(false), m_iFontSize(16) { }

CDrawing::~CDrawing()
{
	ReleaseObjects();
	ImGui_ImplDX9_Shutdown();

	m_imFonts.Clear();
	ImGui::GetIO().Fonts->Clear();
}

void CDrawing::Init(IDirect3DDevice9* device, int fontSize)
{
	m_pDevice = device;
	m_iFontSize = fontSize;

	ImGui_ImplDX9_Init(g_hGameWindow, device);
	ImGui::StyleColorsDark();

	CreateObjects();

	g_pfnOldWndProcHandler = (WNDPROC)SetWindowLongPtrA(g_hGameWindow, GWL_WNDPROC, (LONG_PTR)ImGui_ImplWin32_WndProcHandler);

	Utils::log(XorStr("CDrawing Initialization..."));
}

void CDrawing::OnLostDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ReleaseObjects();

	LOCK_ENDSCENE();

	m_vDrawList.clear();
	m_vStringList.clear();
	m_vStringListW.clear();
	m_vTopStringList.clear();
	m_vSimpleStringList.clear();
	m_bTopStringDrawing = false;

	UNLOCK_ENDSCENE();
}

void CDrawing::OnResetDevice()
{
	CreateObjects();
	ImGui_ImplDX9_CreateDeviceObjects();

	LOCK_ENDSCENE();

	UNLOCK_ENDSCENE();
}

void CDrawing::OnBeginEndScene()
{
	LOCK_ENDSCENE();

	m_pStateBlock->Capture();

	m_pDevice->SetTexture(0, nullptr);
	m_pDevice->SetPixelShader(nullptr);
	m_pDevice->SetVertexShader(nullptr);
	m_pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	m_pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, false);
	m_pDevice->SetRenderState(D3DRS_FOGENABLE, false);
	m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
	m_pDevice->SetRenderState(D3DRS_STENCILENABLE, false);
	m_pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, true);
	m_pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, true);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

	m_bInEndScene = true;

	UNLOCK_ENDSCENE();
}

void CDrawing::OnFinishEndScene()
{
	LOCK_ENDSCENE();

	try
	{
		DrawQueueObject();
	}
	catch (...)
	{
		Utils::logError(XorStr("DrawQueueObject Failed."));
	}

	m_bInEndScene = false;

	if (m_bTopStringDrawing)
	{
		m_vTopStringList.clear();
		m_bTopStringDrawing = false;
	}

	m_vDrawList.clear();
	m_vStringList.clear();
	m_vStringListW.clear();
	m_vSimpleStringList.clear();

	m_pStateBlock->Apply();

	UNLOCK_ENDSCENE();
}

void CDrawing::OnBeginPresent()
{
	LOCK_PRESENT();

	ImGui_ImplDX9_NewFrame();
	m_imDrawData.Valid = false;

	UNLOCK_PRESENT();

	g_pBaseMenu->OnPresent();

	for (const auto& inst : g_pClientHook->_GameHook)
		inst->OnScreenDrawing();
}

void CDrawing::OnFinishPresent()
{
	LOCK_PRESENT();

	if (!m_imDrawList->VtxBuffer.empty())
	{
		m_imDrawData.Valid = true;
		m_imDrawData.CmdLists = &m_imDrawList;
		m_imDrawData.CmdListsCount = 1;
		m_imDrawData.TotalIdxCount = m_imDrawList->IdxBuffer.Size;
		m_imDrawData.TotalVtxCount = m_imDrawList->VtxBuffer.Size;
	}
	else
	{
		m_imDrawData.Valid = false;
	}

	ImGui_ImplDX9_RenderDrawLists(&m_imDrawData);
	ImGui::Render();

	m_imDrawList->Clear();
	m_imDrawList->PushClipRectFullScreen();

	UNLOCK_PRESENT();
}

void CDrawing::OnGameFrame()
{
	time_t curtime = time(nullptr);

	m_iFrameCount += 1;

	if (m_tNextUpdateTime <= curtime)
	{
		m_tNextUpdateTime = curtime + 1;
		m_iFramePerSecond = m_iFrameCount;
		m_iFrameCount = 0;
	}
}

CDrawing::D3DVertex::D3DVertex() : x(0.0f), y(0.0f), z(0.0f), rhw(0.0f), color(CDrawing::WHITE) { }

CDrawing::D3DVertex::D3DVertex(float _x, float _y, float _z, D3DCOLOR _color) : 
	x(_x), y(_y), z(_z), rhw(0.0f), color(_color) { }

CDrawing::D3DVertex::D3DVertex(float _x, float _y, float _z, float _w, D3DCOLOR _color) :
	x(_x), y(_y), z(_z), rhw(_w), color(_color) { }

CDrawing::TopStringList::TopStringList(D3DCOLOR color, const std::string & text, int second) : text(text), color(color)
{
	destoryTime = time(nullptr) + second;
}

CDrawing::DelayDrawList::DelayDrawList(D3DPRIMITIVETYPE type, size_t count, const std::vector<D3DVertex>& vertex) : type(type)
{
	this->vertex = std::move(vertex);
	vertexCount = count;

	if (vertexCount > this->vertex.size())
		throw std::runtime_error(XorStr("Array Size Invalid"));
}

CDrawing::DelayDrawList::DelayDrawList(D3DVertex* vertex, size_t len, D3DPRIMITIVETYPE type) : type(type), vertexCount(len)
{
	for (size_t i = 0; i < len; ++i)
		this->vertex.push_back(vertex[i]);
}

CDrawing::DelayStringList::DelayStringList(float _x, float _y, const std::string & _text, D3DCOLOR _color, DWORD _flags, D3DCOLOR bgcolor) :
	x(_x), y(_y), text(_text), color(_color), flags(_flags), background(bgcolor) { }

CDrawing::DelayStringListWide::DelayStringListWide(float _x, float _y, const std::wstring & _text, D3DCOLOR _color, DWORD _flags, D3DCOLOR bgcolor) :
	x(_x), y(_y), text(_text), color(_color), flags(_flags), background(bgcolor) { }

void CDrawing::PrintInfo(D3DCOLOR color, const char* text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	LOCK_ENDSCENE();

	m_vTopStringList.emplace_back(color, buffer);

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderPoint(int x, int y, D3DCOLOR color)
{
	LOCK_ENDSCENE();

	m_vDrawList.emplace_back(D3DPT_TRIANGLEFAN, 1, std::vector<D3DVertex>{
		D3DVertex((float)x, (float)y, 1.0f, color),
			D3DVertex((float)x + 1.0f, (float)y + 1.0f, 1.0f, color)
	});

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderLine(int x1, int y1, int x2, int y2, D3DCOLOR color)
{
	LOCK_ENDSCENE();

	m_vDrawList.emplace_back(D3DPT_LINELIST, 1, std::vector<D3DVertex>{
		D3DVertex((float)x1, (float)y1, 1.0f, color),
			D3DVertex((float)x2, (float)y2, 1.0f, color)
	});

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderRect(int x, int y, int w, int h, D3DCOLOR color)
{
	LOCK_ENDSCENE();

	m_vDrawList.emplace_back(D3DPT_LINESTRIP, 4, std::vector<D3DVertex>{
		D3DVertex((float)x, (float)y, 1.0f, color),
			D3DVertex((float)(x + w), (float)y, 1.0f, color),
			D3DVertex((float)(x + w), (float)(y + h), 1.0f, color),
			D3DVertex((float)x, (float)(y + h), 1.0f, color),
			D3DVertex((float)x, (float)y, 1.0f, color)
	});

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderCircle(int x, int y, int r, D3DCOLOR color, int resolution)
{
	float curAngle;
	float angle = (float)((2.0f * M_PI_F) / resolution);

	std::vector<D3DVertex> vertices;

	for (int i = 0; i <= resolution; ++i)
	{
		curAngle = i * angle;
		vertices.emplace_back(x + r * cos(curAngle), y - r * sin(curAngle), 0.0f, color);
	}

	LOCK_ENDSCENE();

	m_vDrawList.emplace_back(D3DPT_LINESTRIP, resolution, std::move(vertices));

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderRectFilled(int x, int y, int w, int h, D3DCOLOR color)
{
	LOCK_ENDSCENE();

	m_vDrawList.emplace_back(D3DPT_TRIANGLEFAN, 4, std::vector<D3DVertex>{
		D3DVertex((float)x, (float)y, 1.0f, color),
			D3DVertex((float)(x + w), (float)y, 1.0f, color),
			D3DVertex((float)(x + w), (float)(y + h), 1.0f, color),
			D3DVertex((float)x, (float)(y + h), 1.0f, color),
			D3DVertex((float)x, (float)y, 1.0f, color)
	});

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderCircleFilled(int x, int y, int r, D3DCOLOR color, int resolution)
{
	float curAngle;
	float angle = (float)((2.0f * M_PI_F) / resolution);

	std::vector<D3DVertex> vertices;

	for (int i = 0; i <= resolution; ++i)
	{
		curAngle = i * angle;
		vertices.emplace_back(x + r * cos(curAngle), y - r * sin(curAngle), 0.0f, color);
	}

	LOCK_ENDSCENE();

	m_vDrawList.emplace_back(D3DPT_TRIANGLEFAN, resolution, std::move(vertices));

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderCorner(int x, int y, int w, int h, D3DCOLOR color, int length)
{
	if (length == 0)
	{
		if (w == 0)
			return;

		length = w / 3;
	}

	// Upper left
	RenderLine(x, y, x + length, y, color);
	RenderLine(x, y, x, y - length, color);

	// Upper right
	RenderLine(x + w, y, x + w - length, y, color);
	RenderLine(x + w, y, x + w, y - length, color);

	// Lower left
	RenderLine(x, y + h, x + length, y + h, color);
	RenderLine(x, y + h, x, y + h + length, color);

	// Lower right
	RenderLine(x + w, y + h, x + w - length, y + h, color);
	RenderLine(x + w, y + h, x + w, y + h + length, color);
}

void CDrawing::RenderText(int x, int y, D3DCOLOR color, bool centered, const char* text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	LOCK_ENDSCENE();

	m_vStringList.emplace_back(static_cast<float>(x), static_cast<float>(y),
		buffer, color, (centered ? D3DFONT_CENTERED : 0));

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderText(int x, int y, D3DCOLOR color, bool centered, const wchar_t* text, ...)
{
	va_list ap;
	va_start(ap, text);

	wchar_t buffer[1024];
	vswprintf_s(buffer, text, ap);

	va_end(ap);

	LOCK_ENDSCENE();

	m_vStringListW.emplace_back(static_cast<float>(x), static_cast<float>(y),
		buffer, color, (centered ? D3DFONT_CENTERED : 0));

	UNLOCK_ENDSCENE();
}

void CDrawing::RenderText2(int x, int y, D3DCOLOR color, bool centered, const char* text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	LOCK_ENDSCENE();

	m_vSimpleStringList.emplace_back(static_cast<float>(x), static_cast<float>(y),
		text, color, (centered ? D3DFONT_CENTERED : 0));

	UNLOCK_ENDSCENE();
}

void CDrawing::DrawPoint(int x, int y, D3DCOLOR color)
{
	LOCK_PRESENT();

	m_imDrawList->AddRectFilled(ImVec2(static_cast<float>(x), static_cast<float>(y)),
		ImVec2(static_cast<float>(x + 1), static_cast<float>(y + 1)), D3DCOLOR_IMGUI(color));

	UNLOCK_PRESENT();
}

void CDrawing::DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR color)
{
	LOCK_PRESENT();

	m_imDrawList->AddLine(ImVec2(static_cast<float>(x1), static_cast<float>(y1)),
		ImVec2(static_cast<float>(x2), static_cast<float>(y2)), D3DCOLOR_IMGUI(color));

	UNLOCK_PRESENT();
}

void CDrawing::DrawRect(int x, int y, int w, int h, D3DCOLOR color)
{
	LOCK_PRESENT();

	m_imDrawList->AddRect(ImVec2(static_cast<float>(x), static_cast<float>(y)),
		ImVec2(static_cast<float>(x + w), static_cast<float>(y + h)), D3DCOLOR_IMGUI(color));

	UNLOCK_PRESENT();
}

void CDrawing::DrawCircle(int x, int y, int r, D3DCOLOR color, int resolution)
{
	LOCK_PRESENT();

	m_imDrawList->AddCircle(ImVec2(static_cast<float>(x), static_cast<float>(y)),
		static_cast<float>(r), D3DCOLOR_IMGUI(color), resolution);

	UNLOCK_PRESENT();
}

void CDrawing::DrawRectFilled(int x, int y, int w, int h, D3DCOLOR color)
{
	LOCK_PRESENT();

	m_imDrawList->AddRectFilled(ImVec2(static_cast<float>(x), static_cast<float>(y)),
		ImVec2(static_cast<float>(x + w), static_cast<float>(y + h)), D3DCOLOR_IMGUI(color));

	UNLOCK_PRESENT();
}

void CDrawing::DrawCircleFilled(int x, int y, int r, D3DCOLOR color, int resolution)
{
	LOCK_PRESENT();

	m_imDrawList->AddCircleFilled(ImVec2(static_cast<float>(x), static_cast<float>(y)),
		static_cast<float>(r), D3DCOLOR_IMGUI(color), resolution);

	UNLOCK_PRESENT();
}

void CDrawing::DrawCorner(int x, int y, int w, int h, D3DCOLOR color, int length)
{
	if (length == 0)
	{
		if (w == 0)
			return;

		length = (w < h ? w / 3 : h / 3);
	}

	if (w < 0)
		w = -w;
	if (h < 0)
		h = -h;

	// Upper left
	DrawLine(x, y, x + length, y, color);
	DrawLine(x, y, x, y + length, color);

	// Upper right
	DrawLine(x + w, y, x - length, y, color);
	DrawLine(x + w, y, y + length, color);

	// Lower left
	DrawLine(x, y + h, x + length, color);
	DrawLine(x, y + h, x, y - length, color);

	// Lower right
	DrawLine(x + w, y + h, x - length, y + h, color);
	DrawLine(x + w, y + h, x + w, y - length, color);
}

void CDrawing::DrawText(int x, int y, D3DCOLOR color, bool centered, const char* text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	ImFont* font = m_imFonts.Fonts.back();
	ImVec2 textSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, buffer);

	if (centered)
	{
		x -= static_cast<int>(textSize.x / 2);
		y -= static_cast<int>(textSize.y / 2);
	}

	LOCK_PRESENT();

	m_imDrawList->PushTextureID(m_imFontTexture);

	if (m_imDrawList->_ClipRectStack.empty())
		m_imDrawList->PushClipRectFullScreen();

	try
	{
		m_imDrawList->AddText(font, font->FontSize, ImVec2(static_cast<float>(x), static_cast<float>(y)),
			D3DCOLOR_IMGUI(color), buffer);
	}
	catch (...)
	{
		Utils::logError(XorStr("Unknown ImGui AddText Error."));
	}

	m_imDrawList->PopTextureID();

	UNLOCK_PRESENT();
}

std::pair<int, int> CDrawing::GetDrawTextSize(const char* text, ...)
{
	va_list ap;
	va_start(ap, text);

	char buffer[1024];
	vsprintf_s(buffer, text, ap);

	va_end(ap);

	ImFont* font = m_imFonts.Fonts.back();
	ImVec2 textSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, buffer);

	return std::make_pair(static_cast<int>(textSize.x), static_cast<int>(textSize.y));
}