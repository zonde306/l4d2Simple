#pragma once
#include "../Interfaces/IClientEntity.h"
#include "../definitions.h"
#include "netprop.h"
#include "handle.h"
#include "matrix.h"
#include "../../l4d2Simple2/vector.h"
#include "../../l4d2Simple2/xorstr.h"
#include "../../l4d2Simple2/utils.h"
#include <exception>
#include <memory>
#include <string>
#include <sstream>
#include <map>

extern std::map<std::string, int> g_mPropOffset;

#define Assert_NetProp(_prop)		Assert(_prop)

#define DECL_NETPROP_OFFSET(_table,_name)			static int offset = GetNetPropOffset(XorStr(_table), XorStr(_name));\
	Assert_NetProp(offset)

#define DECL_NETPROP_GET(_type)						*reinterpret_cast<_type*>(reinterpret_cast<DWORD>(this) + offset)
#define DECL_NETPROP_GET_EX(_offset,_type)			*reinterpret_cast<_type*>(reinterpret_cast<DWORD>(this) + _offset)

#define DECL_NETPROP_OFFSET_RET(_table,_name,_type)	DECL_NETPROP_OFFSET(_table, _name); return DECL_NETPROP_GET(_type);


class CBaseEntity : public IClientEntity
{
public:	// NetProp
	template<typename T>
	T& GetNetProp(const std::string& table, const std::string& prop, size_t element = 0);

	template<typename T>
	T& GetNetProp2(const std::string& table, const std::string& prop, const std::string& prop2, size_t element = 0);

	template<typename T, typename... P>
	T& GetNetPropEx(const std::string& table, const std::string& prop, size_t element, const P& ...more);

	// 没有用的，不需要使用
	// 像是 m_Local 这种二次偏移已经不需要了
	template<typename T>
	T& GetNetPropLocal(const std::string& table, const std::string& prop, size_t element = 0);

	template<typename T>
	T& GetNetPropLocal(const std::string& prop, size_t element = 0);

	// 没有用的，不需要使用
	// 像是 m_Collision 这种二次偏移已经不需要了
	template<typename T>
	T& GetNetPropCollision(const std::string& table, const std::string& prop, size_t element = 0);

	template<typename T>
	T& GetNetPropCollision(const std::string& prop, size_t element = 0);

	static int GetNetPropOffset(const std::string& table, const std::string& prop, bool cache = false);

	template<typename ...T>
	static int GetNetPropOffsetEx(const std::string& table, const std::string& prop, const T& ...more);

public:
	/*
	IClientRenderable* m_pClientRenderable;		// 4
	IClientNetworkable* m_pClientNetworkable;	// 8
	*/

	inline IClientRenderable* GetRenderable();
	inline IClientNetworkable* GetNetworkable();

public:
	bool IsValid();
	bool IsDormant();
	int GetIndex();
	bool SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);
	int DrawModel(int flags, float alpha = 1.f);
	model_t* GetModel();
	Vector GetHitboxOrigin(int hitbox);
	Vector GetBoneOrigin(int bone);
	Vector& GetAbsOrigin();
	QAngle& GetAbsAngles();
	ClientClass* GetClientClass();
	int GetClassID();
	int GetSequence();
	int GetTeam();
	bool IsPlayer();
	bool IsNPC();
	MoveType_t GetMoveType();
	const char* GetClassname();
	Vector GetEyePosition();
	const QAngle& GetEyeAngles();
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

	// NetProp 强制 4 字节对齐
	return *reinterpret_cast<T*>((reinterpret_cast<DWORD>(this) + offset) + (element * 4));
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

	// NetProp 强制 4 字节对齐
	return *reinterpret_cast<T*>((reinterpret_cast<DWORD>(this) + offset + offset2) + (element * 4));
}

template<typename T, typename ...P>
inline T & CBaseEntity::GetNetPropEx(const std::string & table, const std::string & prop, size_t element, const P & ...more)
{
	int offset = GetNetPropOffsetEx(table, prop, more...);
	if (offset == -1)
	{
		std::stringstream ss;

		std::string propList[] { more... };
		ss << XorStr("NetProp Not Found: ") << table << "::" << prop;

		for (const std::string& propEach : propList)
			ss << "::" << propEach;
		
		Utils::log(ss.str().c_str());
		throw std::runtime_error(ss.str().c_str());
	}

	// NetProp 强制 4 字节对齐
	return *reinterpret_cast<T*>((reinterpret_cast<DWORD>(this) + offset) + (element * 4));
}

template<typename T>
inline T & CBaseEntity::GetNetPropLocal(const std::string & table, const std::string & prop, size_t element)
{
	// return GetNetProp2<T>(table, prop, XorStr("m_Local"), element);
	return GetNetPropEx<T>(table, prop, element, XorStr("m_Local"));
}

template<typename T>
inline T& CBaseEntity::GetNetPropLocal(const std::string& prop, size_t element)
{
	return GetNetProp2<T>(XorStr("DT_Local"), prop, XorStr("m_Local"), element);
}

template<typename T>
inline T & CBaseEntity::GetNetPropCollision(const std::string & table, const std::string & prop, size_t element)
{
	// return GetNetProp2<T>(table, prop, XorStr("m_Collision"), element);
	return GetNetPropEx<T>(table, prop, element, XorStr("m_Collision"));
}

template<typename T>
inline T& CBaseEntity::GetNetPropCollision(const std::string& prop, size_t element)
{
	return GetNetPropEx<T>(XorStr("DT_CollisionProperty"), prop, element, XorStr("m_Collision"));
}

template<typename ...T>
inline int CBaseEntity::GetNetPropOffsetEx(const std::string & table, const std::string & prop, const T & ...more)
{
	std::string morePropList[] { more... };
	int offset = GetNetPropOffset(table, prop);

	for (const std::string& propEach : morePropList)
		offset += GetNetPropOffset(table, propEach);

	return (offset > -1 ? offset : -1);
}

inline IClientRenderable * CBaseEntity::GetRenderable()
{
	return reinterpret_cast<IClientRenderable*>(reinterpret_cast<DWORD>(this) + 0x4);
}

inline IClientNetworkable * CBaseEntity::GetNetworkable()
{
	return reinterpret_cast<IClientNetworkable*>(reinterpret_cast<DWORD>(this) + 0x8);
}
