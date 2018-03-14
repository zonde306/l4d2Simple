#include "IBaseClientDLL.h"
#include "../indexes.h"
#include "../definitions.h"
#include "../../l4d2Simple2/utils.h"

/*
ClientClass* IBaseClientDll::GetAllClasses()
{
	typedef ClientClass*(__thiscall* OriginalFn)(PVOID);
	// return Utils::GetVTableFunction<OriginalFn>(this, 8)(this);
	return reinterpret_cast<OriginalFn>(Utils::GetVirtualFunction(this, indexes::GetAllClasses))(this);
}
*/
