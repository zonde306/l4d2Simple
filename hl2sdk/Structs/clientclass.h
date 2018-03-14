#pragma once
#include "recvprop.h"

typedef void* (*FnCreateClientClass)(int entnum, int serialNum);

struct ClientClass
{
	FnCreateClientClass m_pCreateFn;
	void*			m_pCreateEventFn;
	char			*m_pNetworkName;
	RecvTable		*m_pRecvTable;
	ClientClass		*m_pNext;
	int				m_ClassID;
};
