#include "IBaseClientDLL.h"
#include "../definitions.h"
#include "../../l4d2Simple2/utils.h"

ClientClass* IBaseClientDll::GetAllClasses()
{
	typedef ClientClass*(__thiscall* OriginalFn)(PVOID);
	return Utils::GetVTableFunction<OriginalFn>(this, 8)(this);
}