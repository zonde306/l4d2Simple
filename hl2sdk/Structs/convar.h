#pragma once

#include "../Interfaces/IAppSystem.h"
#include "../Structs/color.h"
#include "../Utils/utlvector.h"
#include "../Utils/utlstring.h"

#include <Windows.h>

class IConVar;
class CCommand;
class ConVar;

enum ConvarFlags
{
	FCVAR_NONE = 0,
	FCVAR_UNREGISTERED = (1 << 0), // If this is set, don't add to linked list, etc.
	FCVAR_DEVELOPMENTONLY = (1 << 1), // Hidden in released products. Flag is removed automatically if ALLOW_DEVELOPMENT_CVARS is defined.
	FCVAR_GAMEDLL = (1 << 2), // defined by the game DLL
	FCVAR_CLIENTDLL = (1 << 3), // defined by the client DLL
	FCVAR_HIDDEN = (1 << 4), // Hidden. Doesn't appear in find or autocomplete. Like DEVELOPMENTONLY, but can't be compiled out.
	FCVAR_PROTECTED = (1 << 5), // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
	FCVAR_SPONLY = (1 << 6),  // This cvar cannot be changed by clients connected to a multiplayer server.
	FCVAR_ARCHIVE = (1 << 7), // set to cause it to be saved to vars.rc
	FCVAR_NOTIFY = (1 << 8), // notifies players when changed
	FCVAR_USERINFO = (1 << 9), // changes the client's info string
	FCVAR_CHEAT = (1 << 14), // Only useable in singleplayer / debug / multiplayer & sv_cheats
	FCVAR_PRINTABLEONLY = (1 << 10), // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
	FCVAR_UNLOGGED = (1 << 11), // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
	FCVAR_NEVER_AS_STRING = (1 << 12), // never try to print that cvar
	FCVAR_REPLICATED = (1 << 13), // server setting enforced on clients, TODO rename to FCAR_SERVER at some time
	FCVAR_DEMO = (1 << 16), // record this cvar when starting a demo file
	FCVAR_DONTRECORD = (1 << 17), // don't record these command in demofiles
	FCVAR_RELOAD_MATERIALS = (1 << 20), // If this cvar changes, it forces a material reload
	FCVAR_RELOAD_TEXTURES = (1 << 21), // If this cvar changes, if forces a texture reload
	FCVAR_NOT_CONNECTED = (1 << 22), // cvar cannot be changed by a client that is connected to a server
	FCVAR_MATERIAL_SYSTEM_THREAD = (1 << 23), // Indicates this cvar is read from the material system thread
	FCVAR_ARCHIVE_XBOX = (1 << 24), // cvar written to config.cfg on the Xbox
	FCVAR_ACCESSIBLE_FROM_THREADS = (1 << 25), // used as a debugging tool necessary to check material system thread convars
	FCVAR_SERVER_CAN_EXECUTE = (1 << 28), // the server is allowed to execute this command on clients via ClientCommand/NET_StringCmd/CBaseClientState::ProcessStringCmd.
	FCVAR_SERVER_CANNOT_QUERY = (1 << 29), // If this is set, then the server is not allowed to query this cvar's value (via IServerPluginHelpers::StartQueryCvarValue).
	FCVAR_CLIENTCMD_CAN_EXECUTE = (1 << 30), // IVEngineClient::ClientCmd is allowed to execute this command.
	FCVAR_MATERIAL_THREAD_MASK = (FCVAR_RELOAD_MATERIALS | FCVAR_RELOAD_TEXTURES | FCVAR_MATERIAL_SYSTEM_THREAD)
};

typedef void(*FnChangeCallback_t)(IConVar* var, const char* pOldValue, float flOldValue);
typedef void(*FnCommandCallbackVoid_t)(void);
typedef void(*FnCommandCallback_t)(const CCommand& command);

#define COMMAND_COMPLETION_MAXITEMS 64
#define COMMAND_COMPLETION_ITEM_LENGTH 64

typedef int(*FnCommandCompletionCallback)(const char* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

class IConVar
{
public:
	virtual void SetValue(const char* pValue) = 0;
	virtual void SetValue(float flValue) = 0;
	virtual void SetValue(int nValue) = 0;
	virtual const char* GetName(void)const = 0;
	virtual bool IsFlagSet(int nFlag) const = 0;
};

class ConCommandBase;
class IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(ConCommandBase* pVar) = 0;
};

class CDefaultAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(ConCommandBase* pVar) override;
};

void ConVar_Register(int nCVarFlag, IConCommandBaseAccessor* pAccessor = NULL);

class ICommandCallback
{
public:
	virtual void CommandCallback(const CCommand& command) = 0;
};

class ICommandCompletionCallback
{
public:
	virtual int  CommandCompletionCallback(const char* pPartial, CUtlVector< CUtlString >& commands) = 0;
};

class ConCommandBase
{
public:
	ConCommandBase(void);
	ConCommandBase(const char* pName, const char* pHelpString = 0, int flags = 0);

	virtual ~ConCommandBase(void);

	virtual	bool IsCommand(void) const;
	virtual bool IsFlagSet(int flag) const;
	virtual void AddFlags(int flags);
	virtual void RemoveFlags(int flags);
	virtual int GetFlags() const;
	virtual const char* GetName(void) const;
	virtual const char* GetHelpText(void) const;
	virtual bool IsRegistered(void) const;
	virtual int GetDLLIdentifier() const;
	virtual void Create(const char* pName, const char* pHelpString = 0, int flags = 0);
	virtual void Init();

public:
	ConCommandBase* m_pNext;
	bool m_bRegistered;
	const char* m_pszName;
	const char* m_pszHelpString;
	int m_nFlags;

public:
	static ConCommandBase* s_pConCommandBases;
	static IConCommandBaseAccessor* s_pAccessor;
};

class ConCommand : public ConCommandBase
{
	friend class CCvar;

public:
	typedef ConCommandBase BaseClass;

	ConCommand(const char* pName, FnCommandCallbackVoid_t callback,
		const char* pHelpString = 0, int flags = 0, FnCommandCompletionCallback completionFunc = 0);

	ConCommand(const char* pName, FnCommandCallback_t callback,
		const char* pHelpString = 0, int flags = 0, FnCommandCompletionCallback completionFunc = 0);

	ConCommand(const char* pName, ICommandCallback* pCallback,
		const char* pHelpString = 0, int flags = 0, ICommandCompletionCallback* pCommandCompletionCallback = 0);

	virtual ~ConCommand(void);
	virtual	bool IsCommand(void) const;
	virtual int AutoCompleteSuggest(const char* partial, CUtlVector< CUtlString >& commands);
	virtual bool CanAutoComplete(void);
	virtual void Dispatch(const CCommand& command);

private:
	union
	{
		FnCommandCallbackVoid_t m_fnCommandCallbackV1;
		FnCommandCallback_t m_fnCommandCallback;
		ICommandCallback* m_pCommandCallback;
	};

	union
	{
		FnCommandCompletionCallback	m_fnCompletionCallback;
		ICommandCompletionCallback* m_pCommandCompletionCallback;
	};

	bool m_bHasCompletionCallback : 1;
	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;
};

class ConVar : public ConCommandBase, public IConVar
{
public:
	typedef ConCommandBase BaseClass;

	ConVar(const char* pName, const char* pDefaultValue, int flags = 0);
	ConVar(const char* pName, const char* pDefaultValue, int flags, const char* pHelpString);
	ConVar(const char* pName, const char* pDefaultValue, int flags, const char* pHelpString, bool bMin, float fMin, bool bMax, float fMax);
	ConVar(const char* pName, const char* pDefaultValue, int flags, const char* pHelpString, FnChangeCallback_t callback);
	ConVar(const char* pName, const char* pDefaultValue, int flags, const char* pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback);

	virtual ~ConVar(void);

	virtual	bool IsCommand(void) const;
	virtual bool IsFlagSet(int flag) const;
	virtual void AddFlags(int flags);
	virtual void RemoveFlags(int flags) { m_nFlags &= ~flags; };
	virtual int GetFlags() const;
	virtual const char* GetName(void) const;
	virtual const char* GetHelpText(void) const;
	virtual bool IsRegistered(void) const;

	virtual int GetDLLIdentifier() const
	{
		return ConCommandBase::GetDLLIdentifier();
	};

	virtual void Create(const char* pName, const char* pDefaultValue, int flags = 0)
	{
		Create(pName, pDefaultValue, flags, "");
	}

	virtual void Init();

	void SetValue(const char* value);
	void SetValue(float value);
	void SetValue(int value);
	void SetValue(Color value);

	virtual void InternalSetValue(const char* value);
	virtual void InternalSetFloatValue(float fNewValue);
	virtual void InternalSetIntValue(int nValue);
	virtual void InternalSetColorValue(Color value);
	virtual bool ClampValue(float& value);
	virtual void ChangeStringValue(const char* tempVal, float flOldValue = 0.0f);

	virtual void Create(const char* pName, const char* pDefaultValue, int flags = 0,
		const char* pHelpString = 0, bool bMin = false, float fMin = 0.0,
		bool bMax = false, float fMax = false, FnChangeCallback_t callback = 0);

	float GetFloat(void) const;
	int GetInt(void) const;
	DWORD GetColor(void) const;
	const char* GetString(void) const;
	const char* GetDefault(void) const;

	ConVar* m_pParent;
	const char* m_pszDefaultValue;
	char* m_pszString;
	int m_StringLength;
	float m_fValue;
	int m_nValue;
	bool m_bHasMin;
	float m_fMinVal;
	bool m_bHasMax;
	float m_fMaxVal;
	FnChangeCallback_t m_fnChangeCallback;
};

class SpoofedConvar
{
public:
	SpoofedConvar();
	SpoofedConvar(const char* szCVar);
	SpoofedConvar(ConVar* pCVar);

	~SpoofedConvar();

	bool IsSpoofed();

	void Spoof();
	void Unspoof();

	void SetFlags(int flags);
	int GetFlags();

	void SetBool(bool bValue);
	void SetInt(int iValue);
	void SetFloat(float flValue);
	void SetString(const char* szValue);

	ConVar* GetOriginal() const;
	ConVar* GetDummy() const;

private:
	ConVar* m_pOriginalCVar = nullptr;
	ConVar* m_pDummyCVar = nullptr;

	char m_szDummyName[128];
	char m_szDummyValue[128];
	char m_szOriginalName[128];
	char m_szOriginalValue[128];
	int m_iOriginalFlags;
};