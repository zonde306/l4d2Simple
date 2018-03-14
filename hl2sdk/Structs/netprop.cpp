#include "netprop.h"
#include "../Interfaces/IBaseClientDLL.h"
#include "../interfaces.h"
#include "../../l4d2Simple2/utils.h"
#include "../../l4d2Simple2/xorstr.h"
#include <sstream>

#undef GetProp

CNetVars::CNetVars()
{
	_tables.clear();

	ClientClass* clientClass = g_pClientInterface->Client->GetAllClasses();
	if (clientClass == nullptr)
	{
		Utils::log(XorStr("ERROR: ClientClass was not found"));
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
		std::stringstream ss;
		ss << XorStr("ERROR: Failed to find offset for prop: ");
		ss << propName << XorStr(" from table: ") << tableName;
		Utils::log(ss.str().c_str());
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
	std::stringstream ss;

	if (!recvTable)
	{
		ss << XorStr("ERROR: Failed to find table: ") << tableName;
		Utils::log(ss.str().c_str());
		return -1;
	}
	int offset = GetProp(recvTable, propName, prop);
	if (offset <= -1)
	{
		ss << XorStr("ERROR: Failed to find offset for prop: ");
		ss << propName << XorStr(" from table: ") << tableName;
		Utils::log(ss.str().c_str());
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
		std::stringstream ss;
		ss << XorStr("ERROR: Failed to find table: ");
		ss << tableName << XorStr(" (_tables is empty)");
		Utils::log(ss.str().c_str());
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
