#pragma once
#include <Windows.h>

#define INTERFACEVERSION_GAMEEVENTSMANAGER	"GAMEEVENTSMANAGER002"

#define EVENT_DEBUG_ID_INIT		42
#define EVENT_DEBUG_ID_SHUTDOWN	13
#undef CreateEvent

class KeyValues;

class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};

class IGameEvent
{
public:
	virtual ~IGameEvent() {};
	virtual const char *GetName() const = 0;	// get event name

	virtual bool  IsReliable() const = 0; // if event handled reliable
	virtual bool  IsLocal() const = 0; // if event is never networked
	virtual bool  IsEmpty(const char *keyName = NULL) = 0; // check if data field exists

	// Data access
	virtual bool  GetBool(const char *keyName = NULL, bool defaultValue = false) = 0;
	virtual int   GetInt(const char *keyName = NULL, int defaultValue = 0) = 0;
	virtual float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f) = 0;
	virtual const char *GetString(const char *keyName = NULL, const char *defaultValue = "") = 0;
	virtual const wchar_t *GetWString(char const *keyName = NULL, const wchar_t *defaultValue = L"") = 0;

	virtual void SetBool(const char *keyName, bool value) = 0;
	virtual void SetInt(const char *keyName, int value) = 0;
	virtual void SetFloat(const char *keyName, float value) = 0;
	virtual void SetString(const char *keyName, const char *value) = 0;
	virtual void SetWString(const char *keyName, const wchar_t *value) = 0;
};

class IGameEventListener2
{
public:
	virtual	~IGameEventListener2(void) {};

	// FireEvent is called by EventManager if event just occured
	// KeyValue memory will be freed by manager if not needed anymore
	virtual void FireGameEvent(IGameEvent *event) = 0;

	virtual int GetEventDebugID(void) final { return m_nDebugID; };
	virtual int IndicateEventHandling(void) final { return m_nDebugID; };

	const int m_nDebugID = EVENT_DEBUG_ID_INIT;
};

class IGameEventManager2 : public IBaseInterface
{
public:
	virtual	~IGameEventManager2(void) {};

	// load game event descriptions from a file eg "resource\gameevents.res"
	virtual int LoadEventsFromFile(const char *filename) = 0;

	// removes all and anything
	virtual void  Reset() = 0;

	// adds a listener for a particular event
	virtual bool AddListener(IGameEventListener2 *listener, const char *name, bool bServerSide) = 0;

	// returns true if this listener is listens to given event
	virtual bool FindListener(IGameEventListener2 *listener, const char *name) = 0;

	// removes a listener 
	virtual void RemoveListener(IGameEventListener2 *listener) = 0;

	// create an event by name, but doesn't fire it. returns NULL is event is not
	// known or no listener is registered for it. bForce forces the creation even if no listener is active
	virtual IGameEvent *CreateEvent(const char *name, bool bForce = false) = 0;

	// fires a server event created earlier, if bDontBroadcast is set, event is not send to clients
	virtual bool FireEvent(IGameEvent *event, bool bDontBroadcast = false) = 0;

	// fires an event for the local client only, should be used only by client code
	virtual bool FireEventClientSide(IGameEvent *event) = 0;

	// create a new copy of this event, must be free later
	virtual IGameEvent *DuplicateEvent(IGameEvent *event) = 0;

	// if an event was created but not fired for some reason, it has to bee freed, same UnserializeEvent
	virtual void FreeEvent(IGameEvent *event) = 0;

	// write/read event to/from bitbuffer
	virtual bool SerializeEvent(IGameEvent *event, void* buf) = 0;
	virtual IGameEvent *UnserializeEvent(void* buf) = 0; // create new KeyValues, must be deleted

	virtual KeyValues* GetEventDataTypes(IGameEvent* event) = 0;
};

class IGameEventListener
{
public:
	virtual	~IGameEventListener(void) {};

	// FireEvent is called by EventManager if event just occured
	// KeyValue memory will be freed by manager if not needed anymore
	virtual void FireGameEvent(KeyValues * event) = 0;
};

class IGameEventManager : public IBaseInterface
{
public:
	virtual	~IGameEventManager(void) {};

	// load game event descriptions from a file eg "resource\gameevents.res"
	virtual int LoadEventsFromFile(const char * filename) = 0;

	// removes all and anything
	virtual void  Reset() = 0;

	virtual KeyValues *GetEvent(const char * name) = 0; // returns keys for event

														// adds a listener for a particular event
	virtual bool AddListener(IGameEventListener * listener, const char * event, bool bIsServerSide) = 0;
	// registers for all known events
	virtual bool AddListener(IGameEventListener * listener, bool bIsServerSide) = 0;

	// removes a listener 
	virtual void RemoveListener(IGameEventListener * listener) = 0;

	// fires an global event, specific event data is stored in KeyValues
	// local listeners will receive the event instantly
	// a network message will be send to all connected clients to invoke
	// the same event there
	virtual bool FireEvent(KeyValues * event) = 0;

	// fire a side server event, that it wont be broadcasted to player clients
	virtual bool FireEventServerOnly(KeyValues * event) = 0;

	// fires an event only on this local client
	// can be used to fake events coming over the network 
	virtual bool FireEventClientOnly(KeyValues * event) = 0;

	// write/read event to/from bitbuffer
	virtual bool SerializeKeyValues(KeyValues *event, void* buf, void* eventtype = NULL) = 0;
	virtual KeyValues *UnserializeKeyValue(void* msg) = 0; // create new KeyValues, must be deleted
};

