#include "Visual.h"

CVisual* g_pVisual = nullptr;

CVisual::CVisual() : CBaseFeatures::CBaseFeatures()
{
}

CVisual::~CVisual()
{
	CBaseFeatures::~CBaseFeatures();
}

void CVisual::OnPaintTraverse(VPANEL panel)
{
}

void CVisual::OnSceneEnd()
{
}

void CVisual::OnMenuDrawing()
{
}
