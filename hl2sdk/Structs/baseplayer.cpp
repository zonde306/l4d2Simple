#include "baseplayer.h"
#include "../indexes.h"

Vector& CBasePlayer::GetPunch()
{
	return *reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(this) + indexes::GetPunch);
}
