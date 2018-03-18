#pragma once
#include "BaseFeatures.h"

class CVisual : public CBaseFeatures
{
public:
	CVisual();
	~CVisual();

	virtual void OnPaintTraverse(VPANEL panel) override;
	virtual void OnSceneEnd() override;
	virtual void OnMenuDrawing() override;

private:

};

extern CVisual* g_pVisual;
