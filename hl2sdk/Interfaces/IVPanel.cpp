#include "IVPanel.h"
#include "../indexes.h"
#include "../../l4d2Simple2/utils.h"

const char* IVPanel::GetName(int iIndex)
{
	typedef const char*(__thiscall* OriginalFn)(void*, int);
	return Utils::GetVTableFunction<OriginalFn>(this, indexes::GetName)(this, iIndex);
}