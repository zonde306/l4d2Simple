#pragma once
#include "INetChannelInfo.h"
#include "../Structs/handle.h"
#include "../Utils/checksum_crc.h"
#include "../Utils/utlvector.h"
#include "../Utils/utlbitvec.h"
#include "../../l4d2Simple2/xorstr.h"
#include "../../l4d2Simple2/vector.h"

class KeyValues;
class NET_Tick;
class NET_StringCmd;
class NET_SetConVar;
class NET_SignonState;
class NET_SplitScreenUser;
class SVC_Print;
class SVC_ServerInfo;
class SVC_SendTable;
class SVC_ClassInfo;
class SVC_SetPause;
class SVC_CreateStringTable;
class SVC_UpdateStringTable;
class SVC_SetView;
class SVC_PacketEntities;
class SVC_Menu;
class SVC_GameEventList;
class SVC_GetCvarValue;
class SVC_SplitScreen;
class SVC_CmdKeyValues;
class SVC_VoiceInit;
class SVC_VoiceData;
class SVC_Sounds;
class SVC_FixAngle;
class SVC_CrosshairAngle;
class SVC_BSPDecal;
class SVC_GameEvent;
class SVC_UserMessage;
class SVC_EntityMessage;
class SVC_TempEntities;
class SVC_Prefetch;

typedef struct netpacket_s netpacket_t;
#define MAX_USER_MSG_DATA 255
#define NETMSG_TYPE_BITS		6	// must be 2^NETMSG_TYPE_BITS > SVC_LASTMSG
#define MAX_EVENT_NAME_LENGTH	32		// max game event name length
#define MAX_EVENT_BITS			9		// max bits needed for an event index
#define MAX_EVENT_NUMBER		(1<<MAX_EVENT_BITS)		// max number of events 

class IConnectionlessPacketHandler
{
public:
	virtual	~IConnectionlessPacketHandler(void) {};

	virtual bool ProcessConnectionlessPacket(netpacket_t *packet) = 0;	// process a connectionless packet
};

#define PROCESS_NET_MESSAGE( name )	\
	virtual bool Process##name( NET_##name *msg )

#define PROCESS_SVC_MESSAGE( name )	\
	virtual bool Process##name( SVC_##name *msg )

#define PROCESS_CLC_MESSAGE( name )	\
	virtual bool Process##name( CLC_##name *msg )

#define PROCESS_MM_MESSAGE( name )	\
	virtual bool Process##name( MM_##name *msg )

class INetMessageHandler
{
public:
	virtual ~INetMessageHandler(void) {};

	PROCESS_NET_MESSAGE(Tick) = 0;
	PROCESS_NET_MESSAGE(StringCmd) = 0;
	PROCESS_NET_MESSAGE(SetConVar) = 0;
	PROCESS_NET_MESSAGE(SignonState) = 0;
};

class IServerMessageHandler : public INetMessageHandler
{
public:
	virtual ~IServerMessageHandler(void) {};

	// Returns dem file protocol version, or, if not playing a demo, just returns PROTOCOL_VERSION
	virtual int GetDemoProtocolVersion() const = 0;

	PROCESS_SVC_MESSAGE(Print) = 0;
	PROCESS_SVC_MESSAGE(ServerInfo) = 0;
	PROCESS_SVC_MESSAGE(SendTable) = 0;
	PROCESS_SVC_MESSAGE(ClassInfo) = 0;
	PROCESS_SVC_MESSAGE(SetPause) = 0;
	PROCESS_SVC_MESSAGE(CreateStringTable) = 0;
	PROCESS_SVC_MESSAGE(UpdateStringTable) = 0;
	PROCESS_SVC_MESSAGE(VoiceInit) = 0;
	PROCESS_SVC_MESSAGE(VoiceData) = 0;
	PROCESS_SVC_MESSAGE(Sounds) = 0;
	PROCESS_SVC_MESSAGE(SetView) = 0;
	PROCESS_SVC_MESSAGE(FixAngle) = 0;
	PROCESS_SVC_MESSAGE(CrosshairAngle) = 0;
	PROCESS_SVC_MESSAGE(BSPDecal) = 0;
	PROCESS_SVC_MESSAGE(GameEvent) = 0;
	PROCESS_SVC_MESSAGE(UserMessage) = 0;
	PROCESS_SVC_MESSAGE(EntityMessage) = 0;
	PROCESS_SVC_MESSAGE(PacketEntities) = 0;
	PROCESS_SVC_MESSAGE(TempEntities) = 0;
	PROCESS_SVC_MESSAGE(Prefetch) = 0;
	PROCESS_SVC_MESSAGE(Menu) = 0;
	PROCESS_SVC_MESSAGE(GameEventList) = 0;
	PROCESS_SVC_MESSAGE(GetCvarValue) = 0;
	PROCESS_SVC_MESSAGE(CmdKeyValues) = 0;
};

class CClockDriftMgr
{
	friend class CBaseClientState;

public:
	CClockDriftMgr() {};

	// Is clock correction even enabled right now?
	static bool IsClockCorrectionEnabled();

	// Clear our state.
	void Clear();

	// This is called each time a server packet comes in. It is used to correlate
	// where the server is in time compared to us.
	void SetServerTick(int iServerTick);

	// Pass in the frametime you would use, and it will drift it towards the server clock.
	float AdjustFrameTime(float inputFrameTime);

	// Returns how many ticks ahead of the server the client is.
	float GetCurrentClockDifference() const;

private:
	void ShowDebugInfo(float flAdjustment);

	// This scales the offsets so the average produced is equal to the
	// current average + flAmount. This way, as we add corrections,
	// we lower the average accordingly so we don't keep responding
	// as much as we need to after we'd adjusted it a couple times.
	void AdjustAverageDifferenceBy(float flAmountInSeconds);

private:
	enum
	{
		// This controls how much it smoothes out the samples from the server.
		NUM_CLOCKDRIFT_SAMPLES = 16
	};

	// This holds how many ticks the client is ahead each time we get a server tick.
	// We average these together to get our estimate of how far ahead we are.
	float m_ClockOffsets[NUM_CLOCKDRIFT_SAMPLES];
	int m_iCurClockOffset;

	int m_nServerTick; // Last-received tick from the server.
	int m_nClientTick; // The client's own tick counter (specifically, for interpolation during rendering).
					   // The server may be on a slightly different tick and the client will drift towards it.
};

class PackedEntity;
class CServerClassInfo;
class CNetworkStringTableContainer;

class CBaseClientState : /*public INetChannelHandler, public IConnectionlessPacketHandler,*/ public IServerMessageHandler
{
public:
	virtual ~CBaseClientState() = 0;
	
	/*
	// Windows 下 CBaseClientState 被分成了两个，这个没有继承 INetChannelHandler 和 IConnectionlessPacketHandler
public:		// INetMsgHandler
	virtual void ConnectionStart(INetChannel *chan) = 0;
	virtual void ConnectionClosing(const char *reason) = 0;
	virtual void ConnectionCrashed(const char *reason) = 0;

	virtual void PacketStart(int incoming_sequence, int outgoing_acknowledged) {};
	virtual void PacketEnd(void) {};

	virtual void FileRequested(const char *fileName, unsigned int transferID) = 0;
	virtual void FileReceived(const char *fileName, unsigned int transferID) = 0;
	virtual void FileDenied(const char *fileName, unsigned int transferID) = 0;
	virtual void FileSent(char const*, unsigned int, bool) = 0;

public:		// IConnectionlessPacketHandler
	virtual bool ProcessConnectionlessPacket(struct netpacket_s *packet) = 0;
	*/

public:		// IServerMessageHandlers
	virtual bool ProcessTick(NET_Tick*) = 0;
	virtual bool ProcessStringCmd(NET_StringCmd*) = 0;
	virtual bool ProcessSetConVar(NET_SetConVar*) = 0;
	virtual bool ProcessSignonState(NET_SignonState*) = 0;
	virtual bool ProcessSplitScreenUser(NET_SplitScreenUser*) = 0;

	virtual bool ProcessPrint(SVC_Print*) = 0;
	virtual bool ProcessServerInfo(SVC_ServerInfo*) = 0;
	virtual bool ProcessSendTable(SVC_SendTable*) = 0;
	virtual bool ProcessClassInfo(SVC_ClassInfo*) = 0;
	virtual bool ProcessSetPause(SVC_SetPause*) = 0;
	virtual bool ProcessCreateStringTable(SVC_CreateStringTable*) = 0;
	virtual bool ProcessUpdateStringTable(SVC_UpdateStringTable*) = 0;
	virtual bool ProcessSetView(SVC_SetView*) = 0;
	virtual bool ProcessPacketEntities(SVC_PacketEntities*) = 0;
	virtual bool ProcessMenu(SVC_Menu*) = 0;
	virtual bool ProcessGameEventList(SVC_GameEventList*) = 0;
	virtual bool ProcessGetCvarValue(SVC_GetCvarValue*) = 0;
	virtual bool ProcessSplitScreen(SVC_SplitScreen*) = 0;
	virtual bool ProcessCmdKeyValues(SVC_CmdKeyValues*) = 0;

public:
	virtual void OnEvent(KeyValues*) = 0;
	virtual void Clear() = 0;
	virtual void FullConnect(netadr_t &adr) = 0; // a connection was established
	virtual void Connect(const char* adr, char const*) = 0; // start a connection challenge
	virtual void ConnectSplitScreen(const char* adr, char const*) = 0; // start a connection challenge
	virtual bool SetSignonState(int state, int count, NET_SignonState*) = 0;
	virtual void Disconnect(bool bShowMainMenu = true) = 0;
	virtual void SendConnectPacket(netadr_s const&, int, int, unsigned long long, bool) = 0;
	virtual const char *GetCDKeyHash() { return "123"; }
	virtual void RunFrame(void) = 0;
	virtual void CheckForResend(void) = 0;
	virtual void CheckForReservationResend() = 0;
	virtual void ResendGameDetailsRequest(netadr_s&) = 0;
	virtual void InstallStringTableCallback(char const *tableName) { }
	virtual bool HookClientStringTable(char const *tableName) { return false; }
	virtual bool LinkClasses(void) = 0;
	virtual int  GetConnectionRetryNumber() const { return 0; }
	virtual const char *GetClientName() { return ""; }
	virtual void ReserveServer(netadr_s const&, netadr_s const&, unsigned long long, KeyValues*, void*, void**) = 0;
	virtual void HandleReservationResponse(netadr_s&, bool) = 0;
	virtual void HandleReserveServerChallengeResponse(int) = 0;
	virtual void SetServerReservationCookie(unsigned long long) = 0;
	
public:
	// Connection to server.			
	int				m_Socket;		// network socket 
	INetChannel		*m_NetChannel;		// Our sequenced channel to the remote server.
	unsigned int	m_nChallengeNr;	// connection challenge number
	double			m_flConnectTime;	// If gap of connect_time to net_time > 3000, then resend connect packet
	int				m_nRetryNumber;	// number of retry connection attemps
	char			m_szRetryAddress[ MAX_OSPATH ];
	int				m_nSignonState;    // see SIGNONSTATE_* definitions
	double			m_flNextCmdTime; // When can we send the next command packet?
	int				m_nServerCount;	// server identification for prespawns, must match the svs.spawncount which
									// is incremented on server spawning.  This supercedes svs.spawn_issued, in that
									// we can now spend a fair amount of time sitting connected to the server
									// but downloading models, sounds, etc.  So much time that it is possible that the
									// server might change levels again and, if so, we need to know that.
	int			m_nCurrentSequence;	// this is the sequence number of the current incoming packet	

	CClockDriftMgr m_ClockDriftMgr;

	int			m_nDeltaTick;		//	last valid received snapshot (server) tick
	bool		m_bPaused;			// send over by server
	int			m_nViewEntity;		// cl_entitites[cl.viewentity] == player point of view

	int			m_nPlayerSlot;		// own player entity index-1. skips world. Add 1 to get cl_entitites index;

	char		m_szLevelName[40];	// for display on solo scoreboard
	char		m_szLevelNameShort[ 40 ]; // removes maps/ and .bsp extension

	int			m_nMaxClients;		// max clients on server

	PackedEntity	*m_pEntityBaselines[2][MAX_EDICTS];	// storing entity baselines
		
	// This stuff manages the receiving of data tables and instantiating of client versions
	// of server-side classes.
	CServerClassInfo	*m_pServerClasses;
	int					m_nServerClasses;
	int					m_nServerClassBits;
	char				m_szEncrytionKey[STEAM_KEYSIZE];
	unsigned int		m_iEncryptionKeySize;

	CNetworkStringTableContainer *m_StringTableContainer;
	
	bool m_bRestrictServerCommands;	// If true, then the server is only allowed to execute commands marked with FCVAR_SERVER_CAN_EXECUTE on the client.
	bool m_bRestrictClientCommands;	// If true, then IVEngineClient::ClientCmd is only allowed to execute commands marked with FCVAR_CLIENTCMD_CAN_EXECUTE on the client.
};

class CClientState : public IServerMessageHandler
{
public:
	LPVOID __vtbl;
	BYTE __pad[0x4A3C];
	int lastoutgoingcommand;	// Sequence number of last outgoing command
	int chokedcommands;			// number of choked commands
};

#define DECLARE_BASE_MESSAGE(msgtype)                \
public:                                              \
	bool ReadFromBuffer(bf_read &buffer);            \
	bool WriteToBuffer(bf_write &buffer);            \
	const char *ToString() const { return ""; }      \
	const char *GetName() const { return #msgtype; }
	// int GetType() const { return msgtype; }

#define DECLARE_NET_MESSAGE(name)     \
	DECLARE_BASE_MESSAGE(NET_##name); \
	void *m_pMessageHandler;          \
	bool Process() { return true; }

#define DECLARE_SVC_MESSAGE(name)     \
	DECLARE_BASE_MESSAGE(SVC_##name); \
	void *m_pMessageHandler;          \
	bool Process() { return true; }

#define DECLARE_CLC_MESSAGE(name)     \
	DECLARE_BASE_MESSAGE(CLC_##name); \
	void *m_pMessageHandler;          \
	bool Process() { return true; }

#define DECLARE_MM_MESSAGE(name)     \
	DECLARE_BASE_MESSAGE(MM_##name); \
	void *m_pMessageHandler;         \
	bool Process() { return true; }

class CNetMessage : public INetMessage
{
public:
	CNetMessage()
	{
		m_bReliable = true;
		m_NetChannel = NULL;
	}

	virtual ~CNetMessage() {};
	virtual void SetNetChannel(INetChannel *netchan) { m_NetChannel = netchan; }
	virtual void SetReliable(bool state) { m_bReliable = state; };
	virtual bool Process() { return false; }; // no handler set
	virtual void BIncomingMessageForProcessing(double, int, INetMessage*) {};	// 不知道这是什么
	virtual bool ReadFromBuffer(bf_read &buffer) { return false; };
	virtual bool WriteToBuffer(bf_write &buffer) { return false; };
	virtual bool IsReliable() const { return m_bReliable; };
	virtual int GetType(void) const { return 0; };
	virtual int GetGroup() const { return INetChannelInfo::GENERIC; }
	virtual const char *GetName(void) const { return ""; };
	INetChannel *GetNetChannel() const { return m_NetChannel; }
	virtual int GetSize(void) const { return sizeof(CNetMessage); };
	virtual void SetRateLimitPolicy(LPVOID) { };

protected:
	bool m_bReliable;		   // true if message should be send reliable
	INetChannel *m_NetChannel; // netchannel this message is from/for
	byte __padding[68];
};

typedef enum
{
	eQueryCvarValueStatus_ValueIntact = 0, // It got the value fine.
	eQueryCvarValueStatus_CvarNotFound = 1,
	eQueryCvarValueStatus_NotACvar = 2, // There's a ConCommand, but it's not a ConVar.
	eQueryCvarValueStatus_CvarProtected = 3  // The cvar was marked with FCVAR_SERVER_CAN_NOT_QUERY, so the server is not allowed to have its value.
} EQueryCvarValueStatus;

class NET_SetConVar : public CNetMessage
{
public:
	DECLARE_NET_MESSAGE(SetConVar);

	int GetType(void) const { return 0x6; };
	int	GetGroup() const { return INetChannelInfo::STRINGCMD; }
	
	NET_SetConVar() {}
	NET_SetConVar(const char * name, const char * value)
	{
		cvar_t cvar;
		strncpy(cvar.name, name, 260);
		strncpy(cvar.value, value, 260);
		m_ConVars.AddToTail(cvar);
	}

public:

	typedef struct cvar_s
	{
		char	name[260];
		char	value[260];
	} cvar_t;

	CUtlVector<cvar_t> m_ConVars;
};

class SVC_GetCvarValue : public CNetMessage
{
public:
	DECLARE_SVC_MESSAGE(GetCvarValue);

	int GetType(void) const { return 0x1F; };

	int					m_iCookie;
	const char			*m_szCvarName;	// The sender sets this, and it automatically points it at m_szCvarNameBuffer when receiving.

private:
	char		m_szCvarNameBuffer[256];
};

class CLC_RespondCvarValue : public CNetMessage
{
public:
	DECLARE_CLC_MESSAGE(RespondCvarValue);

	int GetType(void) const{ return 0x0D; };
	int GetSize(void) const { return sizeof(CLC_RespondCvarValue); };

	int						m_iCookie;

	const char				*m_szCvarName;
	const char				*m_szCvarValue;	// The sender sets this, and it automatically points it at m_szCvarNameBuffer when receiving.

	EQueryCvarValueStatus	m_eStatusCode;

private:
	char		m_szCvarNameBuffer[256];
	char		m_szCvarValueBuffer[256];
};

class NET_StringCmd : public CNetMessage
{
	DECLARE_NET_MESSAGE(StringCmd);

	int GetType(void) const { return 0x5; };
	int	GetGroup() const { return INetChannelInfo::STRINGCMD; }

	NET_StringCmd() { m_szCommand = NULL; };
	NET_StringCmd(const char *cmd) { m_szCommand = cmd; };

public:
	const char	*m_szCommand;	// execute this command

private:
	char		m_szCommandBuffer[1024];	// buffer for received messages
};

class CLC_Move : public CNetMessage
{
	DECLARE_CLC_MESSAGE(Move);

	int GetType(void) const { return 0x9; };
	int	GetGroup() const { return INetChannelInfo::MOVE; }

	CLC_Move() { m_bReliable = false; }

public:
	int				m_nBackupCommands;
	int				m_nNewCommands;
	int				m_nLength;
	bf_read			m_DataIn;
	bf_write		m_DataOut;
};

class SVC_Print : public CNetMessage
{
	DECLARE_SVC_MESSAGE(Print);

	int GetType(void) const { return 0x10; };
	SVC_Print() { m_bReliable = false; m_szText = NULL; };

	SVC_Print(const char * text) { m_bReliable = false; m_szText = text; };

public:
	const char	*m_szText;	// show this text

private:
	char		m_szTextBuffer[2048];	// buffer for received messages
};

class SVC_ServerInfo : public CNetMessage
{
	DECLARE_SVC_MESSAGE(ServerInfo);

	int GetType(void) const { return 0x8; };
	int	GetGroup() const { return INetChannelInfo::SIGNON; }

public:	// member vars are public for faster handling
	int			m_nProtocol;	// protocol version
	int			m_nServerCount;	// number of changelevels since server start
	bool		m_bIsDedicated;  // dedicated server ?	
	bool		m_bIsHLTV;		// HLTV server ?
	char		m_cOS;			// L = linux, W = Win32
	CRC32_t		m_nMapCRC;		// server map CRC
	CRC32_t		m_nClientCRC;	// client.dll CRC server is using
	int			m_nMaxClients;	// max number of clients on server
	int			m_nMaxClasses;	// max number of server classes
	int			m_nPlayerSlot;	// our client slot number
	float		m_fTickInterval;// server tick interval
	const char	*m_szGameDir;	// game directory eg "tf2"
	const char	*m_szMapName;	// name of current map 
	const char	*m_szSkyName;	// name of current skybox 
	const char	*m_szHostName;	// server name

private:
	char		m_szGameDirBuffer[256];// game directory eg "left4dead2"
	char		m_szMapNameBuffer[256];// name of current map 
	char		m_szSkyNameBuffer[256];// name of current skybox 
	char		m_szHostNameBuffer[256];// name of current skybox 
};

class SVC_UserMessage : public CNetMessage
{
	DECLARE_SVC_MESSAGE(UserMessage);

	SVC_UserMessage() { m_bReliable = false; }

	int GetType(void) const { return 0x17; };
	int	GetGroup() const { return INetChannelInfo::USERMESSAGES; }

public:
	int			m_nMsgType;
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_EntityMessage : public CNetMessage
{
	DECLARE_SVC_MESSAGE(EntityMessage);

	int GetType(void) const { return 0x18; };
	SVC_EntityMessage() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::ENTMESSAGES; }

public:
	int			m_nEntityIndex;
	int			m_nClassID;
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_PacketEntities : public CNetMessage
{
	DECLARE_SVC_MESSAGE(PacketEntities);

	int GetType(void) const { return 0x1A; };
	int	GetGroup() const { return INetChannelInfo::ENTITIES; }

public:

	int			m_nMaxEntries;
	int			m_nUpdatedEntries;
	bool		m_bIsDelta;
	bool		m_bUpdateBaseline;
	int			m_nBaseline;
	int			m_nDeltaFrom;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_TempEntities : public CNetMessage
{
	DECLARE_SVC_MESSAGE(TempEntities);

	int GetType(void) const { return 0x1B; };
	SVC_TempEntities() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::EVENTS; }

	int			m_nNumEntries;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_GameEvent : public CNetMessage
{
	DECLARE_SVC_MESSAGE(GameEvent);

	int GetType(void) const { return 0x5; };
	int	GetGroup() const { return INetChannelInfo::EVENTS; }

public:
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_BSPDecal : public CNetMessage
{
	DECLARE_SVC_MESSAGE(BSPDecal);

	int GetType(void) const { return 0x15; };

public:
	Vector		m_Pos;
	int			m_nDecalTextureIndex;
	int			m_nEntityIndex;
	int			m_nModelIndex;
	bool		m_bLowPriority;
};

class SVC_CrosshairAngle : public CNetMessage
{
	DECLARE_SVC_MESSAGE(CrosshairAngle);

	int GetType(void) const { return 0x14; };
	SVC_CrosshairAngle() {}
	SVC_CrosshairAngle(QAngle angle) { m_Angle = angle; }

public:
	QAngle			m_Angle;
};

class SVC_FixAngle : public CNetMessage
{
	DECLARE_SVC_MESSAGE(FixAngle);

	int GetType(void) const { return 0x13; };
	SVC_FixAngle() { m_bReliable = false; };
	SVC_FixAngle(bool bRelative, QAngle angle)
	{
		m_bReliable = false; m_bRelative = bRelative; m_Angle = angle;
	}

public:
	bool			m_bRelative;
	QAngle			m_Angle;
};

class SVC_SetView : public CNetMessage
{
	DECLARE_SVC_MESSAGE(SetView);

	int GetType(void) const { return 0x12; };
	SVC_SetView() {}
	SVC_SetView(int entity) { m_nEntityIndex = entity; }

public:
	int				m_nEntityIndex;
};

class SVC_UpdateStringTable : public CNetMessage
{
	DECLARE_SVC_MESSAGE(UpdateStringTable);

	int GetType(void) const { return 0x0D; };
	int	GetGroup() const { return INetChannelInfo::STRINGTABLE; }

public:
	int				m_nTableID;	// table to be updated
	int				m_nChangedEntries; // number of how many entries has changed
	int				m_nLength;	// data length in bits
	bf_read			m_DataIn;
	bf_write		m_DataOut;
};

class SVC_SetPause : public CNetMessage
{
	DECLARE_SVC_MESSAGE(SetPause);

	int GetType(void) const { return 0x0B; };
	SVC_SetPause() {}
	SVC_SetPause(bool state) { m_bPaused = state; }

public:
	bool		m_bPaused;		// true or false, what else
};

class CLC_FileCRCCheck : public CNetMessage
{
public:
	DECLARE_CLC_MESSAGE(FileCRCCheck);

	int GetType(void) const { return 0x0E; };

	char		m_szPathID[MAX_PATH];
	char		m_szFilename[MAX_PATH];
	CRC32_t		m_CRC;
};

class NET_SignonState : public CNetMessage
{
	DECLARE_NET_MESSAGE(SignonState);

	int GetType(void) const { return 0x7; };
	int	GetGroup() const { return INetChannelInfo::SIGNON; }

	NET_SignonState() {};
	NET_SignonState(int state, int spawncount) { m_nSignonState = state; m_nSpawnCount = spawncount; };

public:
	int			m_nSignonState;			// See SIGNONSTATE_ defines
	int			m_nSpawnCount;			// server spawn count (session number)
};

class NET_Tick : public CNetMessage
{
	DECLARE_NET_MESSAGE(Tick);

	int GetType(void) const { return 0x4; };

	NET_Tick()
	{
		m_bReliable = false;
		m_flHostFrameTime = 0;
		m_flHostFrameTimeStdDeviation = 0;
	};

	NET_Tick(int tick, float host_frametime, float host_frametime_stddeviation)
	{
		m_bReliable = false;
		m_nTick = tick;
		m_flHostFrameTime = host_frametime;
		m_flHostFrameTimeStdDeviation = host_frametime_stddeviation;
	};

public:
	int			m_nTick;
	float		m_flHostFrameTime;
	float		m_flHostFrameTimeStdDeviation;
};

class CLC_ListenEvents : public CNetMessage
{
	DECLARE_CLC_MESSAGE(ListenEvents);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }
	int GetType(void) const { return 0x0C; };

public:
	CBitVec<MAX_EVENT_NUMBER> m_EventArray;
};

class CLC_ClientInfo : public CNetMessage
{
	DECLARE_CLC_MESSAGE(ClientInfo);
	int GetType(void) const { return 0x8; };

public:
	CRC32_t			m_nSendTableCRC;
	int				m_nServerCount;
	bool			m_bIsHLTV;
	uint32_t			m_nFriendsID;
	char			m_FriendsName[MAX_PLAYER_NAME_LENGTH];
	CRC32_t			m_nCustomFiles[MAX_CUSTOM_FILES];
};
