#pragma once
#include <Windows.h>

class KeyValues
{
public:
	enum MergeKeyValuesOp_t
	{
		MERGE_KV_ALL,
		MERGE_KV_UPDATE,
		MERGE_KV_DELETE,
		MERGE_KV_BORROW
	};

private:

	DWORD m_iKeyName : 24;
	DWORD m_iKeyNameCaseSensitive1 : 8;

	char* m_sValue;
	wchar_t* m_wsValue;

	union
	{
		int m_iValue;
		float m_flValue;
		PVOID m_pValue;
		BYTE m_Color[4];
	};

	char m_iDataType;
	char m_bHasEscapeSequences;
	WORD m_iKeyNameCaseSensitive2;

	KeyValues* m_pPeer;
	KeyValues* m_pSub;
	KeyValues* m_pChain;

	DWORD pad0; // cba to figure out if it's at the bottom of the class or somewhere else. it works, i don't care. 
};

