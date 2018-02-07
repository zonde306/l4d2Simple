#pragma once
#include "../Interfaces/IClientEntity.h"
#include "netprop.h"
#include "handle.h"
#include "../../l4d2Simple2/vector.h"
#include "../../l4d2Simple2/xorstr.h"
#include <exception>
#include <memory>
#include <string>
#include <map>

extern std::map<std::string, int> g_mPropOffset;

class CBaseEntity : public IClientEntity
{
public:


public:	// NetProp
	template<typename T>
	T& GetNetProp(const std::string& table, const std::string& prop, size_t element = 0);

	template<typename T>
	T& GetNetProp2(const std::string& table, const std::string& prop, const std::string& prop2, size_t element = 0);

	template<typename T>
	T& GetNetPropLocal(const std::string& table, const std::string& prop, size_t element = 0);

	template<typename T>
	T& GetNetPropCollision(const std::string& table, const std::string& prop, size_t element = 0);

	static int GetNetPropOffset(const std::string& table, const std::string& prop);

public:
	IClientRenderable* m_pClientRenderable;		// 4
	IClientNetworkable* m_pClientNetworkable;	// 8
};

template<typename T>
inline T & CBaseEntity::GetNetProp(const std::string & table, const std::string & prop, size_t element)
{
	int offset = GetNetPropOffset(table, prop);
	if (offset == -1)
	{
		std::stringstream ss;
		ss << XorStr("NetProp Not Found: ") << table << "::" << prop;
		Utils::log(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	return *reinterpret_cast<T*>((reinterpret_cast<unsigned int>(this) + offset) + (sizeof(T) * element));
}

template<typename T>
inline T & CBaseEntity::GetNetProp2(const std::string & table, const std::string & prop, const std::string & prop2, size_t element)
{
	int offset = GetNetPropOffset(table, prop);
	if (offset == -1)
	{
		std::stringstream ss;
		ss << XorStr("NetProp Not Found: ") << table << "::" << prop;
		Utils::log(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	int offset2 = GetNetPropOffset(table, prop2);
	if(offset2 == -1)
	{
		std::stringstream ss;
		ss << XorStr("NetProp Not Found: ") << table << "::" << prop2;
		Utils::log(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	/*
	if (offset2 < offset)
		offset += offset2;
	*/

	return *reinterpret_cast<T*>((reinterpret_cast<unsigned int>(this) + offset + offset2) + (sizeof(T) * element));
}

template<typename T>
inline T & CBaseEntity::GetNetPropLocal(const std::string & table, const std::string & prop, size_t element)
{
	return GetNetProp2<T>(table, prop, XorStr("m_Local"), element);
}

template<typename T>
inline T & CBaseEntity::GetNetPropCollision(const std::string & table, const std::string & prop, size_t element)
{
	return GetNetProp2<T>(table, prop, XorStr("m_Collision"), element);
}
