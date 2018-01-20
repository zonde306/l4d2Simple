#include "baseentity.h"

std::map<std::string, int> g_mPropOffset;

inline int CBaseEntity::GetNetPropOffset(const std::string & table, const std::string & prop)
{
	auto it = g_mPropOffset.find(prop);
	if (it == g_mPropOffset.end())
		g_mPropOffset.emplace(prop, interfaces::NetProp->GetOffset(table.c_str(), prop.c_str()));
	else
		return it->second;

	return g_mPropOffset[prop];
}
