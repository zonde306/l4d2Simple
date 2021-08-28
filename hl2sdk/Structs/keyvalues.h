#pragma once

#include <Windows.h>

#include "color.h"
#include "../Utils/utlvector.h"

#define FOR_EACH_SUBKEY( kvRoot, kvSubKey ) \
	for ( KeyValues * kvSubKey = kvRoot->GetFirstSubKey(); kvSubKey != NULL; kvSubKey = kvSubKey->GetNextKey() )

#define FOR_EACH_TRUE_SUBKEY( kvRoot, kvSubKey ) \
	for ( KeyValues * kvSubKey = kvRoot->GetFirstTrueSubKey(); kvSubKey != NULL; kvSubKey = kvSubKey->GetNextTrueSubKey() )

#define FOR_EACH_VALUE( kvRoot, kvValue ) \
	for ( KeyValues * kvValue = kvRoot->GetFirstValue(); kvValue != NULL; kvValue = kvValue->GetNextValue() )

class IBaseFileSystem;
class CUtlBuffer;
class Color;

typedef void * FileHandle_t;

class CKeyValuesGrowableStringTable;

class KeyValues
{
public:
	static void SetUseGrowableStringTable(bool bUseGrowableTable);

	KeyValues(const char* setName);

	class AutoDelete
	{
	public:
		explicit inline AutoDelete(KeyValues* pKeyValues) : m_pKeyValues(pKeyValues) {}
		explicit inline AutoDelete(const char* pchKVName) : m_pKeyValues(new KeyValues(pchKVName)) {}

		inline ~AutoDelete(void)
		{
			if (m_pKeyValues)
				m_pKeyValues->deleteThis();
		}

		inline void Assign(KeyValues* pKeyValues)
		{
			m_pKeyValues = pKeyValues;
		}

		KeyValues* operator->()
		{
			return m_pKeyValues;
		}

		operator KeyValues* ()
		{
			return m_pKeyValues;
		}

	private:
		AutoDelete(AutoDelete const& x);
		AutoDelete& operator= (AutoDelete const& x);
		KeyValues* m_pKeyValues;
	};

	KeyValues(const char* setName, const char* firstKey, const char* firstValue);
	KeyValues(const char* setName, const char* firstKey, const wchar_t* firstValue);
	KeyValues(const char* setName, const char* firstKey, int firstValue);
	KeyValues(const char* setName, const char* firstKey, const char* firstValue, const char* secondKey, const char* secondValue);
	KeyValues(const char* setName, const char* firstKey, int firstValue, const char* secondKey, int secondValue);

	const char* GetName() const;
	void SetName(const char* setName);

	int GetNameSymbol() const
	{
		return m_iKeyName;
	}

	void UsesEscapeSequences(bool state);
	void UsesConditionals(bool state);

	bool LoadFromFile(IBaseFileSystem* filesystem, const char* resourceName, const char* pathID = NULL, bool refreshCache = false);
	bool SaveToFile(IBaseFileSystem* filesystem, const char* resourceName, const char* pathID = NULL, bool sortKeys = false, bool bAllowEmptyString = false, bool bCacheResult = false);

	bool LoadFromBuffer(char const* resourceName, const char* pBuffer, IBaseFileSystem* pFileSystem = NULL, const char* pPathID = NULL);
	bool LoadFromBuffer(char const* resourceName, CUtlBuffer& buf, IBaseFileSystem* pFileSystem = NULL, const char* pPathID = NULL);

	KeyValues* FindKey(const char* keyName, bool bCreate = false);
	KeyValues* FindKey(int keySymbol) const;
	KeyValues* CreateNewKey();
	void AddSubKey(KeyValues* pSubkey);
	void RemoveSubKey(KeyValues* subKey);

	KeyValues* GetFirstSubKey()
	{
		return m_pSub;
	}

	KeyValues* GetNextKey()
	{
		return m_pPeer;
	}

	const KeyValues* GetNextKey() const
	{
		return m_pPeer;
	}

	void SetNextKey(KeyValues* pDat);
	KeyValues* FindLastSubKey();

	KeyValues* GetFirstTrueSubKey();
	KeyValues* GetNextTrueSubKey();

	KeyValues* GetFirstValue();
	KeyValues* GetNextValue();

	int GetInt(const char* keyName = NULL, int defaultValue = 0);
	uint64_t GetUint64(const char* keyName = NULL, uint64_t defaultValue = 0);
	float GetFloat(const char* keyName = NULL, float defaultValue = 0.0f);
	const char* GetString(const char* keyName = NULL, const char* defaultValue = "");
	const wchar_t* GetWString(const char* keyName = NULL, const wchar_t* defaultValue = L"");
	void* GetPtr(const char* keyName = NULL, void* defaultValue = (void*)0);
	bool GetBool(const char* keyName = NULL, bool defaultValue = false, bool* optGotDefault = NULL);
	Color GetColor(const char* keyName = NULL);
	bool IsEmpty(const char* keyName = NULL);

	int GetInt(int keySymbol, int defaultValue = 0);
	float GetFloat(int keySymbol, float defaultValue = 0.0f);
	const char* GetString(int keySymbol, const char* defaultValue = "");
	const wchar_t* GetWString(int keySymbol, const wchar_t* defaultValue = L"");
	void* GetPtr(int keySymbol, void* defaultValue = (void*)0);
	Color GetColor(int keySymbol);
	bool IsEmpty(int keySymbol);

	void SetWString(const char* keyName, const wchar_t* value);
	void SetString(const char* keyName, const char* value);
	void SetInt(const char* keyName, int value);
	void SetUint64(const char* keyName, uint64_t value);
	void SetFloat(const char* keyName, float value);
	void SetPtr(const char* keyName, void* value);
	void SetColor(const char* keyName, Color value);

	void SetBool(const char* keyName, bool value)
	{
		SetInt(keyName, value ? 1 : 0);
	}

	void* operator new(size_t iAllocSize);
	void* operator new(size_t iAllocSize, int nBlockUse, const char* pFileName, int nLine);
	void operator delete(void* pMem);
	void operator delete(void* pMem, int nBlockUse, const char* pFileName, int nLine);

	KeyValues& operator=(const KeyValues& src);

	void ChainKeyValue(KeyValues* pChain);

	void RecursiveSaveToFile(CUtlBuffer& buf, int indentLevel, bool sortKeys = false, bool bAllowEmptyString = false);

	bool WriteAsBinary(CUtlBuffer& buffer);
	bool ReadAsBinary(CUtlBuffer& buffer, int nStackDepth = 0);

	KeyValues* MakeCopy(void) const;
	KeyValues* MakeCopy(bool copySiblings) const;

	void CopySubkeys(KeyValues* pParent) const;
	void Clear(void);

	enum types_t
	{
		TYPE_NONE = 0,
		TYPE_STRING,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_PTR,
		TYPE_WSTRING,
		TYPE_COLOR,
		TYPE_UINT64,
		TYPE_NUMTYPES
	};

	types_t GetDataType(const char* keyName = NULL);

	void deleteThis();

	void SetStringValue(char const* strValue);

	void UnpackIntoStructure(struct KeyValuesUnpackStructure const* pUnpackTable, void* pDest, size_t DestSizeInBytes);
	bool ProcessResolutionKeys(const char* pResString);

	bool Dump(class IKeyValuesDumpContext* pDump, int nIndentLevel = 0);

	void RecursiveMergeKeyValues(KeyValues* baseKV);

private:
	KeyValues(KeyValues&);
	~KeyValues();

	KeyValues* CreateKey(const char* keyName);
	KeyValues* CreateKeyUsingKnownLastChild(const char* keyName, KeyValues* pLastChild);
	void AddSubkeyUsingKnownLastChild(KeyValues* pSubKey, KeyValues* pLastChild);

	void CopyKeyValuesFromRecursive(const KeyValues& src);
	void CopyKeyValue(const KeyValues& src, size_t tmpBufferSizeB, char* tmpBuffer);

	void RemoveEverything();

	void RecursiveSaveToFile(IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, int indentLevel, bool sortKeys, bool bAllowEmptyString);
	void SaveKeyToFile(KeyValues* dat, IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, int indentLevel, bool sortKeys, bool bAllowEmptyString);
	void WriteConvertedString(IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, const char* pszString);

	void RecursiveLoadFromBuffer(char const* resourceName, CUtlBuffer& buf);

	void AppendIncludedKeys(CUtlVector< KeyValues* >& includedKeys);
	void ParseIncludedKeys(char const* resourceName, const char* filetoinclude,
		IBaseFileSystem* pFileSystem, const char* pPathID, CUtlVector< KeyValues* >& includedKeys);

	void MergeBaseKeys(CUtlVector< KeyValues* >& baseKeys);

	void InternalWrite(IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, const void* pData, int len);

	void Init();
	const char* ReadToken(CUtlBuffer& buf, bool& wasQuoted, bool& wasConditional);
	void WriteIndents(IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, int indentLevel);

	void FreeAllocatedValue();
	void AllocateValueBlock(int size);

	int m_iKeyName;
	char* m_sValue;
	wchar_t* m_wsValue;

	union
	{
		int m_iValue;
		float m_flValue;
		void* m_pValue;
		unsigned char m_Color[4];
	};

	char m_iDataType;
	char m_bHasEscapeSequences;
	char m_bEvaluateConditionals;
	char unused[1];

	KeyValues* m_pPeer;
	KeyValues* m_pSub;
	KeyValues* m_pChain;

private:
	static int(*s_pfGetSymbolForString)(const char* name, bool bCreate);
	static const char* (*s_pfGetStringForSymbol)(int symbol);
	static CKeyValuesGrowableStringTable* s_pGrowableStringTable;

public:
	static int GetSymbolForStringClassic(const char* name, bool bCreate = true);
	static const char* GetStringForSymbolClassic(int symbol);

	static int GetSymbolForStringGrowable(const char* name, bool bCreate = true);
	static const char* GetStringForSymbolGrowable(int symbol);

	static int CallGetSymbolForString(const char* name, bool bCreate = true) { return s_pfGetSymbolForString(name, bCreate); }
	static const char* CallGetStringForSymbol(int symbol) { return s_pfGetStringForSymbol(symbol); }
};

typedef KeyValues::AutoDelete KeyValuesAD;

enum KeyValuesUnpackDestinationTypes_t
{
	UNPACK_TYPE_FLOAT, 
	UNPACK_TYPE_VECTOR, 
	UNPACK_TYPE_VECTOR_COLOR, 
	UNPACK_TYPE_STRING,
	UNPACK_TYPE_INT, 
	UNPACK_TYPE_FOUR_FLOATS, 
	UNPACK_TYPE_TWO_FLOATS, 
};

#define UNPACK_FIXED( kname, kdefault, dtype, ofs ) { kname, kdefault, dtype, ofs, 0 }
#define UNPACK_VARIABLE( kname, kdefault, dtype, ofs, sz ) { kname, kdefault, dtype, ofs, sz }
#define UNPACK_END_MARKER { NULL, NULL, UNPACK_TYPE_FLOAT, 0 }

struct KeyValuesUnpackStructure
{
	char const *m_pKeyName; 
	char const *m_pKeyDefault;
	KeyValuesUnpackDestinationTypes_t m_eDataType; 
	size_t m_nFieldOffset; 
	size_t m_nFieldSize; 
};

inline int KeyValues::GetInt(int keySymbol, int defaultValue)
{
	KeyValues* dat = FindKey(keySymbol);
	return dat ? dat->GetInt((const char*)NULL, defaultValue) : defaultValue;
}

inline float KeyValues::GetFloat(int keySymbol, float defaultValue)
{
	KeyValues* dat = FindKey(keySymbol);
	return dat ? dat->GetFloat((const char*)NULL, defaultValue) : defaultValue;
}

inline const char* KeyValues::GetString(int keySymbol, const char* defaultValue)
{
	KeyValues* dat = FindKey(keySymbol);
	return dat ? dat->GetString((const char*)NULL, defaultValue) : defaultValue;
}

inline const wchar_t* KeyValues::GetWString(int keySymbol, const wchar_t* defaultValue)
{
	KeyValues* dat = FindKey(keySymbol);
	return dat ? dat->GetWString((const char*)NULL, defaultValue) : defaultValue;
}

inline void* KeyValues::GetPtr(int keySymbol, void* defaultValue)
{
	KeyValues* dat = FindKey(keySymbol);
	return dat ? dat->GetPtr((const char*)NULL, defaultValue) : defaultValue;
}

inline Color KeyValues::GetColor(int keySymbol)
{
	Color defaultValue(0, 0, 0, 0);
	KeyValues* dat = FindKey(keySymbol);
	return dat ? dat->GetColor() : defaultValue;
}

inline bool  KeyValues::IsEmpty(int keySymbol)
{
	KeyValues* dat = FindKey(keySymbol);
	return dat ? dat->IsEmpty() : true;
}

bool EvaluateConditional(const char *str);

class CUtlSortVectorKeyValuesByName
{
public:
	bool Less(const KeyValues* lhs, const KeyValues* rhs, void*)
	{
		return Q_stricmp(lhs->GetName(), rhs->GetName()) < 0;
	}
};

class IKeyValuesDumpContext
{
public:
	virtual bool KvBeginKey(KeyValues* pKey, int nIndentLevel) = 0;
	virtual bool KvWriteValue(KeyValues* pValue, int nIndentLevel) = 0;
	virtual bool KvEndKey(KeyValues* pKey, int nIndentLevel) = 0;
};

class IKeyValuesDumpContextAsText : public IKeyValuesDumpContext
{
public:
	virtual bool KvBeginKey(KeyValues* pKey, int nIndentLevel);
	virtual bool KvWriteValue(KeyValues* pValue, int nIndentLevel);
	virtual bool KvEndKey(KeyValues* pKey, int nIndentLevel);

public:
	virtual bool KvWriteIndent(int nIndentLevel);
	virtual bool KvWriteText(char const* szText) = 0;
};

class CKeyValuesDumpContextAsDevMsg : public IKeyValuesDumpContextAsText
{
public:
	CKeyValuesDumpContextAsDevMsg(int nDeveloperLevel = 1) : m_nDeveloperLevel(nDeveloperLevel) {}

public:
	virtual bool KvBeginKey(KeyValues* pKey, int nIndentLevel);
	virtual bool KvWriteText(char const* szText);

protected:
	int m_nDeveloperLevel;
};

inline bool KeyValuesDumpAsDevMsg(KeyValues* pKeyValues, int nIndentLevel = 0, int nDeveloperLevel = 1)
{
	CKeyValuesDumpContextAsDevMsg ctx(nDeveloperLevel);
	return pKeyValues->Dump(&ctx, nIndentLevel);
}