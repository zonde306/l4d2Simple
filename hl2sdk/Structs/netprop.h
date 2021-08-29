#pragma once

#include "clientclass.h"

#include <vector>
#include <iostream>

#undef GetProp

class CNetVars
{
public:
	CNetVars();

	int GetOffset(const char* tableName, const char* propName);
	size_t GetCount();
	void DumpClassID(std::ostream& out);

private:
	int GetProp(const char* tableName, const char* propName, RecvProp** prop = 0);
	int GetProp(RecvTable* recvTable, const char* propName, RecvProp** prop = 0);
	RecvTable* GetTable(const char* tableName);

	std::vector<RecvTable*> _tables;
};

typedef enum _fieldtypes
{
	FIELD_VOID = 0, // No type or value
	FIELD_FLOAT, // Any floating point value
	FIELD_STRING, // A string ID (return from ALLOC_STRING)
	FIELD_VECTOR, // Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION, // A quaternion
	FIELD_INTEGER, // Any integer or enum
	FIELD_BOOLEAN, // boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT, // 2 byte integer
	FIELD_CHARACTER, // a byte
	FIELD_COLOR32, // 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED, // an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM, // special type that contains function pointers to it's read/write/parse functions
	FIELD_CLASSPTR, // CBaseEntity *
	FIELD_EHANDLE, // Entity handle
	FIELD_EDICT, // edict_t *
	FIELD_POSITION_VECTOR, // A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME, // a floating point time (these are fixed up automatically too!)
	FIELD_TICK, // an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME, // Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME, // Engine string that is a sound name (needs precache)
	FIELD_INPUT, // a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION, // A class function pointer (Think, Use, etc)
	FIELD_VMATRIX, // a vmatrix (output coords are NOT worldspace)
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_INTERVAL, // a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX, // a model index
	FIELD_MATERIALINDEX, // a material index (using the material precache string table)
	FIELD_VECTOR2D, // 2 floats
	FIELD_TYPECOUNT // MUST BE LAST
} fieldtype_t;

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,
	TD_OFFSET_COUNT
};

struct inputdata_t;
class ISaveRestoreOps;
struct datamap_t;

typedef void (*inputfunc_t)(inputdata_t &data);

class typedescription_t
{
public:
	fieldtype_t fieldType;
	const char* fieldName;
	int fieldOffset[TD_OFFSET_COUNT];
	unsigned short fieldSize;
	short flags;
	const char* externalName;
	ISaveRestoreOps* pSaveRestoreOps;
	inputfunc_t inputFunc;
	datamap_t* td;
	int fieldSizeInBytes;
	typedescription_t* override_field;
	int override_count;
	float fieldTolerance;
};

struct datamap_t
{
	typedescription_t* dataDesc;
	int dataNumFields;
	char const* dataClassName;
	datamap_t* baseMap;
	bool chains_validated;
	bool packed_offsets_computed;
	int packed_size;
};