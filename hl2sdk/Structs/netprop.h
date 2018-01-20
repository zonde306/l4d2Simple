#pragma once
#include "clientclass.h"
#include <vector>

#undef GetProp

class CNetVars
{
public:
	CNetVars();
	int GetOffset(const char* tableName, const char* propName);
	size_t GetCount();
private:
	int GetProp(const char* tableName, const char* propName, RecvProp **prop = 0);
	int GetProp(RecvTable *recvTable, const char *propName, RecvProp **prop = 0);
	RecvTable *GetTable(const char *tableName);
	std::vector<RecvTable*> _tables;
};
