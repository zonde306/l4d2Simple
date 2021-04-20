#include "baseentity.h"
#include "../Utils/math.h"
#include "../interfaces.h"
#include "../indexes.h"

std::map<std::string, int> g_mPropOffset;
#define SIG_SET_MOVETYPE	XorStr("55 8B EC 8A 45 08 8A 55 0C 88 81")
#define SIG_GET_CLASSNAME	XorStr("57 8B F9 C6 05")

int CBaseEntity::GetNetPropOffset(const std::string & table, const std::string & prop, bool cache)
{
	if (!cache)
		return g_pInterface->NetProp->GetOffset(table.c_str(), prop.c_str());
	
	auto it = g_mPropOffset.find(prop);
	if (it == g_mPropOffset.end())
		g_mPropOffset.emplace(prop, g_pInterface->NetProp->GetOffset(table.c_str(), prop.c_str()));
	else
		return it->second;

	std::stringstream ss;
	ss << table << "::" << prop << " = " << g_mPropOffset[prop];
	Utils::log(ss.str().c_str());

	return g_mPropOffset[prop];
}

bool CBaseEntity::IsValid()
{
	if (this == nullptr)
		return false;

	IClientNetworkable* net = GetNetworkable();
	if (net == nullptr)
		return false;

	// 检查虚函数表(client.dll)
	static auto moduleInfo = Utils::GetModuleSize(XorStr("client.dll"));
	if (moduleInfo.first > moduleInfo.second)
	{
		DWORD address = reinterpret_cast<DWORD>(Utils::GetVirtualFunction(net, indexes::IsDormant));
		if (address < moduleInfo.first || address > moduleInfo.second)
			return false;
	}

	try
	{
		return (!IsDormant() && GetIndex() > 0);
	}
	catch (...)
	{
	}

	return false;
}

bool CBaseEntity::IsDormant()
{
	try
	{
		return GetNetworkable()->IsDormant();
	}
	catch (...)
	{

	}

	return true;
}

int CBaseEntity::GetIndex()
{
	return GetNetworkable()->entindex();
}

bool CBaseEntity::SetupBones(matrix3x4_t * pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	
	return GetRenderable()->SetupBones(pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}

int CBaseEntity::DrawModel(int flags, float alpha)
{
	return GetRenderable()->DrawModel(flags, alpha);
}

model_t * CBaseEntity::GetModel()
{
	return const_cast<model_t*>(GetRenderable()->GetModel());
}

Vector CBaseEntity::GetHitboxOrigin(int hitbox)
{
	matrix3x4_t boneMatrix[128];
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* hitboxMat;
	Vector min, max;

	try
	{
		if (!SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, g_pInterface->GlobalVars->curtime))
		{
			Utils::log(XorStr("GetHitboxOrigin.SetupBones Failed."));
			return INVALID_VECTOR;
		}

		if ((model = GetModel()) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.GetModel Failed."));
			return INVALID_VECTOR;
		}

		if ((hdr = g_pInterface->ModelInfo->GetStudiomodel(model)) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.GetStudiomodel Failed."));
			return INVALID_VECTOR;
		}

		if ((set = hdr->pHitboxSet(0)) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.GetHitboxSet Failed."));
			return INVALID_VECTOR;
		}

		if ((hitboxMat = set->pHitbox(hitbox)) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.GetHitbox Failed."));
			return INVALID_VECTOR;
		}
	}
	catch (...)
	{
		Utils::log(XorStr("GetHitboxOrigin.Unknown Error."));
		return INVALID_VECTOR;
	}

	math::VectorTransform(hitboxMat->bbmin, boneMatrix[hitboxMat->bone], min);
	math::VectorTransform(hitboxMat->bbmax, boneMatrix[hitboxMat->bone], max);
	return ((min + max) * 0.5f);
}

Vector CBaseEntity::GetBoneOrigin(int bone)
{
	matrix3x4_t boneMatrix[128];

	try
	{
		if (SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, g_pInterface->GlobalVars->curtime))
			return Vector(boneMatrix[bone][0][3], boneMatrix[bone][1][3], boneMatrix[bone][2][3]);
	}
	catch (...)
	{
		Utils::log(XorStr("GetBoneOrigin.Unknown Error."));
		return INVALID_VECTOR;
	}

	return INVALID_VECTOR;
}

Vector& CBaseEntity::GetAbsOrigin()
{
	using Fn = Vector&(__thiscall*)(CBaseEntity*);
	return Utils::GetVTableFunction<Fn>(this, indexes::GetAbsOrigin)(this);
}

QAngle& CBaseEntity::GetAbsAngles()
{
	using Fn = QAngle&(__thiscall*)(CBaseEntity*);
	return Utils::GetVTableFunction<Fn>(this, indexes::GetAbsAngles)(this);
}

ClientClass * CBaseEntity::GetClientClass()
{
	return GetNetworkable()->GetClientClass();
}

int CBaseEntity::GetClassID()
{
	ClientClass* cc = GetClientClass();
	if (cc == nullptr)
		return ET_INVALID;

	return cc->m_ClassID;
}

int CBaseEntity::GetSequence()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseAnimating"), XorStr("m_nSequence"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(WORD);
}

int CBaseEntity::GetTeam()
{
	using FnGetTeamNumber = int(__thiscall*)(CBaseEntity*);
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_iTeamNum"));
	FnGetTeamNumber fn = nullptr;
	Assert_NetProp(offset);

	try
	{
		fn = Utils::GetVTableFunction<FnGetTeamNumber>(this, indexes::GetTeamNumber);
		if (fn != nullptr)
			return fn(this);

		return DECL_NETPROP_GET(byte);
	}
	catch (...)
	{

	}

	return 1;
}

bool CBaseEntity::IsPlayer()
{
	/*
	int index = GetIndex();
	if(index <= 0 || index > 32)
		return false;
	*/

	using FnIsPlayer = bool(__thiscall*)(CBaseEntity*);
	FnIsPlayer fn = Utils::GetVTableFunction<FnIsPlayer>(this, indexes::IsPlayer);
	return fn(this);
}

bool CBaseEntity::IsNPC()
{
	using FnIsNextBot = bool(__thiscall*)(CBaseEntity*);
	FnIsNextBot fn = Utils::GetVTableFunction<FnIsNextBot>(this, indexes::IsNextBot);
	return fn(this);
}

MoveType_t CBaseEntity::GetMoveType()
{
	// 在 C_BaseEntity::SetMoveType 里
	// this 后的第一个参数
	return static_cast<MoveType_t>(*reinterpret_cast<PBYTE>(reinterpret_cast<DWORD>(this) + indexes::MoveType));
}

const char * CBaseEntity::GetClassname()
{
	using FnGetClassname = const char*(__thiscall*)(CBaseEntity*);
	static FnGetClassname fn = reinterpret_cast<FnGetClassname>(Utils::FindPattern(XorStr("client.dll"), SIG_GET_CLASSNAME));
	return fn(this);
}

const char* CBaseEntity::GetClientClassname()
{
	ClientClass* cc = GetClientClass();
	if (cc == nullptr)
		return "";

	return cc->m_pNetworkName;
}

Vector CBaseEntity::GetEyePosition()
{
	using FnEyePosition = Vector(__thiscall*)(CBaseEntity*);
	FnEyePosition fn = Utils::GetVTableFunction<FnEyePosition>(this, indexes::EyePosition);

	try
	{
		return fn(this);
	}
	catch (...)
	{
		
	}

	return INVALID_VECTOR;
}

QAngle CBaseEntity::GetEyeAngles()
{
	using FnEyeAngles = const QAngle&(__thiscall*)(CBaseEntity*);
	FnEyeAngles fn = Utils::GetVTableFunction<FnEyeAngles>(this, indexes::EyeAngles);

	try
	{
		return fn(this);
	}
	catch (...)
	{

	}

	return INVALID_VECTOR;
}

ICollideable* CBaseEntity::GetCollideable()
{
	using FnGetCollideable = ICollideable*(__thiscall*)(CBaseEntity*);
	FnGetCollideable fn = Utils::GetVTableFunction<FnGetCollideable>(this, indexes::GetCollideable);
	return fn(this);
}

int CBaseEntity::GetSolidFlags()
{
	ICollideable* collideable = nullptr;

	try
	{
		collideable = GetCollideable();
	}
	catch (...)
	{
		return 0;
	}

	if (collideable)
		return collideable->GetSolidFlags();

	return 0;
}

CBaseEntity* CBaseEntity::GetOwner()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseEntity"), XorStr("m_hOwnerEntity"));
	Assert_NetProp(offset);

	CBaseHandle hdl = DECL_NETPROP_GET(CBaseHandle);
	if (!hdl.IsValid())
		return nullptr;

	return reinterpret_cast<CBaseEntity*>(g_pInterface->EntList->GetClientEntityFromHandle(hdl));
}

Vector CBaseEntity::GetVelocity()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseGrenade"), XorStr("m_vecVelocity"));
	Assert_NetProp(offset);

	return DECL_NETPROP_GET(Vector);
}
