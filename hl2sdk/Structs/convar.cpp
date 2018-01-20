#include "convar.h"
#include "../definitions.h"
#include "../Interfaces/ICvar.h"
#include "../../l4d2Simple2/utils.h"

namespace interfaces
{
	extern ICvar* Cvar;
}

ConCommandBase*		ConCommandBase::s_pConCommandBases = nullptr;
static int			s_nCVarFlag = 0;
static int			s_nDLLIdentifier = -1;
static bool			s_bRegistered = false;
static CDefaultAccessor s_DefaultAccessor;
IConCommandBaseAccessor	*ConCommandBase::s_pAccessor = &s_DefaultAccessor;

bool CDefaultAccessor::RegisterConCommandBase(ConCommandBase * pVar)
{
#ifdef _DEBUG
	Utils::log("Register ConVar: %s", pVar->GetName());
#endif
	interfaces::Cvar->RegisterConCommand(pVar);
	return true;
}

void ConVar_Register(int nCVarFlag, IConCommandBaseAccessor *pAccessor)
{
	if (!interfaces::Cvar || s_bRegistered)
		return;

	s_bRegistered = true;
	s_nCVarFlag = nCVarFlag;
	s_nDLLIdentifier = interfaces::Cvar->AllocateDLLIdentifier();

	ConCommandBase *pCur = ConCommandBase::s_pConCommandBases, *pNext = nullptr;
	ConCommandBase::s_pAccessor = pAccessor ? pAccessor : &s_DefaultAccessor;

	while (pCur)
	{
		pNext = pCur->m_pNext;
		pCur->AddFlags(s_nCVarFlag);
		pCur->Init();
		pCur = pNext;
	}

	ConCommandBase::s_pConCommandBases = nullptr;
}


//-------------------------------------------------------------
// ConVar methods
//-------------------------------------------------------------
ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags)
{
	Create(pName, pDefaultValue, flags, "");
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString)
{
	Create(pName, pDefaultValue, flags, pHelpString);
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax)
{
	Create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax);
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, FnChangeCallback_t callback)
{
	Create(pName, pDefaultValue, flags, pHelpString, false, 0.0, false, 0.0, callback);
}

ConVar::ConVar(const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback)
{
	Create(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, callback);
}

ConVar::~ConVar(void)
{
	if (m_pszString)
	{
		delete[] m_pszString;
		m_pszString = NULL;
	}
}
bool ConVar::IsFlagSet(int flag) const
{
	return (flag & m_pParent->m_nFlags) ? true : false;
}

const char *ConVar::GetHelpText(void) const
{
	return m_pParent->m_pszHelpString;
}

void ConVar::AddFlags(int flags)
{
	m_pParent->m_nFlags |= flags;
}

int ConVar::GetFlags(void) const
{
	return m_pParent->m_nFlags;
}

bool ConVar::IsRegistered(void) const
{
	return m_pParent->m_bRegistered;
}
bool ConVar::IsCommand(void) const
{
	return false;
}

const char *ConVar::GetName(void) const
{
	return m_pParent->m_pszName;
}
/*
const char* ConVar::GetBaseName(void) const
{
return m_pParent->m_pszName;
}
*/
float ConVar::GetFloat(void) const
{
	return m_pParent->m_fValue;
}
int ConVar::GetInt(void) const
{
	return m_pParent->m_nValue;
}
const char* ConVar::GetString(void) const
{
	return m_pParent->m_pszString;
}
/*
int ConVar::GetSplitScreenPlayerSlot(void) const
{
return 0;
}
*/
DWORD ConVar::GetColor(void) const
{
	unsigned char *pColorElement = ((unsigned char *)&m_pParent->m_nValue);
	return COLORCODE(pColorElement[0], pColorElement[1], pColorElement[2], pColorElement[3]);
}
void ConVar::InternalSetValue(const char *value)
{
	float fNewValue;
	char  tempVal[32];
	char  *val;

	float flOldValue = m_fValue;

	val = (char *)value;
	fNewValue = (float)atof(value);

	if (ClampValue(fNewValue))
	{
		val = tempVal;
	}

	// Redetermine value
	m_fValue = fNewValue;
	m_nValue = (int)(fNewValue);

	if (!(m_nFlags & (int)ConvarFlags::FCVAR_NEVER_AS_STRING))
	{
		ChangeStringValue(val, flOldValue);
	}
}
void ConVar::ChangeStringValue(const char *tempVal, float flOldValue)
{
	UNREFERENCED_PARAMETER(flOldValue);
	int len = strlen(tempVal) + 1;

	if (len > m_StringLength)
	{
		if (m_pszString)
		{
			delete[] m_pszString;
		}

		m_pszString = new char[len];
		m_StringLength = len;
	}

	memcpy(m_pszString, tempVal, len);

}
bool ConVar::ClampValue(float& value)
{
	if (m_bHasMin && (value < m_fMinVal))
	{
		value = m_fMinVal;
		return true;
	}

	if (m_bHasMax && (value > m_fMaxVal))
	{
		value = m_fMaxVal;
		return true;
	}

	return false;
}
void ConVar::InternalSetFloatValue(float fNewValue)
{
	if (fNewValue == m_fValue)
		return;
	// Check bounds
	ClampValue(fNewValue);

	// Redetermine value
	float flOldValue = m_fValue;
	m_fValue = fNewValue;
	m_nValue = (int)fNewValue;

	if (!(m_nFlags & (int)ConvarFlags::FCVAR_NEVER_AS_STRING))
	{
		char tempVal[32];
		ChangeStringValue(tempVal, flOldValue);
	}
}
void ConVar::InternalSetIntValue(int nValue)
{
	if (nValue == m_nValue)
		return;

	float fValue = (float)nValue;
	if (ClampValue(fValue))
	{
		nValue = (int)(fValue);
	}

	float flOldValue = m_fValue;
	m_fValue = fValue;
	m_nValue = nValue;

	if (!(m_nFlags & (int)ConvarFlags::FCVAR_NEVER_AS_STRING))
	{
		char tempVal[32];
		ChangeStringValue(tempVal, flOldValue);
	}
}

void ConVar::InternalSetColorValue(Color cValue)
{
	int color = (cValue.a() << 24 | cValue.r() << 16 | cValue.g() << 8 | cValue.b());
	InternalSetIntValue(color);
}

void ConVar::Create(const char *pName, const char *pDefaultValue,
	int flags, const char *pHelpString, bool bMin, float fMin,
	bool bMax, float fMax, FnChangeCallback_t callback)
{
	static const char *empty_string = "";

	m_pParent = this;

	// Name should be static data
	m_pszDefaultValue = pDefaultValue ? pDefaultValue : empty_string;

	m_StringLength = strlen(m_pszDefaultValue) + 1;
	m_pszString = new char[m_StringLength];
	memcpy(m_pszString, m_pszDefaultValue, m_StringLength);

	m_bHasMin = bMin;
	m_fMinVal = fMin;
	m_bHasMax = bMax;
	m_fMaxVal = fMax;

	m_fnChangeCallback = callback;
	m_fValue = (float)atof(m_pszString);
	m_nValue = (int)m_fValue;

	BaseClass::Create(pName, pHelpString, flags);
}
void ConVar::SetValue(const char *value)
{
	m_pParent->InternalSetValue(value);
}
void ConVar::SetValue(float value)
{
	m_pParent->InternalSetFloatValue(value);
}
void ConVar::SetValue(int value)
{
	m_pParent->InternalSetIntValue(value);
}

void ConVar::SetValue(Color value)
{
	m_pParent->InternalSetColorValue(value);
}

const char* ConVar::GetDefault(void) const
{
	return m_pParent->m_pszDefaultValue;
}

inline void ConVar::Init()
{
	ConCommandBase::Init();
}

//-------------------------------------------------------------
// ConCommandBase methods
//-------------------------------------------------------------
ConCommandBase::ConCommandBase(void)
{
	m_bRegistered = false;
	m_pszName = NULL;
	m_pszHelpString = NULL;

	m_nFlags = 0;
	m_pNext = NULL;
}

ConCommandBase::ConCommandBase(const char *pName, const char *pHelpString, int flags)
{
	Create(pName, pHelpString, flags);
}
ConCommandBase::~ConCommandBase(void)
{}

bool ConCommandBase::IsCommand(void) const
{
	return true;
}
int ConCommandBase::GetDLLIdentifier() const
{
	return s_nDLLIdentifier;
}
void ConCommandBase::Create(const char *pName, const char *pHelpString, int flags)
{
	static const char* empty_string = "";

	m_bRegistered = false;

	m_pszName = pName;
	m_pszHelpString = pHelpString ? pHelpString : empty_string;

	m_nFlags = flags;

	if (!(m_nFlags & (int)ConvarFlags::FCVAR_UNREGISTERED))
	{
		m_pNext = s_pConCommandBases;
		s_pConCommandBases = this;
	}
	else
	{
		m_pNext = NULL;
	}

	Init();
}
void ConCommandBase::Init()
{
	if (s_pAccessor)
	{
		s_pAccessor->RegisterConCommandBase(this);
	}
}
const char* ConCommandBase::GetName(void) const
{
	return m_pszName;
}
bool ConCommandBase::IsFlagSet(int flag) const
{
	return (flag & m_nFlags) ? true : false;
}
void ConCommandBase::AddFlags(int flags)
{
	m_nFlags |= flags;
}
void ConCommandBase::RemoveFlags(int flags)
{
	m_nFlags &= ~flags;
}
int ConCommandBase::GetFlags(void) const
{
	return m_nFlags;
}
const char* ConCommandBase::GetHelpText(void) const
{
	return m_pszHelpString;
}
bool ConCommandBase::IsRegistered(void) const
{
	return m_bRegistered;
}

//-----------------------------------------------------------------------------
// Purpose: Constructs a console command
//-----------------------------------------------------------------------------
//ConCommand::ConCommand()
//{
//	m_bIsNewConCommand = true;
//}

//-----------------------------------------------------------------------------
// Default console command autocompletion function 
//-----------------------------------------------------------------------------
int DefaultCompletionFunc(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
	return 0;
}

ConCommand::ConCommand(const char *pName, FnCommandCallbackVoid_t callback, const char *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*= 0*/)
{
	// Set the callback
	m_fnCommandCallbackV1 = callback;
	m_bUsingNewCommandCallback = false;
	m_bUsingCommandCallbackInterface = false;
	m_fnCompletionCallback = completionFunc ? completionFunc : DefaultCompletionFunc;
	m_bHasCompletionCallback = completionFunc != 0 ? true : false;

	// Setup the rest
	BaseClass::Create(pName, pHelpString, flags);
}

ConCommand::ConCommand(const char *pName, FnCommandCallback_t callback, const char *pHelpString /*= 0*/, int flags /*= 0*/, FnCommandCompletionCallback completionFunc /*= 0*/)
{
	// Set the callback
	m_fnCommandCallback = callback;
	m_bUsingNewCommandCallback = true;
	m_fnCompletionCallback = completionFunc ? completionFunc : DefaultCompletionFunc;
	m_bHasCompletionCallback = completionFunc != 0 ? true : false;
	m_bUsingCommandCallbackInterface = false;

	// Setup the rest
	BaseClass::Create(pName, pHelpString, flags);
}

ConCommand::ConCommand(const char *pName, ICommandCallback *pCallback, const char *pHelpString /*= 0*/, int flags /*= 0*/, ICommandCompletionCallback *pCompletionCallback /*= 0*/)
{
	// Set the callback
	m_pCommandCallback = pCallback;
	m_bUsingNewCommandCallback = false;
	m_pCommandCompletionCallback = pCompletionCallback;
	m_bHasCompletionCallback = (pCompletionCallback != 0);
	m_bUsingCommandCallbackInterface = true;

	// Setup the rest
	BaseClass::Create(pName, pHelpString, flags);
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
ConCommand::~ConCommand(void)
{
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if this is a command 
//-----------------------------------------------------------------------------
bool ConCommand::IsCommand(void) const
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Invoke the function if there is one
//-----------------------------------------------------------------------------
void ConCommand::Dispatch(const CCommand &command)
{
	if (m_bUsingNewCommandCallback)
	{
		if (m_fnCommandCallback)
		{
			(*m_fnCommandCallback)(command);
			return;
		}
	}
	else if (m_bUsingCommandCallbackInterface)
	{
		if (m_pCommandCallback)
		{
			m_pCommandCallback->CommandCallback(command);
			return;
		}
	}
	else
	{
		if (m_fnCommandCallbackV1)
		{
			(*m_fnCommandCallbackV1)();
			return;
		}
	}

	// Command without callback!!!
	// AssertMsg(0, "Encountered ConCommand '%s' without a callback!\n", GetName());
}


//-----------------------------------------------------------------------------
// Purpose: Calls the autocompletion method to get autocompletion suggestions
//-----------------------------------------------------------------------------
int	ConCommand::AutoCompleteSuggest(const char *partial, CUtlVector< CUtlString > &commands)
{
	if (m_bUsingCommandCallbackInterface)
	{
		if (!m_pCommandCompletionCallback)
			return 0;
		return m_pCommandCompletionCallback->CommandCompletionCallback(partial, commands);
	}

	Assert(m_fnCompletionCallback);
	if (!m_fnCompletionCallback)
		return 0;

	char rgpchCommands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH];
	int iret = (m_fnCompletionCallback)(partial, rgpchCommands);
	for (int i = 0; i < iret; ++i)
	{
		CUtlString str = rgpchCommands[i];
		commands.AddToTail(str);
	}
	return iret;
}

//-----------------------------------------------------------------------------
// Returns true if the console command can autocomplete 
//-----------------------------------------------------------------------------
bool ConCommand::CanAutoComplete(void)
{
	return m_bHasCompletionCallback;
}


SpoofedConvar::SpoofedConvar()
{}

SpoofedConvar::SpoofedConvar(const char* szCVar)
{
	m_pOriginalCVar = interfaces::Cvar->FindVar(szCVar);
	Spoof();
}
SpoofedConvar::SpoofedConvar(ConVar* pCVar)
{
	m_pOriginalCVar = pCVar;
	Spoof();
}
SpoofedConvar::~SpoofedConvar()
{
	if (IsSpoofed())
	{
		DWORD dwOld;

		SetFlags(m_iOriginalFlags);
		SetString(m_szOriginalValue);

		VirtualProtect((LPVOID)m_pOriginalCVar->m_pszName, 128, PAGE_READWRITE, &dwOld);
		strcpy((char*)m_pOriginalCVar->m_pszName, m_szOriginalName);
		VirtualProtect((LPVOID)m_pOriginalCVar->m_pszName, 128, dwOld, &dwOld);

		//Unregister dummy cvar
		interfaces::Cvar->UnregisterConCommand(m_pDummyCVar);
		free(m_pDummyCVar);
		m_pDummyCVar = nullptr;
	}
}
bool SpoofedConvar::IsSpoofed()
{
	return m_pDummyCVar != nullptr;
}
void SpoofedConvar::Spoof()
{
	if (!IsSpoofed() && m_pOriginalCVar)
	{
		//Save old name value and flags so we can restore the cvar lates if needed
		m_iOriginalFlags = m_pOriginalCVar->m_nFlags;
		strcpy(m_szOriginalName, m_pOriginalCVar->m_pszName);
		strcpy(m_szOriginalValue, m_pOriginalCVar->m_pszDefaultValue);

		sprintf_s(m_szDummyName, 128, "d_%s", m_szOriginalName);

		//Create the dummy cvar
		m_pDummyCVar = (ConVar*)malloc(sizeof(ConVar));
		if (!m_pDummyCVar) return;
		memcpy(m_pDummyCVar, m_pOriginalCVar, sizeof(ConVar));

		m_pDummyCVar->m_pNext = nullptr;
		//Register it
		interfaces::Cvar->RegisterConCommand(m_pDummyCVar);

		//Fix "write access violation" bullshit
		DWORD dwOld;
		VirtualProtect((LPVOID)m_pOriginalCVar->m_pszName, 128, PAGE_READWRITE, &dwOld);

		//Rename the cvar
		strcpy((char*)m_pOriginalCVar->m_pszName, m_szDummyName);

		VirtualProtect((LPVOID)m_pOriginalCVar->m_pszName, 128, dwOld, &dwOld);

		SetFlags(FCVAR_NONE);
	}
}
void SpoofedConvar::SetFlags(int flags)
{
	if (IsSpoofed())
	{
		m_pOriginalCVar->m_nFlags = flags;
	}
}
int SpoofedConvar::GetFlags()
{
	return m_pOriginalCVar->m_nFlags;
}
void SpoofedConvar::SetInt(int iValue)
{
	if (IsSpoofed())
	{
		m_pOriginalCVar->SetValue(iValue);
	}
}
void SpoofedConvar::SetBool(bool bValue)
{
	if (IsSpoofed())
	{
		m_pOriginalCVar->SetValue(bValue);
	}
}
void SpoofedConvar::SetFloat(float flValue)
{
	if (IsSpoofed())
	{
		m_pOriginalCVar->SetValue(flValue);
	}
}
void SpoofedConvar::SetString(const char* szValue)
{
	if (IsSpoofed())
	{
		m_pOriginalCVar->SetValue(szValue);
	}
}
