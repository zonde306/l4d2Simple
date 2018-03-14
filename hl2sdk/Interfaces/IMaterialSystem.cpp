#include "IMaterialSystem.h"
#include "IMaterial.h"
#include "../Structs/keyvalues.h"
#include "../../l4d2Simple2/xorstr.h"
#include "../../l4d2Simple2/utils.h"
#include <string>

/*
int created = 0
IMaterial* IMaterialSystem::CreateMaterial( bool flat, bool ignorez, bool wireframed )
{
	std::string type = ( flat ) ? XorStr( "UnlitGeneric" ) : XorStr( "VertexLitGeneric" );

	std::string matdata = XorStr( "\"" ) + type + XorStr( "\"\n{\n\t\"$basetexture\" \"vgui/white_additive\"\n\t\"$envmap\"  \"\"\n\t\"$model\" \"1\"\n\t\"$flat\" \"1\"\n\t\"$nocull\"  \"0\"\n\t\"$selfillum\" \"1\"\n\t\"$halflambert\" \"1\"\n\t\"$nofog\"  \"0\"\n\t\"$znearer\" \"0\"\n\t\"$wireframe\" \"" ) + std::to_string( wireframed ) + XorStr( "\"\n\t\"$ignorez\" \"" ) + std::to_string( ignorez ) + XorStr( "\"\n}\n" );

	std::string matname = XorStr( "custom_" ) + std::to_string( created );
	++created;

	KeyValues* pKeyValues = new KeyValues( matname.c_str() );
	Utils::InitKeyValues( pKeyValues, type.c_str() );
	Utils::LoadFromBuffer( pKeyValues, matname.c_str(), matdata.c_str() );

	typedef IMaterial*( __thiscall* OriginalFn )( void*, const char *pMaterialName, KeyValues *pVMTKeyValues );
	IMaterial* createdMaterial = Utils::GetVTableFunction<OriginalFn>( this, 83 )( this, matname.c_str(), pKeyValues );

	createdMaterial->IncrementReferenceCount();

	return createdMaterial;
}

IMaterial* IMaterialSystem::FindMaterial( char const* pMaterialName, const char *pTextureGroupName, bool complain, const char *pComplainPrefix )
{
	typedef IMaterial*( __thiscall* OriginalFn )( void*, char const* pMaterialName, const char *pTextureGroupName, bool complain, const char *pComplainPrefix );
	return Utils::GetVTableFunction<OriginalFn>( this, 84 )( this, pMaterialName, pTextureGroupName, complain, pComplainPrefix );
}
*/
