#include "ISurface.h"
#include "../indexes.h"
#include "../../l4d2Simple2/utils.h"

#ifdef _CSGO
void ISurface::DrawSetColor(int r, int g, int b, int a)
{
	typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawSetColor)(this, r, g, b, a);
}

void ISurface::DrawSetColor(Color col)
{
	typedef void(__thiscall* OriginalFn)(void*, Color);
	return Utils::GetVTableFunction<OriginalFn>(this, 14)(this, col);
}

void ISurface::DrawFilledRect(int x0, int y0, int x1, int y1)
{ 
	typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawFilledRect)(this, x0, y0, x1, y1);
}

void ISurface::DrawOutlinedRect(int x0, int y0, int x1, int y1)
{ 
	typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawOutlinedRect)(this, x0, y0, x1, y1);
}

void ISurface::DrawLine(int x0, int y0, int x1, int y1)
{
	typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawLine)(this, x0, y0, x1, y1);
}

void ISurface::DrawPolyLine(int *px, int *py, int numPoints) 
{
	typedef void(__thiscall* OriginalFn)(void*, int*, int*, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawPolyLine)(this, px, py, numPoints);
}

void ISurface::DrawSetTextFont(HFont font)
{ 
	typedef void(__thiscall* OriginalFn)(void*, HFont);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawSetTextFont)(this, font);
}

void ISurface::DrawSetTextColor(int r, int g, int b, int a) 
{
	typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawSetTextColor)(this, r, g, b, a);
}

void ISurface::DrawSetTextColor(Color col)
{
	typedef void(__thiscall* OriginalFn)(void*, Color);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawSetTextColor_Color)(this, col);
}

void ISurface::DrawSetTextPos(int x, int y) 
{ 
	typedef void(__thiscall* OriginalFn)(void*, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawSetTextPos)(this, x, y);
}

void ISurface::DrawPrintText(const wchar_t *text, int textLen, FontDrawType_t drawType)
{ 
	typedef void(__thiscall* OriginalFn)(void*, const wchar_t*, int, FontDrawType_t);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::DrawPrintText)(this, text, textLen, drawType);
}

void ISurface::DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall)
{
	typedef void(__thiscall* OriginalFn)(void*, int, const unsigned char*, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, 37)(this, id, rgba, wide, tall);
}

void ISurface::DrawSetTexture(int id) 
{
	typedef void(__thiscall* OriginalFn)(void*, int);
	return Utils::GetVTableFunction<OriginalFn>(this, 38)(this, id);
}

int	ISurface::CreateNewTextureID(bool procedural)
{
	typedef int(__thiscall* OriginalFn)(void*, bool);
	return Utils::GetVTableFunction<OriginalFn>(this, 43)(this, procedural);
}

HFont ISurface::Create_Font()
{
	typedef HFont(__thiscall* OriginalFn)(void*);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::SCreateFont)(this);
}

bool ISurface::SetFontGlyphSet(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin, int nRangeMax)
{
	typedef bool(__thiscall* OriginalFn)(void*, HFont, const char*, int, int, int, int, int, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::SetFontGlyphSet)(this, font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
}

void ISurface::GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall)
{
	typedef void(__thiscall* OriginalFn)(void*, HFont, const wchar_t*, int&, int&);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::GetTextSize)(this, font, text, wide, tall);
}

void ISurface::DrawOutlinedCircle(int x, int y, int radius, int segments) 
{ 
	typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
	return Utils::GetVTableFunction<OriginalFn>(this, 103)(this, x, y, radius, segments);
}

void ISurface::DrawTexturedPolygon(int n, Vertex_t *pVertice, bool bClipVertices)
{
	typedef void(__thiscall* OriginalFn)(void*, int, Vertex_t*, bool);
	return Utils::GetVTableFunction<OriginalFn>(this, 106)(this, n, pVertice, bClipVertices);
}
#endif
