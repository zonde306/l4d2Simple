#include "netprop.h"
#include "../Interfaces/IBaseClientDLL.h"
#include "../../l4d2Simple2/utils.h"

namespace interfaces
{
	extern IBaseClientDll* Client;
}

#undef GetProp

CNetVars::CNetVars()
{
	_tables.clear();

	ClientClass* clientClass = interfaces::Client->GetAllClasses();
	if (clientClass == nullptr)
	{
		Utils::log("ERROR: ClientClass was not found");
		return;
	}

	while (clientClass)
	{
		RecvTable *recvTable = clientClass->m_pRecvTable;

		_tables.push_back(recvTable);

		clientClass = clientClass->m_pNext;
	}
}

int CNetVars::GetOffset(const char* tableName, const char* propName)
{
	int offset = GetProp(tableName, propName);
	if (offset <= -1)
	{
		Utils::log("ERROR: Failed to find offset for prop: %s from table: %s", propName, tableName);
		return -1;
	}

	return offset;
}

size_t CNetVars::GetCount()
{
	return _tables.size();
}

int CNetVars::GetProp(const char* tableName, const char* propName, RecvProp **prop)
{
	RecvTable* recvTable = GetTable(tableName);
	if (!recvTable)
	{
		Utils::log("ERROR: Failed to find table: %s", tableName);
		return -1;
	}
	int offset = GetProp(recvTable, propName, prop);
	if (offset <= -1)
	{
		Utils::log("ERROR: Failed to find offset for prop: %s from table: %s", propName, tableName);
		return -1;
	}

	return offset;
}

int CNetVars::GetProp(RecvTable *recvTable, const char *propName, RecvProp **prop)
{
	int extraOffset = 0;
	for (int i = 0; i < recvTable->m_nProps; ++i)
	{
		RecvProp* recvProp = &recvTable->m_pProps[i];

		RecvTable *child = recvProp->m_pDataTable;

		if (child && (child->m_nProps > 0))
		{
			int tmp = GetProp(child, propName, prop);
			if (tmp)
			{
				extraOffset += (recvProp->m_Offset + tmp);
			}
		}

		if (_stricmp(recvProp->m_pVarName, propName))
			continue;
		if (prop)
		{
			*prop = recvProp;
		}
		return (recvProp->m_Offset + extraOffset);
	}
	return extraOffset;
}

RecvTable *CNetVars::GetTable(const char *tableName)
{
	if (_tables.empty())
	{
		Utils::log("ERROR: Failed to find table: %s (_tables is empty)", tableName);
		return nullptr;
	}

	for (RecvTable* table : _tables)
	{
		if (!table)
			continue;
		if (_stricmp(table->m_pNetTableName, tableName) == 0)
			return table;
	}
	return nullptr;
}
