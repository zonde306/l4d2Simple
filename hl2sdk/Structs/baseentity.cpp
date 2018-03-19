#include "baseentity.h"
#include "../Utils/math.h"
#include "../interfaces.h"
#include "../indexes.h"

std::map<std::string, int> g_mPropOffset;

int CBaseEntity::GetNetPropOffset(const std::string & table, const std::string & prop)
{
	auto it = g_mPropOffset.find(prop);
	if (it == g_mPropOffset.end())
		g_mPropOffset.emplace(prop, g_pClientInterface->NetProp->GetOffset(table.c_str(), prop.c_str()));
	else
		return it->second;

	std::stringstream ss;
	ss << table << "::" << prop << " = " << g_mPropOffset[prop];
	Utils::log(ss.str().c_str());

	return g_mPropOffset[prop];
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
		if (!SetupBones(boneMatrix, 128, 0x00000100, g_pClientInterface->GlobalVars->curtime))
		{
			Utils::log(XorStr("GetHitboxOrigin.SetupBones Failed."));
			return Vector();
		}

		if ((model = GetModel()) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.GetModel Failed."));
			return Vector();
		}

		if ((hdr = g_pClientInterface->ModelInfo->GetStudiomodel(model)) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.GetStudiomodel Failed."));
			return Vector();
		}

		if ((set = hdr->pHitboxSet(0)) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.pHitboxSet Failed."));
			return Vector();
		}

		if ((hitboxMat = set->pHitbox(hitbox)) == nullptr)
		{
			Utils::log(XorStr("GetHitboxOrigin.pHitbox Failed."));
			return Vector();
		}
	}
	catch (...)
	{
		Utils::log(XorStr("GetHitboxOrigin.Unknown Error."));
		return Vector();
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
		if (SetupBones(boneMatrix, 128, 0x00000100, g_pClientInterface->GlobalVars->curtime))
			return Vector(boneMatrix[bone][0][3], boneMatrix[bone][1][3], boneMatrix[bone][2][3]);
	}
	catch (...)
	{
		Utils::log(XorStr("GetBoneOrigin.Unknown Error."));
		return Vector();
	}

	return Vector();
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
