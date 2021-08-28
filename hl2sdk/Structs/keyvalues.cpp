﻿#include "keyvalues.h"
#include "../interfaces.h"
#include "../definitions.h"
#include "../Utils/utlbuffer.h"
#include "../Utils/utlhash.h"
#include "../Utils/utlqueue.h"
#include "../Utils/utlsortvector.h"
#include "../Utils/threadtools.h"
#include "../../l4d2Simple2/vector.h"

typedef int HKeySymbol;

#define INVALID_KEY_SYMBOL (-1)
#define FILESYSTEM_INVALID_HANDLE ( FileHandle_t )0

static const char * s_LastFileLoadingFrom = "unknown";

int(*KeyValues::s_pfGetSymbolForString)(const char *name, bool bCreate) = &KeyValues::GetSymbolForStringClassic;
const char *(*KeyValues::s_pfGetStringForSymbol)(int symbol) = &KeyValues::GetStringForSymbolClassic;
CKeyValuesGrowableStringTable *KeyValues::s_pGrowableStringTable = NULL;

#define INTERNAL_WRITE( pData, len ) InternalWrite( filesystem, f, pBuf, pData, len )
const int MAX_ERROR_STACK = 64;

#define KEYVALUES_TOKEN_SIZE 4096
static char s_pTokenBuf[KEYVALUES_TOKEN_SIZE];

class CKeyValuesErrorStack
{
public:
	CKeyValuesErrorStack() : m_pFilename("NULL"), m_errorIndex(0), m_maxErrorIndex(0) {}

	void SetFilename(const char* pFilename)
	{
		m_pFilename = pFilename;
		m_maxErrorIndex = 0;
	}

	int Push(int symName)
	{
		if (m_errorIndex < MAX_ERROR_STACK)
		{
			m_errorStack[m_errorIndex] = symName;
		}

		m_errorIndex++;
		m_maxErrorIndex = max(m_maxErrorIndex, (m_errorIndex - 1));
		return m_errorIndex - 1;
	}

	void Pop()
	{
		m_errorIndex--;
		Assert(m_errorIndex >= 0);
	}

	void Reset(int stackLevel, int symName)
	{
		Assert(stackLevel >= 0);
		Assert(stackLevel < m_errorIndex);

		if (stackLevel < MAX_ERROR_STACK)
			m_errorStack[stackLevel] = symName;
	}

	void ReportError(const char* pError)
	{
		bool bSpewCR = false;

		for (int i = 0; i < m_maxErrorIndex; i++)
		{
			if (i < MAX_ERROR_STACK && m_errorStack[i] != INVALID_KEY_SYMBOL)
			{
				if (i < m_errorIndex)
				{
					std::stringstream ss;
					ss << XorStr("%s, ", KeyValues::CallGetStringForSymbol(m_errorStack[i]));
					Utils::logError(ss.str().c_str());
				}
				else
				{
					std::stringstream ss;
					ss << XorStr("(*%s*), ", KeyValues::CallGetStringForSymbol(m_errorStack[i]));
					Utils::logError(ss.str().c_str());
				}

				bSpewCR = true;
			}
		}
	}

private:
	int m_errorStack[MAX_ERROR_STACK];
	const char* m_pFilename;
	int m_errorIndex;
	int m_maxErrorIndex;
} g_KeyValuesErrorStack;

class CKeyErrorContext
{
public:
	CKeyErrorContext(KeyValues* pKv)
	{
		Init(pKv->GetNameSymbol());
	}

	~CKeyErrorContext()
	{
		g_KeyValuesErrorStack.Pop();
	}

	CKeyErrorContext(int symName)
	{
		Init(symName);
	}

	void Reset(int symName)
	{
		g_KeyValuesErrorStack.Reset(m_stackLevel, symName);
	}

	int GetStackLevel() const
	{
		return m_stackLevel;
	}

private:
	void Init(int symName)
	{
		m_stackLevel = g_KeyValuesErrorStack.Push(symName);
	}

	int m_stackLevel;
};

#define LEAKTRACK

#ifdef LEAKTRACK

class CLeakTrack
{
public:
	CLeakTrack()
	{

	}

	~CLeakTrack()
	{
		if (keys.Count() != 0)
		{
			Assert(0);
		}
	}

	struct kve
	{
		KeyValues* kv;
		char name[256];
	};

	void AddKv(KeyValues* kv, char const* name)
	{
		kve k;
		Q_strncpy(k.name, name ? name : "NULL", sizeof(k.name));
		k.kv = kv;

		keys.AddToTail(k);
	}

	void RemoveKv(KeyValues* kv)
	{
		int c = keys.Count();

		for (int i = 0; i < c; i++)
		{
			if (keys[i].kv == kv)
			{
				keys.Remove(i);
				break;
			}
		}
	}

	CUtlVector< kve > keys;
};

static CLeakTrack track;

#define TRACK_KV_ADD( ptr, name ) track.AddKv( ptr, name )
#define TRACK_KV_REMOVE( ptr ) track.RemoveKv( ptr )

#else

#define TRACK_KV_ADD( ptr, name ) 
#define TRACK_KV_REMOVE( ptr )	

#endif

static unsigned g_nRandomValues[256] =
{
	238, 164, 191, 168, 115, 16, 142, 11, 213, 214, 57, 151, 248, 252, 26, 198,
	13, 105, 102, 25, 43, 42, 227, 107, 210, 251, 86, 66, 83, 193, 126, 108,
	131, 3, 64, 186, 192, 81, 37, 158, 39, 244, 14, 254, 75, 30, 2, 88,
	172, 176, 255, 69, 0, 45, 116, 139, 23, 65, 183, 148, 33, 46, 203, 20,
	143, 205, 60, 197, 118, 9, 171, 51, 233, 135, 220, 49, 71, 184, 82, 109,
	36, 161, 169, 150, 63, 96, 173, 125, 113, 67, 224, 78, 232, 215, 35, 219,
	79, 181, 41, 229, 149, 153, 111, 217, 21, 72, 120, 163, 133, 40, 122, 140,
	208, 231, 211, 200, 160, 182, 104, 110, 178, 237, 15, 101, 27, 50, 24, 189,
	177, 130, 187, 92, 253, 136, 100, 212, 19, 174, 70, 22, 170, 206, 162, 74,
	247, 5, 47, 32, 179, 117, 132, 195, 124, 123, 245, 128, 236, 223, 12, 84,
	54, 218, 146, 228, 157, 94, 106, 31, 17, 29, 194, 34, 56, 134, 239, 246,
	241, 216, 127, 98, 7, 204, 154, 152, 209, 188, 48, 61, 87, 97, 225, 85,
	90, 167, 155, 112, 145, 114, 141, 93, 250, 4, 201, 156, 38, 89, 226, 196,
	1, 235, 44, 180, 159, 121, 119, 166, 190, 144, 10, 91, 76, 230, 221, 80,
	207, 55, 58, 53, 175, 8, 6, 52, 68, 242, 18, 222, 103, 249, 147, 129,
	138, 243, 28, 185, 62, 59, 240, 202, 234, 99, 77, 73, 199, 137, 95, 165,
};

unsigned __fastcall HashStringCaseless(const char* pszKey)
{
	const uint8_t* k = (const uint8_t*)pszKey;
	unsigned even = 0, odd = 0, n;

	while ((n = toupper(*k++)) != 0)
	{
		even = g_nRandomValues[odd ^ n];

		if ((n = toupper(*k++)) != 0)
			odd = g_nRandomValues[even ^ n];
		else
			break;
	}

	return (even << 8) | odd;
}

class CKeyValuesGrowableStringTable
{
public:
	CKeyValuesGrowableStringTable() :
#ifdef PLATFORM_64BITS
		m_vecStrings(0, 4 * 512 * 1024)
#else
		m_vecStrings(0, 512 * 1024)
#endif
		, m_hashLookup(2048, 0, 0, m_Functor, m_Functor)
	{
		m_vecStrings.AddToTail('\0');
	}

	int GetSymbolForString(const char* name, bool bCreate = true)
	{
		AUTO_LOCK(m_mutex);

		m_Functor.SetCurString(name);
		m_Functor.SetCurStringBase((const char*)m_vecStrings.Base());

		if (bCreate)
		{
			bool bInserted = false;
			UtlHashHandle_t hElement = m_hashLookup.Insert(-1, &bInserted);

			if (bInserted)
			{
				int iIndex = m_vecStrings.AddMultipleToTail(V_strlen(name) + 1, name);
				m_hashLookup[hElement] = iIndex;
			}

			return m_hashLookup[hElement];
		}
		else
		{
			UtlHashHandle_t hElement = m_hashLookup.Find(-1);

			if (m_hashLookup.IsValidHandle(hElement))
				return m_hashLookup[hElement];
			else
				return -1;
		}
	}

	const char* GetStringForSymbol(int symbol)
	{
		return (const char*)m_vecStrings.Base() + symbol;
	}

private:
	class CLookupFunctor
	{
	public:
		CLookupFunctor() : m_pchCurString(NULL), m_pchCurBase(NULL) {}

		void SetCurString(const char* pchCurString)
		{
			m_pchCurString = pchCurString;
		}

		void SetCurStringBase(const char* pchCurBase)
		{
			m_pchCurBase = pchCurBase;
		}

		bool operator()(int nLhs, int nRhs) const
		{
			const char* pchLhs = nLhs > 0 ? m_pchCurBase + nLhs : m_pchCurString;
			const char* pchRhs = nRhs > 0 ? m_pchCurBase + nRhs : m_pchCurString;

			return (0 == V_stricmp(pchLhs, pchRhs));
		}

		unsigned int operator()(int nItem) const
		{
			return HashStringCaseless(m_pchCurString);
		}

	private:
		const char* m_pchCurString;
		const char* m_pchCurBase;
	};

	CThreadFastMutex m_mutex;
	CLookupFunctor m_Functor;
	CUtlHash<int, CLookupFunctor&, CLookupFunctor&> m_hashLookup;
	CUtlVector<char> m_vecStrings;
};

void KeyValues::SetUseGrowableStringTable(bool bUseGrowableTable)
{
	if (bUseGrowableTable)
	{
		s_pfGetStringForSymbol = &(KeyValues::GetStringForSymbolGrowable);
		s_pfGetSymbolForString = &(KeyValues::GetSymbolForStringGrowable);

		if (NULL == s_pGrowableStringTable)
		{
			s_pGrowableStringTable = new CKeyValuesGrowableStringTable;
		}
	}
	else
	{
		s_pfGetStringForSymbol = &(KeyValues::GetStringForSymbolClassic);
		s_pfGetSymbolForString = &(KeyValues::GetSymbolForStringClassic);

		delete s_pGrowableStringTable;
		s_pGrowableStringTable = NULL;
	}
}

int KeyValues::GetSymbolForStringClassic(const char* name, bool bCreate)
{
	return g_pInterface->KeyValueSystem->GetSymbolForString(name, bCreate);
}

const char* KeyValues::GetStringForSymbolClassic(int symbol)
{
	return g_pInterface->KeyValueSystem->GetStringForSymbol(symbol);
}

int KeyValues::GetSymbolForStringGrowable(const char* name, bool bCreate)
{
	return s_pGrowableStringTable->GetSymbolForString(name, bCreate);
}

const char* KeyValues::GetStringForSymbolGrowable(int symbol)
{
	return s_pGrowableStringTable->GetStringForSymbol(symbol);
}

KeyValues::KeyValues(const char* setName)
{
	TRACK_KV_ADD(this, setName);

	Init();
	SetName(setName);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, const char* firstValue)
{
	TRACK_KV_ADD(this, setName);

	Init();
	SetName(setName);
	SetString(firstKey, firstValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, const wchar_t* firstValue)
{
	TRACK_KV_ADD(this, setName);

	Init();
	SetName(setName);
	SetWString(firstKey, firstValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, int firstValue)
{
	TRACK_KV_ADD(this, setName);

	Init();
	SetName(setName);
	SetInt(firstKey, firstValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, const char* firstValue, const char* secondKey, const char* secondValue)
{
	TRACK_KV_ADD(this, setName);

	Init();
	SetName(setName);
	SetString(firstKey, firstValue);
	SetString(secondKey, secondValue);
}

KeyValues::KeyValues(const char* setName, const char* firstKey, int firstValue, const char* secondKey, int secondValue)
{
	TRACK_KV_ADD(this, setName);

	Init();
	SetName(setName);
	SetInt(firstKey, firstValue);
	SetInt(secondKey, secondValue);
}

void KeyValues::Init()
{
	m_iKeyName = INVALID_KEY_SYMBOL;
	m_iDataType = TYPE_NONE;

	m_pSub = NULL;
	m_pPeer = NULL;
	m_pChain = NULL;

	m_sValue = NULL;
	m_wsValue = NULL;
	m_pValue = NULL;

	m_bHasEscapeSequences = false;
	m_bEvaluateConditionals = true;

	memset(unused, 0, sizeof(unused));
}

KeyValues::~KeyValues()
{
	TRACK_KV_REMOVE(this);
	RemoveEverything();
}

void KeyValues::RemoveEverything()
{
	KeyValues* dat;
	KeyValues* datNext = NULL;

	for (dat = m_pSub; dat != NULL; dat = datNext)
	{
		datNext = dat->m_pPeer;
		dat->m_pPeer = NULL;
		delete dat;
	}

	for (dat = m_pPeer; dat && dat != this; dat = datNext)
	{
		datNext = dat->m_pPeer;
		dat->m_pPeer = NULL;
		delete dat;
	}

	delete[] m_sValue;
	m_sValue = NULL;
	delete[] m_wsValue;
	m_wsValue = NULL;
}

void KeyValues::RecursiveSaveToFile(CUtlBuffer& buf, int indentLevel, bool sortKeys /*= false*/, bool bAllowEmptyString /*= false*/)
{
	RecursiveSaveToFile(NULL, FILESYSTEM_INVALID_HANDLE, &buf, indentLevel, sortKeys, bAllowEmptyString);
}

void KeyValues::ChainKeyValue(KeyValues* pChain)
{
	m_pChain = pChain;
}

const char* KeyValues::GetName(void) const
{
	return s_pfGetStringForSymbol(m_iKeyName);
}

#pragma warning (disable:4706)
const char* KeyValues::ReadToken(CUtlBuffer& buf, bool& wasQuoted, bool& wasConditional)
{
	wasQuoted = false;
	wasConditional = false;

	if (!buf.IsValid())
		return NULL;

	while (true)
	{
		buf.EatWhiteSpace();

		if (!buf.IsValid())
			return NULL;

		if (!buf.EatCPPComment())
			break;
	}

	const char* c = (const char*)buf.PeekGet(sizeof(char), 0);

	if (!c)
		return NULL;

	if (*c == '\"')
	{
		wasQuoted = true;
		buf.GetDelimitedString(m_bHasEscapeSequences ? GetCStringCharConversion() : GetNoEscCharConversion(),
			s_pTokenBuf, KEYVALUES_TOKEN_SIZE);
		return s_pTokenBuf;
	}

	if (*c == '{' || *c == '}')
	{
		s_pTokenBuf[0] = *c;
		s_pTokenBuf[1] = 0;
		buf.SeekGet(CUtlBuffer::SEEK_CURRENT, 1);
		return s_pTokenBuf;
	}

	bool bReportedError = false;
	bool bConditionalStart = false;
	int nCount = 0;

	while ((c = (const char*)buf.PeekGet(sizeof(char), 0)))
	{
		if (*c == 0)
			break;

		if (*c == '"' || *c == '{' || *c == '}')
			break;

		if (*c == '[')
			bConditionalStart = true;

		if (*c == ']' && bConditionalStart)
		{
			wasConditional = true;
		}

		if (isspace(*c))
			break;

		if (nCount < (KEYVALUES_TOKEN_SIZE - 1))
		{
			s_pTokenBuf[nCount++] = *c;
		}
		else if (!bReportedError)
		{
			bReportedError = true;
			g_KeyValuesErrorStack.ReportError(" ReadToken overflow");
		}

		buf.SeekGet(CUtlBuffer::SEEK_CURRENT, 1);
	}

	s_pTokenBuf[nCount] = 0;
	return s_pTokenBuf;
}
#pragma warning (default:4706)

void KeyValues::UsesEscapeSequences(bool state)
{
	m_bHasEscapeSequences = state;
}

void KeyValues::UsesConditionals(bool state)
{
	m_bEvaluateConditionals = state;
}

bool KeyValues::LoadFromFile(IBaseFileSystem* filesystem, const char* resourceName, const char* pathID, bool refreshCache)
{
	Assert(filesystem);
#ifdef WIN32
	Assert(IsPC() && _heapchk() == _HEAPOK);
#endif

#ifdef STAGING_ONLY
	static bool s_bCacheEnabled = !!CommandLine()->FindParm("-enable_keyvalues_cache");
	const bool bUseCache = s_bCacheEnabled && (s_pfGetSymbolForString == KeyValues::GetSymbolForStringClassic);
#else

	const bool bUseCache = false;
#endif

	const bool bUseCacheForRead = bUseCache && !refreshCache && pathID != NULL;
	const bool bUseCacheForWrite = bUseCache && pathID != NULL;

	if (bUseCacheForRead && g_pInterface->KeyValueSystem->LoadFileKeyValuesFromCache(this, resourceName, pathID, filesystem))
	{
		return true;
	}

	FileHandle_t f = filesystem->Open(resourceName, "rb", pathID);

	if (!f)
	{
		return false;
	}

	s_LastFileLoadingFrom = (char*)resourceName;

	int fileSize = filesystem->Size(f);
	unsigned bufSize = ((IFileSystem*)filesystem)->GetOptimalReadSize(f, fileSize + 2);

	char* buffer = (char*)((IFileSystem*)filesystem)->AllocOptimalReadBuffer(f, bufSize);
	Assert(buffer);

	bool bRetOK = (((IFileSystem*)filesystem)->ReadEx(buffer, bufSize, fileSize, f) != 0);

	filesystem->Close(f);

	if (bRetOK)
	{
		buffer[fileSize] = 0;
		buffer[fileSize + 1] = 0;
		bRetOK = LoadFromBuffer(resourceName, buffer, filesystem);
	}

	if (bUseCacheForWrite && bRetOK)
	{
		g_pInterface->KeyValueSystem->AddFileKeyValuesToCache(this, resourceName, pathID);
	}

	((IFileSystem*)filesystem)->FreeOptimalReadBuffer(buffer);
	return bRetOK;
}

bool KeyValues::SaveToFile(IBaseFileSystem* filesystem, const char* resourceName, const char* pathID, bool sortKeys /*= false*/, bool bAllowEmptyString /*= false*/, bool bCacheResult /*= false*/)
{
	FileHandle_t f = filesystem->Open(resourceName, "wb", pathID);

	if (f == FILESYSTEM_INVALID_HANDLE)
	{
		std::stringstream ss;

		ss << XorStr("KeyValues::SaveToFile: couldn't open file \"%s\" in path \"%s\"\n",
			resourceName ? resourceName : "NULL", pathID ? pathID : "NULL");

		Utils::logError(ss.str().c_str());
		return false;
	}

	g_pInterface->KeyValueSystem->InvalidateCacheForFile(resourceName, pathID);

	if (bCacheResult)
	{
		g_pInterface->KeyValueSystem->AddFileKeyValuesToCache(this, resourceName, pathID);
	}

	RecursiveSaveToFile(filesystem, f, NULL, 0, sortKeys, bAllowEmptyString);
	filesystem->Close(f);

	return true;
}

void KeyValues::WriteIndents(IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, int indentLevel)
{
	for (int i = 0; i < indentLevel; i++)
	{
		INTERNAL_WRITE("\t", 1);
	}
}

void KeyValues::WriteConvertedString(IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, const char* pszString)
{
	int len = Q_strlen(pszString);
	char* convertedString = (char*)_alloca((len + 1) * sizeof(char) * 2);
	int j = 0;

	for (int i = 0; i <= len; i++)
	{
		if (pszString[i] == '\"')
		{
			convertedString[j] = '\\';
			j++;
		}
		else if (m_bHasEscapeSequences && pszString[i] == '\\')
		{
			convertedString[j] = '\\';
			j++;
		}

		convertedString[j] = pszString[i];
		j++;
	}

	INTERNAL_WRITE(convertedString, Q_strlen(convertedString));
}

void KeyValues::InternalWrite(IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, const void* pData, int len)
{
	if (filesystem)
	{
		filesystem->Write(pData, len, f);
	}

	if (pBuf)
	{
		pBuf->Put(pData, len);
	}
}

void KeyValues::RecursiveSaveToFile(IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, int indentLevel, bool sortKeys, bool bAllowEmptyString)
{
	WriteIndents(filesystem, f, pBuf, indentLevel);
	INTERNAL_WRITE("\"", 1);
	WriteConvertedString(filesystem, f, pBuf, GetName());
	INTERNAL_WRITE("\"\n", 2);
	WriteIndents(filesystem, f, pBuf, indentLevel);
	INTERNAL_WRITE("{\n", 2);

	if (sortKeys)
	{
		CUtlSortVector< KeyValues*, CUtlSortVectorKeyValuesByName > vecSortedKeys;

		for (KeyValues* dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
		{
			vecSortedKeys.InsertNoSort(dat);
		}

		vecSortedKeys.RedoSort();

		FOR_EACH_VEC(vecSortedKeys, i)
		{
			SaveKeyToFile(vecSortedKeys[i], filesystem, f, pBuf, indentLevel, sortKeys, bAllowEmptyString);
		}
	}
	else
	{
		for (KeyValues* dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
			SaveKeyToFile(dat, filesystem, f, pBuf, indentLevel, sortKeys, bAllowEmptyString);
	}

	WriteIndents(filesystem, f, pBuf, indentLevel);
	INTERNAL_WRITE("}\n", 2);
}

void KeyValues::SaveKeyToFile(KeyValues* dat, IBaseFileSystem* filesystem, FileHandle_t f, CUtlBuffer* pBuf, int indentLevel, bool sortKeys, bool bAllowEmptyString)
{
	if (dat->m_pSub)
	{
		dat->RecursiveSaveToFile(filesystem, f, pBuf, indentLevel + 1, sortKeys, bAllowEmptyString);
	}
	else
	{
		switch (dat->m_iDataType)
		{
		case TYPE_STRING:
		{
			if (dat->m_sValue && (bAllowEmptyString || *(dat->m_sValue)))
			{
				WriteIndents(filesystem, f, pBuf, indentLevel + 1);
				INTERNAL_WRITE("\"", 1);
				WriteConvertedString(filesystem, f, pBuf, dat->GetName());
				INTERNAL_WRITE("\"\t\t\"", 4);
				WriteConvertedString(filesystem, f, pBuf, dat->m_sValue);
				INTERNAL_WRITE("\"\n", 2);
			}
			break;
		}
		case TYPE_WSTRING:
		{
			if (dat->m_wsValue)
			{
				static char buf[KEYVALUES_TOKEN_SIZE];
				int result = Q_UnicodeToUTF8(dat->m_wsValue, buf, KEYVALUES_TOKEN_SIZE);

				if (result)
				{
					WriteIndents(filesystem, f, pBuf, indentLevel + 1);
					INTERNAL_WRITE("\"", 1);
					INTERNAL_WRITE(dat->GetName(), Q_strlen(dat->GetName()));
					INTERNAL_WRITE("\"\t\t\"", 4);
					WriteConvertedString(filesystem, f, pBuf, buf);
					INTERNAL_WRITE("\"\n", 2);
				}
			}
			break;
		}
		case TYPE_INT:
		{
			WriteIndents(filesystem, f, pBuf, indentLevel + 1);
			INTERNAL_WRITE("\"", 1);
			INTERNAL_WRITE(dat->GetName(), Q_strlen(dat->GetName()));
			INTERNAL_WRITE("\"\t\t\"", 4);

			char buf[32];
			Q_snprintf(buf, sizeof(buf), "%d", dat->m_iValue);

			INTERNAL_WRITE(buf, Q_strlen(buf));
			INTERNAL_WRITE("\"\n", 2);
			break;
		}
		case TYPE_UINT64:
		{
			WriteIndents(filesystem, f, pBuf, indentLevel + 1);
			INTERNAL_WRITE("\"", 1);
			INTERNAL_WRITE(dat->GetName(), Q_strlen(dat->GetName()));
			INTERNAL_WRITE("\"\t\t\"", 4);

			char buf[32];
#ifdef WIN32
			Q_snprintf(buf, sizeof(buf), "0x%016I64X", *((uint64_t*)dat->m_sValue));
#else
			Q_snprintf(buf, sizeof(buf), "0x%016llX", *((uint64*)dat->m_sValue));
#endif
			INTERNAL_WRITE(buf, Q_strlen(buf));
			INTERNAL_WRITE("\"\n", 2);
			break;
		}

		case TYPE_FLOAT:
		{
			WriteIndents(filesystem, f, pBuf, indentLevel + 1);
			INTERNAL_WRITE("\"", 1);
			INTERNAL_WRITE(dat->GetName(), Q_strlen(dat->GetName()));
			INTERNAL_WRITE("\"\t\t\"", 4);

			char buf[48];
			Q_snprintf(buf, sizeof(buf), "%f", dat->m_flValue);

			INTERNAL_WRITE(buf, Q_strlen(buf));
			INTERNAL_WRITE("\"\n", 2);
			break;
		}
		case TYPE_COLOR:
		{
			std::stringstream ss;
			ss << XorStr("KeyValues::RecursiveSaveToFile: TODO, missing code for TYPE_COLOR\n");
			Utils::log(ss.str().c_str());
			break;
		}
		default:
			break;
		}
	}
}

KeyValues* KeyValues::FindKey(int keySymbol) const
{
	for (KeyValues* dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		if (dat->m_iKeyName == keySymbol)
			return dat;
	}

	return NULL;
}

KeyValues* KeyValues::FindKey(const char* keyName, bool bCreate)
{
	if (!keyName || !keyName[0])
		return this;

	char szBuf[256];
	const char* subStr = strchr(keyName, '/');
	const char* searchStr = keyName;

	if (subStr)
	{
		int size = subStr - keyName;
		Q_memcpy(szBuf, keyName, size);
		szBuf[size] = 0;
		searchStr = szBuf;
	}

	HKeySymbol iSearchStr = s_pfGetSymbolForString(searchStr, bCreate);

	if (iSearchStr == INVALID_KEY_SYMBOL)
	{
		return NULL;
	}

	KeyValues* lastItem = NULL;
	KeyValues* dat;

	for (dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		lastItem = dat;

		if (dat->m_iKeyName == iSearchStr)
		{
			break;
		}
	}

	if (!dat && m_pChain)
	{
		dat = m_pChain->FindKey(keyName, false);
	}

	if (!dat)
	{
		if (bCreate)
		{
			dat = new KeyValues(searchStr);

			dat->UsesEscapeSequences(m_bHasEscapeSequences != 0);
			dat->UsesConditionals(m_bEvaluateConditionals != 0);

			if (lastItem)
			{
				lastItem->m_pPeer = dat;
			}
			else
			{
				m_pSub = dat;
			}

			dat->m_pPeer = NULL;
			m_iDataType = TYPE_NONE;
		}
		else
		{
			return NULL;
		}
	}

	if (subStr)
	{
		return dat->FindKey(subStr + 1, bCreate);
	}

	return dat;
}

KeyValues* KeyValues::CreateNewKey()
{
	int newID = 1;
	KeyValues* pLastChild = NULL;

	for (KeyValues* dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		int val = atoi(dat->GetName());

		if (newID <= val)
		{
			newID = val + 1;
		}

		pLastChild = dat;
	}

	char buf[12];
	Q_snprintf(buf, sizeof(buf), "%d", newID);

	return CreateKeyUsingKnownLastChild(buf, pLastChild);
}

KeyValues* KeyValues::CreateKey(const char* keyName)
{
	KeyValues* pLastChild = FindLastSubKey();
	return CreateKeyUsingKnownLastChild(keyName, pLastChild);
}

KeyValues* KeyValues::CreateKeyUsingKnownLastChild(const char* keyName, KeyValues* pLastChild)
{
	KeyValues* dat = new KeyValues(keyName);

	dat->UsesEscapeSequences(m_bHasEscapeSequences != 0);
	dat->UsesConditionals(m_bEvaluateConditionals != 0);

	AddSubkeyUsingKnownLastChild(dat, pLastChild);
	return dat;
}

void KeyValues::AddSubkeyUsingKnownLastChild(KeyValues* pSubkey, KeyValues* pLastChild)
{
	Assert(pSubkey != NULL);
	Assert(pSubkey->m_pPeer == NULL);

	if (pLastChild == NULL)
	{
		Assert(m_pSub == NULL);
		m_pSub = pSubkey;
	}
	else
	{
		Assert(m_pSub != NULL);
		Assert(pLastChild->m_pPeer == NULL);
		pLastChild->SetNextKey(pSubkey);
	}
}

void KeyValues::AddSubKey(KeyValues* pSubkey)
{
	Assert(pSubkey != NULL);
	Assert(pSubkey->m_pPeer == NULL);

	if (m_pSub == NULL)
	{
		m_pSub = pSubkey;
	}
	else
	{
		KeyValues* pTempDat = m_pSub;

		while (pTempDat->GetNextKey() != NULL)
		{
			pTempDat = pTempDat->GetNextKey();
		}

		pTempDat->SetNextKey(pSubkey);
	}
}

void KeyValues::RemoveSubKey(KeyValues* subKey)
{
	if (!subKey)
		return;

	if (m_pSub == subKey)
	{
		m_pSub = subKey->m_pPeer;
	}
	else
	{
		KeyValues* kv = m_pSub;

		while (kv->m_pPeer)
		{
			if (kv->m_pPeer == subKey)
			{
				kv->m_pPeer = subKey->m_pPeer;
				break;
			}

			kv = kv->m_pPeer;
		}
	}

	subKey->m_pPeer = NULL;
}

KeyValues* KeyValues::FindLastSubKey()
{

	if (m_pSub == NULL)
		return NULL;

	KeyValues* pLastChild = m_pSub;

	while (pLastChild->m_pPeer)
		pLastChild = pLastChild->m_pPeer;

	return pLastChild;
}

void KeyValues::SetNextKey(KeyValues* pDat)
{
	m_pPeer = pDat;
}

KeyValues* KeyValues::GetFirstTrueSubKey()
{
	KeyValues* pRet = m_pSub;

	while (pRet && pRet->m_iDataType != TYPE_NONE)
		pRet = pRet->m_pPeer;

	return pRet;
}

KeyValues* KeyValues::GetNextTrueSubKey()
{
	KeyValues* pRet = m_pPeer;

	while (pRet && pRet->m_iDataType != TYPE_NONE)
		pRet = pRet->m_pPeer;

	return pRet;
}

KeyValues* KeyValues::GetFirstValue()
{
	KeyValues* pRet = m_pSub;

	while (pRet && pRet->m_iDataType == TYPE_NONE)
		pRet = pRet->m_pPeer;

	return pRet;
}

KeyValues* KeyValues::GetNextValue()
{
	KeyValues* pRet = m_pPeer;

	while (pRet && pRet->m_iDataType == TYPE_NONE)
		pRet = pRet->m_pPeer;

	return pRet;
}

int KeyValues::GetInt(const char* keyName, int defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_STRING:
			return atoi(dat->m_sValue);
		case TYPE_WSTRING:
			return _wtoi(dat->m_wsValue);
		case TYPE_FLOAT:
			return (int)dat->m_flValue;
		case TYPE_UINT64:
			Assert(0);
			return 0;
		case TYPE_INT:
		case TYPE_PTR:
		default:
			return dat->m_iValue;
		};
	}

	return defaultValue;
}

uint64_t KeyValues::GetUint64(const char* keyName, uint64_t defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_STRING:
			return (uint64_t)V_atoi64(dat->m_sValue);
		case TYPE_WSTRING:
			return _wtoi64(dat->m_wsValue);
		case TYPE_FLOAT:
			return (int)dat->m_flValue;
		case TYPE_UINT64:
			return *((uint64_t*)dat->m_sValue);
		case TYPE_INT:
		case TYPE_PTR:
		default:
			return dat->m_iValue;
		};
	}

	return defaultValue;
}

void* KeyValues::GetPtr(const char* keyName, void* defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_PTR:
			return dat->m_pValue;

		case TYPE_WSTRING:
		case TYPE_STRING:
		case TYPE_FLOAT:
		case TYPE_INT:
		case TYPE_UINT64:
		default:
			return NULL;
		};
	}

	return defaultValue;
}

float KeyValues::GetFloat(const char* keyName, float defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_STRING:
			return (float)atof(dat->m_sValue);
		case TYPE_WSTRING:
#ifdef WIN32
			return (float)_wtof(dat->m_wsValue);
#else
			Assert(!"impl me");
			return 0.0;
#endif
		case TYPE_FLOAT:
			return dat->m_flValue;
		case TYPE_INT:
			return (float)dat->m_iValue;
		case TYPE_UINT64:
			return (float)(*((uint64_t*)dat->m_sValue));
		case TYPE_PTR:
		default:
			return 0.0f;
		};
	}

	return defaultValue;
}

const char* KeyValues::GetString(const char* keyName, const char* defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
	{
		char buf[64];

		switch (dat->m_iDataType)
		{
		case TYPE_FLOAT:
			Q_snprintf(buf, sizeof(buf), "%f", dat->m_flValue);
			SetString(keyName, buf);
			break;
		case TYPE_PTR:
			Q_snprintf(buf, sizeof(buf), "%lld", (int64_t)(size_t)dat->m_pValue);
			SetString(keyName, buf);
			break;
		case TYPE_INT:
			Q_snprintf(buf, sizeof(buf), "%d", dat->m_iValue);
			SetString(keyName, buf);
			break;
		case TYPE_UINT64:
			Q_snprintf(buf, sizeof(buf), "%lld", *((uint64_t*)(dat->m_sValue)));
			SetString(keyName, buf);
			break;

		case TYPE_WSTRING:
		{
			char wideBuf[512];
			int result = Q_UnicodeToUTF8(dat->m_wsValue, wideBuf, 512);

			if (result)
			{
				SetString(keyName, wideBuf);
			}
			else
			{
				return defaultValue;
			}
			break;
		}
		case TYPE_STRING:
			break;
		default:
			return defaultValue;
		};

		return dat->m_sValue;
	}

	return defaultValue;
}

const wchar_t* KeyValues::GetWString(const char* keyName, const wchar_t* defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
	{
		wchar_t wbuf[64];

		switch (dat->m_iDataType)
		{
		case TYPE_FLOAT:
			swprintf(wbuf, ARRAYSIZE(wbuf), L"%f", dat->m_flValue);
			SetWString(keyName, wbuf);
			break;
		case TYPE_PTR:
			swprintf(wbuf, ARRAYSIZE(wbuf), L"%lld", (int64_t)(size_t)dat->m_pValue);
			SetWString(keyName, wbuf);
			break;
		case TYPE_INT:
			swprintf(wbuf, ARRAYSIZE(wbuf), L"%d", dat->m_iValue);
			SetWString(keyName, wbuf);
			break;
		case TYPE_UINT64:
		{
			swprintf(wbuf, ARRAYSIZE(wbuf), L"%lld", *((uint64_t*)(dat->m_sValue)));
			SetWString(keyName, wbuf);
		}
		break;

		case TYPE_WSTRING:
			break;
		case TYPE_STRING:
		{
			int bufSize = Q_strlen(dat->m_sValue) + 1;
			wchar_t* pWBuf = new wchar_t[bufSize];
			int result = Q_UTF8ToUnicode(dat->m_sValue, pWBuf, bufSize * sizeof(wchar_t));

			if (result >= 0)
			{
				SetWString(keyName, pWBuf);
			}
			else
			{
				delete[] pWBuf;
				return defaultValue;
			}
			delete[] pWBuf;
			break;
		}
		default:
			return defaultValue;
		};

		return (const wchar_t*)dat->m_wsValue;
	}

	return defaultValue;
}

bool KeyValues::GetBool(const char* keyName, bool defaultValue, bool* optGotDefault)
{
	if (FindKey(keyName))
	{
		if (optGotDefault)
			(*optGotDefault) = false;

		return 0 != GetInt(keyName, 0);
	}

	if (optGotDefault)
		(*optGotDefault) = true;

	return defaultValue;
}

Color KeyValues::GetColor(const char* keyName)
{
	Color color(0, 0, 0, 0);
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
	{
		if (dat->m_iDataType == TYPE_COLOR)
		{
			color[0] = dat->m_Color[0];
			color[1] = dat->m_Color[1];
			color[2] = dat->m_Color[2];
			color[3] = dat->m_Color[3];
		}
		else if (dat->m_iDataType == TYPE_FLOAT)
		{
			color[0] = static_cast<byte>(dat->m_flValue);
		}
		else if (dat->m_iDataType == TYPE_INT)
		{
			color[0] = dat->m_iValue;
		}
		else if (dat->m_iDataType == TYPE_STRING)
		{
			float a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f;
			sscanf(dat->m_sValue, "%f %f %f %f", &a, &b, &c, &d);
			color[0] = (unsigned char)a;
			color[1] = (unsigned char)b;
			color[2] = (unsigned char)c;
			color[3] = (unsigned char)d;
		}
	}

	return color;
}

void KeyValues::SetColor(const char* keyName, Color value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_iDataType = TYPE_COLOR;
		dat->m_Color[0] = value[0];
		dat->m_Color[1] = value[1];
		dat->m_Color[2] = value[2];
		dat->m_Color[3] = value[3];
	}
}

void KeyValues::SetStringValue(char const* strValue)
{
	delete[] m_sValue;
	delete[] m_wsValue;
	m_wsValue = NULL;

	if (!strValue)
	{
		strValue = "";
	}

	int len = Q_strlen(strValue);
	m_sValue = new char[len + 1];
	Q_memcpy(m_sValue, strValue, len + 1);
	m_iDataType = TYPE_STRING;
}

void KeyValues::SetString(const char* keyName, const char* value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		if (dat->m_iDataType == TYPE_STRING && dat->m_sValue == value)
		{
			return;
		}

		delete[] dat->m_sValue;
		delete[] dat->m_wsValue;
		dat->m_wsValue = NULL;

		if (!value)
		{
			value = "";
		}

		int len = Q_strlen(value);
		dat->m_sValue = new char[len + 1];
		Q_memcpy(dat->m_sValue, value, len + 1);
		dat->m_iDataType = TYPE_STRING;
	}
}

void KeyValues::SetWString(const char* keyName, const wchar_t* value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		delete[] dat->m_wsValue;
		delete[] dat->m_sValue;
		dat->m_sValue = NULL;

		if (!value)
		{
			value = L"";
		}

		int len = Q_wcslen(value);
		dat->m_wsValue = new wchar_t[len + 1];
		Q_memcpy(dat->m_wsValue, value, (len + 1) * sizeof(wchar_t));

		dat->m_iDataType = TYPE_WSTRING;
	}
}

void KeyValues::SetInt(const char* keyName, int value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_iValue = value;
		dat->m_iDataType = TYPE_INT;
	}
}

void KeyValues::SetUint64(const char* keyName, uint64_t value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		delete[] dat->m_sValue;
		delete[] dat->m_wsValue;
		dat->m_wsValue = NULL;

		dat->m_sValue = new char[sizeof(uint64_t)];
		*((uint64_t*)dat->m_sValue) = value;
		dat->m_iDataType = TYPE_UINT64;
	}
}

void KeyValues::SetFloat(const char* keyName, float value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_flValue = value;
		dat->m_iDataType = TYPE_FLOAT;
	}
}

void KeyValues::SetName(const char* setName)
{
	m_iKeyName = s_pfGetSymbolForString(setName, true);
}

void KeyValues::SetPtr(const char* keyName, void* value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_pValue = value;
		dat->m_iDataType = TYPE_PTR;
	}
}

void KeyValues::CopyKeyValuesFromRecursive(const KeyValues& rootSrc)
{
	struct CopyStruct
	{
		KeyValues* dst;
		const KeyValues* src;
	};

	char tmp[256];
	KeyValues* localDst = NULL;

	CUtlQueue<CopyStruct> nodeQ;
	nodeQ.Insert({ this, &rootSrc });

	while (nodeQ.Count() > 0)
	{
		CopyStruct cs = nodeQ.RemoveAtHead();

		while (cs.src)
		{
			Assert((cs.src != NULL) == (cs.dst != NULL));
			cs.dst->CopyKeyValue(*cs.src, sizeof(tmp), tmp);

			if (cs.src->m_pSub)
			{
				cs.dst->m_pSub = localDst = new KeyValues(NULL);
				nodeQ.Insert({ localDst, cs.src->m_pSub });
			}

			if (cs.src->m_pPeer)
			{
				cs.dst->m_pPeer = new KeyValues(NULL);
			}
			else
			{
				cs.dst->m_pPeer = NULL;
			}

			cs.src = cs.src->m_pPeer;
			cs.dst = cs.dst->m_pPeer;
		}
	}
}

void KeyValues::CopyKeyValue(const KeyValues& src, size_t tmpBufferSizeB, char* tmpBuffer)
{
	m_iKeyName = src.GetNameSymbol();

	if (src.m_pSub)
		return;

	m_iDataType = src.m_iDataType;

	switch (src.m_iDataType)
	{
	case TYPE_NONE:
		break;
	case TYPE_STRING:
		if (src.m_sValue)
		{
			int len = Q_strlen(src.m_sValue) + 1;
			m_sValue = new char[len];
			Q_strncpy(m_sValue, src.m_sValue, len);
		}
		break;
	case TYPE_INT:
	{
		m_iValue = src.m_iValue;
		Q_snprintf(tmpBuffer, tmpBufferSizeB, "%d", m_iValue);
		int len = Q_strlen(tmpBuffer) + 1;
		m_sValue = new char[len];
		Q_strncpy(m_sValue, tmpBuffer, len);
	}
	break;
	case TYPE_FLOAT:
	{
		m_flValue = src.m_flValue;
		Q_snprintf(tmpBuffer, tmpBufferSizeB, "%f", m_flValue);
		int len = Q_strlen(tmpBuffer) + 1;
		m_sValue = new char[len];
		Q_strncpy(m_sValue, tmpBuffer, len);
	}
	break;
	case TYPE_PTR:
	{
		m_pValue = src.m_pValue;
	}
	break;
	case TYPE_UINT64:
	{
		m_sValue = new char[sizeof(uint64_t)];
		Q_memcpy(m_sValue, src.m_sValue, sizeof(uint64_t));
	}
	break;
	case TYPE_COLOR:
	{
		m_Color[0] = src.m_Color[0];
		m_Color[1] = src.m_Color[1];
		m_Color[2] = src.m_Color[2];
		m_Color[3] = src.m_Color[3];
	}
	break;

	default:
	{
		Assert(0);
	}
	break;
	}
}

KeyValues& KeyValues::operator=(const KeyValues& src)
{
	RemoveEverything();
	Init();
	CopyKeyValuesFromRecursive(src);
	return *this;
}

void KeyValues::CopySubkeys(KeyValues* pParent) const
{
	KeyValues* pPrev = NULL;

	for (KeyValues* sub = m_pSub; sub != NULL; sub = sub->m_pPeer)
	{
		KeyValues* dat = sub->MakeCopy();

		if (pPrev)
		{
			pPrev->m_pPeer = dat;
		}
		else
		{
			pParent->m_pSub = dat;
		}

		dat->m_pPeer = NULL;
		pPrev = dat;
	}
}

KeyValues* KeyValues::MakeCopy(void) const
{
	KeyValues* newKeyValue = new KeyValues(GetName());

	newKeyValue->UsesEscapeSequences(m_bHasEscapeSequences != 0);
	newKeyValue->UsesConditionals(m_bEvaluateConditionals != 0);
	newKeyValue->m_iDataType = m_iDataType;

	switch (m_iDataType)
	{
	case TYPE_STRING:
	{
		if (m_sValue)
		{
			int len = Q_strlen(m_sValue);
			Assert(!newKeyValue->m_sValue);
			newKeyValue->m_sValue = new char[len + 1];
			Q_memcpy(newKeyValue->m_sValue, m_sValue, len + 1);
		}
	}
	break;
	case TYPE_WSTRING:
	{
		if (m_wsValue)
		{
			int len = Q_wcslen(m_wsValue);
			newKeyValue->m_wsValue = new wchar_t[len + 1];
			Q_memcpy(newKeyValue->m_wsValue, m_wsValue, (len + 1) * sizeof(wchar_t));
		}
	}
	break;

	case TYPE_INT:
		newKeyValue->m_iValue = m_iValue;
		break;

	case TYPE_FLOAT:
		newKeyValue->m_flValue = m_flValue;
		break;

	case TYPE_PTR:
		newKeyValue->m_pValue = m_pValue;
		break;

	case TYPE_COLOR:
		newKeyValue->m_Color[0] = m_Color[0];
		newKeyValue->m_Color[1] = m_Color[1];
		newKeyValue->m_Color[2] = m_Color[2];
		newKeyValue->m_Color[3] = m_Color[3];
		break;

	case TYPE_UINT64:
		newKeyValue->m_sValue = new char[sizeof(uint64_t)];
		Q_memcpy(newKeyValue->m_sValue, m_sValue, sizeof(uint64_t));
		break;
	};

	CopySubkeys(newKeyValue);
	return newKeyValue;
}

KeyValues* KeyValues::MakeCopy(bool copySiblings) const
{
	KeyValues* rootDest = MakeCopy();

	if (!copySiblings)
		return rootDest;

	const KeyValues* curSrc = GetNextKey();
	KeyValues* curDest = rootDest;

	while (curSrc)
	{
		curDest->SetNextKey(curSrc->MakeCopy());
		curDest = curDest->GetNextKey();
		curSrc = curSrc->GetNextKey();
	}

	return rootDest;
}

bool KeyValues::IsEmpty(const char* keyName)
{
	KeyValues* dat = FindKey(keyName, false);

	if (!dat)
		return true;

	if (dat->m_iDataType == TYPE_NONE && dat->m_pSub == NULL)
		return true;

	return false;
}

void KeyValues::Clear(void)
{
	delete m_pSub;
	m_pSub = NULL;
	m_iDataType = TYPE_NONE;
}

KeyValues::types_t KeyValues::GetDataType(const char* keyName)
{
	KeyValues* dat = FindKey(keyName, false);

	if (dat)
		return (types_t)dat->m_iDataType;

	return TYPE_NONE;
}

void KeyValues::deleteThis()
{
	delete this;
}

void KeyValues::AppendIncludedKeys(CUtlVector< KeyValues* >& includedKeys)
{
	KeyValues* insertSpot = this;
	int includeCount = includedKeys.Count();

	for (int i = 0; i < includeCount; i++)
	{
		KeyValues* kv = includedKeys[i];
		Assert(kv);

		while (insertSpot->GetNextKey())
		{
			insertSpot = insertSpot->GetNextKey();
		}

		insertSpot->SetNextKey(kv);
	}
}

void KeyValues::ParseIncludedKeys(char const* resourceName, const char* filetoinclude,
	IBaseFileSystem* pFileSystem, const char* pPathID, CUtlVector< KeyValues* >& includedKeys)
{
	Assert(resourceName);
	Assert(filetoinclude);
	Assert(pFileSystem);

	if (!pFileSystem)
	{
		return;
	}

	char fullpath[512];
	Q_strncpy(fullpath, resourceName, sizeof(fullpath));
	int len = Q_strlen(fullpath);

	for (;;)
	{
		if (len <= 0)
		{
			break;
		}

		if (fullpath[len - 1] == '\\' ||
			fullpath[len - 1] == '/')
		{
			break;
		}

		fullpath[len - 1] = 0;
		--len;
	}

	Q_strncat(fullpath, filetoinclude, sizeof(fullpath), COPY_ALL_CHARACTERS);
	KeyValues* newKV = new KeyValues(fullpath);

	newKV->UsesEscapeSequences(m_bHasEscapeSequences != 0);
	newKV->UsesConditionals(m_bEvaluateConditionals != 0);

	if (newKV->LoadFromFile(pFileSystem, fullpath, pPathID))
	{
		includedKeys.AddToTail(newKV);
	}
	else
	{
		std::stringstream ss;
		ss << XorStr("KeyValues::ParseIncludedKeys: Couldn't load included keyvalue file %s\n", fullpath);

		Utils::logError(ss.str().c_str());
		newKV->deleteThis();
	}
}

void KeyValues::MergeBaseKeys(CUtlVector< KeyValues* >& baseKeys)
{
	int includeCount = baseKeys.Count();
	int i;

	for (i = 0; i < includeCount; i++)
	{
		KeyValues* kv = baseKeys[i];
		Assert(kv);
		RecursiveMergeKeyValues(kv);
	}
}

void KeyValues::RecursiveMergeKeyValues(KeyValues* baseKV)
{
	for (KeyValues* baseChild = baseKV->m_pSub; baseChild != NULL; baseChild = baseChild->m_pPeer)
	{
		bool bFoundMatch = false;

		for (KeyValues* newChild = m_pSub; newChild != NULL; newChild = newChild->m_pPeer)
		{
			if (!Q_strcmp(baseChild->GetName(), newChild->GetName()))
			{
				newChild->RecursiveMergeKeyValues(baseChild);
				bFoundMatch = true;
				break;
			}
		}

		if (!bFoundMatch)
		{
			KeyValues* dat = baseChild->MakeCopy();
			Assert(dat);
			AddSubKey(dat);
		}
	}
}

bool EvaluateConditional(const char* str)
{
	if (!str)
		return false;

	if (*str == '[')
		str++;

	bool bNot = false;

	if (*str == '!')
		bNot = true;

	if (Q_stristr(str, "$X360"))
		return false ^ bNot;

	if (Q_stristr(str, "$WIN32"))
		return true ^ bNot;

	if (Q_stristr(str, "$WINDOWS"))
		return true ^ bNot;

	if (Q_stristr(str, "$OSX"))
		return false ^ bNot;

	if (Q_stristr(str, "$LINUX"))
		return false ^ bNot;

	if (Q_stristr(str, "$POSIX"))
		return false ^ bNot;

	return false;
}

bool KeyValues::LoadFromBuffer(char const* resourceName, CUtlBuffer& buf, IBaseFileSystem* pFileSystem, const char* pPathID)
{
	KeyValues* pPreviousKey = NULL;
	KeyValues* pCurrentKey = this;

	CUtlVector< KeyValues* > includedKeys;
	CUtlVector< KeyValues* > baseKeys;

	bool wasQuoted;
	bool wasConditional;

	g_KeyValuesErrorStack.SetFilename(resourceName);
	do
	{
		bool bAccepted = true;
		const char* s = ReadToken(buf, wasQuoted, wasConditional);

		if (!buf.IsValid() || !s || *s == 0)
			break;

		if (!Q_stricmp(s, "#include"))
		{
			s = ReadToken(buf, wasQuoted, wasConditional);

			if (!s || *s == 0)
			{
				g_KeyValuesErrorStack.ReportError("#include is NULL ");
			}
			else
			{
				ParseIncludedKeys(resourceName, s, pFileSystem, pPathID, includedKeys);
			}

			continue;
		}
		else if (!Q_stricmp(s, "#base"))
		{
			s = ReadToken(buf, wasQuoted, wasConditional);

			if (!s || *s == 0)
			{
				g_KeyValuesErrorStack.ReportError("#base is NULL ");
			}
			else
			{
				ParseIncludedKeys(resourceName, s, pFileSystem, pPathID, baseKeys);
			}

			continue;
		}

		if (!pCurrentKey)
		{
			pCurrentKey = new KeyValues(s);
			Assert(pCurrentKey);

			pCurrentKey->UsesEscapeSequences(m_bHasEscapeSequences != 0);
			pCurrentKey->UsesConditionals(m_bEvaluateConditionals != 0);

			if (pPreviousKey)
			{
				pPreviousKey->SetNextKey(pCurrentKey);
			}
		}
		else
		{
			pCurrentKey->SetName(s);
		}

		s = ReadToken(buf, wasQuoted, wasConditional);

		if (wasConditional)
		{
			bAccepted = !m_bEvaluateConditionals || EvaluateConditional(s);
			s = ReadToken(buf, wasQuoted, wasConditional);
		}

		if (s && *s == '{' && !wasQuoted)
		{
			pCurrentKey->RecursiveLoadFromBuffer(resourceName, buf);
		}
		else
		{
			g_KeyValuesErrorStack.ReportError("LoadFromBuffer: missing {");
		}

		if (!bAccepted)
		{
			if (pPreviousKey)
			{
				pPreviousKey->SetNextKey(NULL);
			}
			pCurrentKey->Clear();
		}
		else
		{
			pPreviousKey = pCurrentKey;
			pCurrentKey = NULL;
		}
	} while (buf.IsValid());

	AppendIncludedKeys(includedKeys);
	{
		int i;

		for (i = includedKeys.Count() - 1; i > 0; i--)
		{
			KeyValues* kv = includedKeys[i];
			kv->deleteThis();
		}
	}

	MergeBaseKeys(baseKeys);
	{
		int i;

		for (i = baseKeys.Count() - 1; i >= 0; i--)
		{
			KeyValues* kv = baseKeys[i];
			kv->deleteThis();
		}
	}

	g_KeyValuesErrorStack.SetFilename("");
	return true;
}

bool KeyValues::LoadFromBuffer(char const* resourceName, const char* pBuffer, IBaseFileSystem* pFileSystem, const char* pPathID)
{
	if (!pBuffer)
		return true;

	int nLen = Q_strlen(pBuffer);
	CUtlBuffer buf(pBuffer, nLen, CUtlBuffer::READ_ONLY | CUtlBuffer::TEXT_BUFFER);

	if (nLen > 2 && (uint8_t)pBuffer[0] == 0xFF && (uint8_t)pBuffer[1] == 0xFE)
	{
		int nUTF8Len = V_UnicodeToUTF8((wchar_t*)(pBuffer + 2), NULL, 0);
		char* pUTF8Buf = new char[nUTF8Len];
		V_UnicodeToUTF8((wchar_t*)(pBuffer + 2), pUTF8Buf, nUTF8Len);
		buf.AssumeMemory(pUTF8Buf, nUTF8Len, nUTF8Len, CUtlBuffer::READ_ONLY | CUtlBuffer::TEXT_BUFFER);
	}

	bool retVal = LoadFromBuffer(resourceName, buf, pFileSystem, pPathID);
	return retVal;
}

void KeyValues::RecursiveLoadFromBuffer(char const* resourceName, CUtlBuffer& buf)
{
	CKeyErrorContext errorReport(this);
	bool wasQuoted;
	bool wasConditional;

	if (errorReport.GetStackLevel() > 100)
	{
		g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  recursion overflow");
		return;
	}

	CKeyErrorContext errorKey(INVALID_KEY_SYMBOL);
	KeyValues* pLastChild = FindLastSubKey();;

	while (1)
	{
		bool bAccepted = true;
		const char* name = ReadToken(buf, wasQuoted, wasConditional);

		if (!name)
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got EOF instead of keyname");
			break;
		}

		if (!*name)
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got empty keyname");
			break;
		}

		if (*name == '}' && !wasQuoted)
			break;

		KeyValues* dat = CreateKeyUsingKnownLastChild(name, pLastChild);
		errorKey.Reset(dat->GetNameSymbol());
		const char* value = ReadToken(buf, wasQuoted, wasConditional);

		if (wasConditional && value)
		{
			bAccepted = !m_bEvaluateConditionals || EvaluateConditional(value);
			value = ReadToken(buf, wasQuoted, wasConditional);
		}

		if (!value)
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got NULL key");
			break;
		}

		if (*value == '}' && !wasQuoted)
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got } in key");
			break;
		}

		if (*value == '{' && !wasQuoted)
		{
			errorKey.Reset(INVALID_KEY_SYMBOL);
			dat->RecursiveLoadFromBuffer(resourceName, buf);
		}
		else
		{
			if (wasConditional)
			{
				g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got conditional between key and value");
				break;
			}

			if (dat->m_sValue)
			{
				delete[] dat->m_sValue;
				dat->m_sValue = NULL;
			}

			int len = Q_strlen(value);

			char* pIEnd;
			char* pFEnd;
			const char* pSEnd = value + len;

			int ival = strtol(value, &pIEnd, 10);
			float fval = (float)strtod(value, &pFEnd);
			bool bOverflow = (ival == LONG_MAX || ival == LONG_MIN) && errno == ERANGE;
#ifdef POSIX
			if (len > 1 && tolower(value[1]) == 'x')
			{
				fval = 0.0f;
				pFEnd = (char*)value;
			}
#endif

			if (*value == 0)
			{
				dat->m_iDataType = TYPE_STRING;
			}
			else if ((18 == len) && (value[0] == '0') && (value[1] == 'x'))
			{
				int64_t retVal = 0;

				for (int i = 2; i < 2 + 16; i++)
				{
					char digit = value[i];
					if (digit >= 'a')
						digit -= 'a' - ('9' + 1);
					else
						if (digit >= 'A')
							digit -= 'A' - ('9' + 1);
					retVal = (retVal * 16) + (digit - '0');
				}

				dat->m_sValue = new char[sizeof(uint64_t)];
				*((uint64_t*)dat->m_sValue) = retVal;
				dat->m_iDataType = TYPE_UINT64;
			}
			else if ((pFEnd > pIEnd) && (pFEnd == pSEnd))
			{
				dat->m_flValue = fval;
				dat->m_iDataType = TYPE_FLOAT;
			}
			else if (pIEnd == pSEnd && !bOverflow)
			{
				dat->m_iValue = ival;
				dat->m_iDataType = TYPE_INT;
			}
			else
			{
				dat->m_iDataType = TYPE_STRING;
			}

			if (dat->m_iDataType == TYPE_STRING)
			{
				dat->m_sValue = new char[len + 1];
				Q_memcpy(dat->m_sValue, value, len + 1);
			}

			int prevPos = buf.TellGet();
			const char* peek = ReadToken(buf, wasQuoted, wasConditional);

			if (wasConditional)
			{
				bAccepted = !m_bEvaluateConditionals || EvaluateConditional(peek);
			}
			else
			{
				buf.SeekGet(CUtlBuffer::SEEK_HEAD, prevPos);
			}
		}

		Assert(dat->m_pPeer == NULL);

		if (bAccepted)
		{
			Assert(pLastChild == NULL || pLastChild->m_pPeer == dat);
			pLastChild = dat;
		}
		else
		{
			if (pLastChild == NULL)
			{
				Assert(m_pSub == dat);
				m_pSub = NULL;
			}
			else
			{
				Assert(pLastChild->m_pPeer == dat);
				pLastChild->m_pPeer = NULL;
			}

			dat->deleteThis();
			dat = NULL;
		}
	}
}

bool KeyValues::WriteAsBinary(CUtlBuffer& buffer)
{
	if (buffer.IsText())
		return false;

	if (!buffer.IsValid())
		return false;

	for (KeyValues* dat = this; dat != NULL; dat = dat->m_pPeer)
	{
		buffer.PutUnsignedChar(dat->m_iDataType);
		buffer.PutString(dat->GetName());

		switch (dat->m_iDataType)
		{
		case TYPE_NONE:
		{
			dat->m_pSub->WriteAsBinary(buffer);
			break;
		}
		case TYPE_STRING:
		{
			if (dat->m_sValue && *(dat->m_sValue))
			{
				buffer.PutString(dat->m_sValue);
			}
			else
			{
				buffer.PutString("");
			}
			break;
		}
		case TYPE_WSTRING:
		{
			Assert(!"TYPE_WSTRING");
			break;
		}

		case TYPE_INT:
		{
			buffer.PutInt(dat->m_iValue);
			break;
		}

		case TYPE_UINT64:
		{
			buffer.PutDouble(*((double*)dat->m_sValue));
			break;
		}

		case TYPE_FLOAT:
		{
			buffer.PutFloat(dat->m_flValue);
			break;
		}
		case TYPE_COLOR:
		{
			buffer.PutUnsignedChar(dat->m_Color[0]);
			buffer.PutUnsignedChar(dat->m_Color[1]);
			buffer.PutUnsignedChar(dat->m_Color[2]);
			buffer.PutUnsignedChar(dat->m_Color[3]);
			break;
		}
		case TYPE_PTR:
		{
			buffer.PutUnsignedInt((int)dat->m_pValue);
		}

		default:
			break;
		}
	}

	buffer.PutUnsignedChar(TYPE_NUMTYPES);
	return buffer.IsValid();
}

bool KeyValues::ReadAsBinary(CUtlBuffer& buffer, int nStackDepth)
{
	if (buffer.IsText())
		return false;

	if (!buffer.IsValid())
		return false;

	RemoveEverything();
	Init();

	if (nStackDepth > 100)
	{
		AssertMsgOnce(false, "KeyValues::ReadAsBinary() stack depth > 100\n");
		return false;
	}

	KeyValues* dat = this;
	types_t type = (types_t)buffer.GetUnsignedChar();

	while (true)
	{
		if (type == TYPE_NUMTYPES)
			break;

		dat->m_iDataType = type;

		{
			char token[KEYVALUES_TOKEN_SIZE];
			buffer.GetString(token);
			token[KEYVALUES_TOKEN_SIZE - 1] = 0;
			dat->SetName(token);
		}

		switch (type)
		{
		case TYPE_NONE:
		{
			dat->m_pSub = new KeyValues("");
			dat->m_pSub->ReadAsBinary(buffer, nStackDepth + 1);
			break;
		}
		case TYPE_STRING:
		{
			char token[KEYVALUES_TOKEN_SIZE];
			buffer.GetString(token);
			token[KEYVALUES_TOKEN_SIZE - 1] = 0;

			int len = Q_strlen(token);
			dat->m_sValue = new char[len + 1];
			Q_memcpy(dat->m_sValue, token, len + 1);

			break;
		}
		case TYPE_WSTRING:
		{
			Assert(!"TYPE_WSTRING");
			break;
		}
		case TYPE_INT:
		{
			dat->m_iValue = buffer.GetInt();
			break;
		}
		case TYPE_UINT64:
		{
			dat->m_sValue = new char[sizeof(uint64_t)];
			*((uint64_t*)dat->m_sValue) = buffer.GetInt64();
			break;
		}
		case TYPE_FLOAT:
		{
			dat->m_flValue = buffer.GetFloat();
			break;
		}
		case TYPE_COLOR:
		{
			dat->m_Color[0] = buffer.GetUnsignedChar();
			dat->m_Color[1] = buffer.GetUnsignedChar();
			dat->m_Color[2] = buffer.GetUnsignedChar();
			dat->m_Color[3] = buffer.GetUnsignedChar();
			break;
		}
		case TYPE_PTR:
		{
			dat->m_pValue = (void*)buffer.GetUnsignedInt();
		}
		default:
			break;
		}

		if (!buffer.IsValid())
			return false;

		type = (types_t)buffer.GetUnsignedChar();

		if (type == TYPE_NUMTYPES)
			break;

		dat->m_pPeer = new KeyValues("");
		dat = dat->m_pPeer;
	}

	return buffer.IsValid();
}

void* KeyValues::operator new(size_t iAllocSize)
{
	return g_pInterface->KeyValueSystem->AllocKeyValuesMemory((int)iAllocSize);
}

void* KeyValues::operator new(size_t iAllocSize, int nBlockUse, const char* pFileName, int nLine)
{
	void* p = g_pInterface->KeyValueSystem->AllocKeyValuesMemory((int)iAllocSize);
	return p;
}

void KeyValues::operator delete(void* pMem)
{
	g_pInterface->KeyValueSystem->FreeKeyValuesMemory(pMem);
}

void KeyValues::operator delete(void* pMem, int nBlockUse, const char* pFileName, int nLine)
{
	g_pInterface->KeyValueSystem->FreeKeyValuesMemory(pMem);
}

void KeyValues::UnpackIntoStructure(KeyValuesUnpackStructure const* pUnpackTable, void* pDest, size_t DestSizeInBytes)
{
#ifdef DBGFLAG_ASSERT
	void* pDestEnd = (char*)pDest + DestSizeInBytes + 1;
#endif

	uint8_t* dest = (uint8_t*)pDest;

	while (pUnpackTable->m_pKeyName)
	{
		uint8_t* dest_field = dest + pUnpackTable->m_nFieldOffset;
		KeyValues* find_it = FindKey(pUnpackTable->m_pKeyName);

		switch (pUnpackTable->m_eDataType)
		{
		case UNPACK_TYPE_FLOAT:
		{
			Assert(dest_field + sizeof(float) < pDestEnd);

			float default_value = (pUnpackTable->m_pKeyDefault) ? static_cast<float>(atof(pUnpackTable->m_pKeyDefault)) : 0.0f;
			*((float*)dest_field) = GetFloat(pUnpackTable->m_pKeyName, default_value);

			break;
		}
		break;
		case UNPACK_TYPE_VECTOR:
		{
			Assert(dest_field + sizeof(Vector) < pDestEnd);

			Vector* dest_v = (Vector*)dest_field;
			char const* src_string = GetString(pUnpackTable->m_pKeyName, pUnpackTable->m_pKeyDefault);

			if ((!src_string) || (sscanf(src_string, "%f %f %f", &(dest_v->x), &(dest_v->y), &(dest_v->z)) != 3))
				dest_v->Init(0, 0, 0);
		}
		break;
		case UNPACK_TYPE_FOUR_FLOATS:
		{
			Assert(dest_field + sizeof(float) * 4 < pDestEnd);

			float* dest_f = (float*)dest_field;
			char const* src_string = GetString(pUnpackTable->m_pKeyName, pUnpackTable->m_pKeyDefault);

			if ((!src_string) || (sscanf(src_string, "%f %f %f %f", dest_f, dest_f + 1, dest_f + 2, dest_f + 3)) != 4)
				memset(dest_f, 0, 4 * sizeof(float));
		}
		break;
		case UNPACK_TYPE_TWO_FLOATS:
		{
			Assert(dest_field + sizeof(float) * 2 < pDestEnd);

			float* dest_f = (float*)dest_field;
			char const* src_string = GetString(pUnpackTable->m_pKeyName, pUnpackTable->m_pKeyDefault);

			if ((!src_string) || (sscanf(src_string, "%f %f", dest_f, dest_f + 1)) != 2)
				memset(dest_f, 0, 2 * sizeof(float));
		}
		break;
		case UNPACK_TYPE_STRING:
		{
			Assert(dest_field + pUnpackTable->m_nFieldSize < pDestEnd);
			char* dest_s = (char*)dest_field;
			strncpy(dest_s, GetString(pUnpackTable->m_pKeyName, pUnpackTable->m_pKeyDefault), pUnpackTable->m_nFieldSize);

		}
		break;
		case UNPACK_TYPE_INT:
		{
			Assert(dest_field + sizeof(int) < pDestEnd);

			int* dest_i = (int*)dest_field;
			int default_int = 0;

			if (pUnpackTable->m_pKeyDefault)
				default_int = atoi(pUnpackTable->m_pKeyDefault);

			*(dest_i) = GetInt(pUnpackTable->m_pKeyName, default_int);
		}
		break;
		case UNPACK_TYPE_VECTOR_COLOR:
		{
			Assert(dest_field + sizeof(Vector) < pDestEnd);
			Vector* dest_v = (Vector*)dest_field;

			if (find_it)
			{
				Color c = GetColor(pUnpackTable->m_pKeyName);
				dest_v->x = static_cast<float>(c.r());
				dest_v->y = static_cast<float>(c.g());
				dest_v->z = static_cast<float>(c.b());
			}
			else
			{
				if (pUnpackTable->m_pKeyDefault)
					sscanf(pUnpackTable->m_pKeyDefault, "%f %f %f", &(dest_v->x), &(dest_v->y), &(dest_v->z));
				else
					dest_v->Init(0, 0, 0);
			}

			*(dest_v) *= (1.0f / 255);
		}
		}

		pUnpackTable++;
	}
}

bool KeyValues::ProcessResolutionKeys(const char* pResString)
{
	if (!pResString)
	{
		return false;
	}

	KeyValues* pSubKey = GetFirstSubKey();

	if (!pSubKey)
	{
		return false;
	}

	for (; pSubKey != NULL; pSubKey = pSubKey->GetNextKey())
	{
		pSubKey->ProcessResolutionKeys(pResString);

		if (Q_stristr(pSubKey->GetName(), pResString) != NULL)
		{
			char normalKeyName[128];
			V_strncpy(normalKeyName, pSubKey->GetName(), sizeof(normalKeyName));
			char* pString = Q_stristr(normalKeyName, pResString);

			if (pString && !Q_stricmp(pString, pResString))
			{
				*pString = '\0';
				KeyValues* pKey = FindKey(normalKeyName);

				if (pKey)
				{
					RemoveSubKey(pKey);
				}

				pSubKey->SetName(normalKeyName);
			}
		}
	}

	return true;
}

bool KeyValues::Dump(IKeyValuesDumpContext* pDump, int nIndentLevel /* = 0 */)
{
	if (!pDump->KvBeginKey(this, nIndentLevel))
		return false;

	for (KeyValues* val = this ? GetFirstValue() : NULL; val; val = val->GetNextValue())
	{
		if (!pDump->KvWriteValue(val, nIndentLevel + 1))
			return false;
	}

	for (KeyValues* sub = this ? GetFirstTrueSubKey() : NULL; sub; sub = sub->GetNextTrueSubKey())
	{
		if (!sub->Dump(pDump, nIndentLevel + 1))
			return false;
	}

	return pDump->KvEndKey(this, nIndentLevel);
}

bool IKeyValuesDumpContextAsText::KvBeginKey(KeyValues* pKey, int nIndentLevel)
{
	if (pKey)
	{
		return KvWriteIndent(nIndentLevel) && KvWriteText(pKey->GetName()) && KvWriteText(" {\n");
	}
	else
	{
		return KvWriteIndent(nIndentLevel) && KvWriteText("<< NULL >>\n");
	}
}

#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) )

#define stackalloc( _size ) _alloca( ALIGN_VALUE( _size, 16 ) )
#define mallocsize( _p ) ( _msize( _p ) )

bool IKeyValuesDumpContextAsText::KvWriteValue(KeyValues* val, int nIndentLevel)
{
	if (!val)
	{
		return KvWriteIndent(nIndentLevel) && KvWriteText("<< NULL >>\n");
	}

	if (!KvWriteIndent(nIndentLevel))
		return false;

	if (!KvWriteText(val->GetName()))
		return false;

	if (!KvWriteText(" "))
		return false;

	switch (val->GetDataType())
	{
	case KeyValues::TYPE_STRING:
	{
		if (!KvWriteText(val->GetString()))
			return false;
	}
	break;
	case KeyValues::TYPE_INT:
	{
		int n = val->GetInt();
		char* chBuffer = (char*)stackalloc(128);
		V_snprintf(chBuffer, 128, "int( %d = 0x%X )", n, n);

		if (!KvWriteText(chBuffer))
			return false;
	}
	break;
	case KeyValues::TYPE_FLOAT:
	{
		float fl = val->GetFloat();
		char* chBuffer = (char*)stackalloc(128);
		V_snprintf(chBuffer, 128, "float( %f )", fl);

		if (!KvWriteText(chBuffer))
			return false;
	}
	break;
	case KeyValues::TYPE_PTR:
	{
		void* ptr = val->GetPtr();
		char* chBuffer = (char*)stackalloc(128);
		V_snprintf(chBuffer, 128, "ptr( 0x%p )", ptr);

		if (!KvWriteText(chBuffer))
			return false;
	}
	break;
	case KeyValues::TYPE_WSTRING:
	{
		wchar_t const* wsz = val->GetWString();

		int nLen = V_wcslen(wsz);
		int numBytes = nLen * 2 + 64;

		char* chBuffer = (char*)stackalloc(numBytes);
		V_snprintf(chBuffer, numBytes, "%ls [wstring, len = %d]", wsz, nLen);

		if (!KvWriteText(chBuffer))
			return false;
	}
	break;
	case KeyValues::TYPE_UINT64:
	{
		uint64_t n = val->GetUint64();
		char* chBuffer = (char*)stackalloc(128);
		V_snprintf(chBuffer, 128, "u64( %lld = 0x%llX )", n, n);

		if (!KvWriteText(chBuffer))
			return false;
	}
	break;
	default:
		break;
		{
			int n = val->GetDataType();
			char* chBuffer = (char*)stackalloc(128);
			V_snprintf(chBuffer, 128, "??kvtype[%d]", n);

			if (!KvWriteText(chBuffer))
				return false;
		}
		break;
	}

	return KvWriteText("\n");
}

bool IKeyValuesDumpContextAsText::KvEndKey(KeyValues* pKey, int nIndentLevel)
{
	if (pKey)
	{
		return KvWriteIndent(nIndentLevel) && KvWriteText("}\n");
	}
	else
	{
		return true;
	}
}

bool IKeyValuesDumpContextAsText::KvWriteIndent(int nIndentLevel)
{
	int numIndentBytes = (nIndentLevel * 2 + 1);
	char* pchIndent = (char*)stackalloc(numIndentBytes);

	memset(pchIndent, ' ', numIndentBytes - 1);
	pchIndent[numIndentBytes - 1] = 0;

	return KvWriteText(pchIndent);
}

bool CKeyValuesDumpContextAsDevMsg::KvBeginKey(KeyValues* pKey, int nIndentLevel)
{
	static ConVar* r_developer = g_pInterface->Cvar->FindVar(XorStr("developer"));

	if (r_developer != nullptr && r_developer->GetInt() < m_nDeveloperLevel)
		return false;
	else
		return IKeyValuesDumpContextAsText::KvBeginKey(pKey, nIndentLevel);
}

bool CKeyValuesDumpContextAsDevMsg::KvWriteText(char const* szText)
{
	if (m_nDeveloperLevel > 0)
	{
		std::stringstream ss;
		ss << m_nDeveloperLevel, XorStr("%s", szText);
		Utils::log(ss.str().c_str());
	}
	else
	{
		std::stringstream ss;
		ss << XorStr("%s", szText);
		Utils::log(ss.str().c_str());
	}

	return true;
}