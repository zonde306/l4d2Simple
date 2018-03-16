#pragma once
#include "IGlobalVarsBase.h"
#include "IInputSystem.h"
#include "ICvar.h"
#include "IBaseFileSystem.h"
#include "../Structs/clientclass.h"
#include "../Utils/utlvector.h"
#include "../Utils/utlbuffer.h"
#include <cstdint>

struct vrect_t
{
	int				x, y, width, height;
	vrect_t			*pnext;
};

struct ScreenFade_t
{
	unsigned short 	duration;		// FIXED 16 bit, with SCREENFADE_FRACBITS fractional, seconds duration
	unsigned short 	holdTime;		// FIXED 16 bit, with SCREENFADE_FRACBITS fractional, seconds duration until reset (fade & hold)
	short			fadeFlags;		// flags
	byte			r, g, b, a;		// fade to color ( max alpha )
};

class CEngineSprite;
class CViewSetup;
typedef int qboolean;

enum ClientFrameStage_t
{
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

typedef struct _XUSER_CONTEXT
{
	DWORD                               dwContextId;
	DWORD                               dwValue;
} XUSER_CONTEXT, *PXUSER_CONTEXT;

typedef struct _XUSER_DATA
{
	BYTE                                type;

	union
	{
		int                            nData;     // XUSER_DATA_TYPE_INT32
		int64_t                        i64Data;   // XUSER_DATA_TYPE_INT64
		double                          dblData;   // XUSER_DATA_TYPE_DOUBLE
		struct                                     // XUSER_DATA_TYPE_UNICODE
		{
			unsigned int                       cbData;    // Includes null-terminator
			char *                      pwszData;
		} string;
		float                           fData;     // XUSER_DATA_TYPE_FLOAT
		struct                                     // XUSER_DATA_TYPE_BINARY
		{
			unsigned int                       cbData;
			char *                       pbData;
		} binary;
	};
} XUSER_DATA, *PXUSER_DATA;

typedef struct _XUSER_PROPERTY
{
	DWORD                               dwPropertyId;
	XUSER_DATA                          value;
} XUSER_PROPERTY, *PXUSER_PROPERTY;

class CMouthInfo;
class CSaveRestoreData;
class CStandardRecvProxies;
class CRenamedRecvTableInfo;

class IBaseClientDll
{
public:
	// Called once when the client DLL is loaded
	virtual int				Init( CreateInterfaceFn appSystemFactory, 
									CreateInterfaceFn physicsFactory,
									CGlobalVarsBase *pGlobals ) = 0;

	// virtual void			PostInit() = 0;

	// Called once when the client DLL is being unloaded
	virtual void			Shutdown( void ) = 0;
	
	// Called once the client is initialized to setup client-side replay interface pointers
	virtual bool			ReplayInit( CreateInterfaceFn replayFactory ) = 0;
	virtual bool			ReplayPostInit() = 0;

	// Called at the start of each level change
	virtual void			LevelInitPreEntity( char const* pMapName ) = 0;
	// Called at the start of a new level, after the entities have been received and created
	virtual void			LevelInitPostEntity( ) = 0;
	// Called at the end of a level
	virtual void			LevelShutdown( void ) = 0;

	// Request a pointer to the list of client datatable classes
	virtual ClientClass		*GetAllClasses( void ) = 0;

	// Called once per level to re-initialize any hud element drawing stuff
	virtual int				HudVidInit( void ) = 0;
	// Called by the engine when gathering user input
	virtual void			HudProcessInput( bool bActive ) = 0;
	// Called oncer per frame to allow the hud elements to think
	virtual void			HudUpdate( bool bActive ) = 0;
	// Reset the hud elements to their initial states
	virtual void			HudReset( void ) = 0;
	// Display a hud text message
	virtual void			HudText( const char * message ) = 0;

	// Mouse Input Interfaces
	// Activate the mouse (hides the cursor and locks it to the center of the screen)
	virtual void			IN_ActivateMouse( void ) = 0;
	// Deactivates the mouse (shows the cursor and unlocks it)
	virtual void			IN_DeactivateMouse( void ) = 0;
	// This is only called during extra sound updates and just accumulates mouse x, y offets and recenters the mouse.
	//  This call is used to try to prevent the mouse from appearing out of the side of a windowed version of the engine if 
	//  rendering or other processing is taking too long
	virtual void			IN_Accumulate (void) = 0;
	// Reset all key and mouse states to their initial, unpressed state
	virtual void			IN_ClearStates (void) = 0;
	// If key is found by name, returns whether it's being held down in isdown, otherwise function returns false
	virtual bool			IN_IsKeyDown( const char *name, bool& isdown ) = 0;
	// Notify the client that the mouse was wheeled while in game - called prior to executing any bound commands.
	virtual void			IN_OnMouseWheeled( int nDelta ) = 0;
	// Raw keyboard signal, if the client .dll returns 1, the engine processes the key as usual, otherwise,
	//  if the client .dll returns 0, the key is swallowed.
	virtual int				IN_KeyEvent( int eventcode, ButtonCode_t keynum, const char *pszCurrentBinding ) = 0;

	// This function is called once per tick to create the player CUserCmd (used for prediction/physics simulation of the player)
	// Because the mouse can be sampled at greater than the tick interval, there is a separate input_sample_frametime, which
	//  specifies how much additional mouse / keyboard simulation to perform.
	virtual void			CreateMove ( 
		int sequence_number,			// sequence_number of this cmd
		float input_sample_frametime,	// Frametime for mouse input sampling
		bool active ) = 0;				// True if the player is active (not paused)

	// If the game is running faster than the tick_interval framerate, then we do extra mouse sampling to avoid jittery input
	//  This code path is much like the normal move creation code, except no move is created
	virtual void			ExtraMouseSample( float frametime, bool active ) = 0;

	// Encode the delta (changes) between the CUserCmd in slot from vs the one in slot to.  The game code will have
	//  matching logic to read the delta.
	virtual bool			WriteUsercmdDeltaToBuffer( bf_write *buf, int from, int to, bool isnewcommand ) = 0;
	// Demos need to be able to encode/decode CUserCmds to memory buffers, so these functions wrap that
	virtual void			EncodeUserCmdToBuffer( bf_write& buf, int slot ) = 0;
	virtual void			DecodeUserCmdFromBuffer( bf_read& buf, int slot ) = 0;

	// Set up and render one or more views (e.g., rear view window, etc.).  This called into RenderView below
	virtual void			View_Render( vrect_t *rect ) = 0;

	// Allow engine to expressly render a view (e.g., during timerefresh)
	// See IVRenderView.h, PushViewFlags_t for nFlags values
	virtual void			RenderView( const CViewSetup &view, int nClearFlags, int whatToDraw ) = 0;

	// Apply screen fade directly from engine
	virtual void			View_Fade( ScreenFade_t *pSF ) = 0;

	// The engine has parsed a crosshair angle message, this function is called to dispatch the new crosshair angle
	virtual void			SetCrosshairAngle( const QAngle& angle ) = 0;

	// Load a .spr file by name
	// Sprite (.spr) model handling code
	virtual void			InitSprite( CEngineSprite *pSprite, const char *loadname ) = 0;
	// Shutdown a .spr file
	virtual void			ShutdownSprite( CEngineSprite *pSprite ) = 0;
	// Returns sizeof( CEngineSprite ) so the engine can allocate appropriate memory
	virtual int				GetSpriteSize( void ) const = 0;

	// Called when a player starts or stops talking.
	// entindex is -1 to represent the local client talking (before the data comes back from the server). 
	// entindex is -2 to represent the local client's voice being acked by the server.
	// entindex is GetPlayer() when the server acknowledges that the local client is talking.
	virtual void			VoiceStatus( int entindex, qboolean bTalking ) = 0;

	// Networked string table definitions have arrived, allow client .dll to 
	//  hook string changes with a callback function ( see INetworkStringTableClient.h )
	virtual void			InstallStringTableCallback( char const *tableName ) = 0;

	// Notification that we're moving into another stage during the frame.
	virtual void			FrameStageNotify( ClientFrameStage_t curStage ) = 0;

	// The engine has received the specified user message, this code is used to dispatch the message handler
	virtual bool			DispatchUserMessage( int msg_type, bf_read &msg_data ) = 0;

	// Save/restore system hooks
	virtual CSaveRestoreData  *SaveInit( int size ) = 0;
	virtual void			SaveWriteFields( CSaveRestoreData *, const char *, void *, datamap_t *, typedescription_t *, int ) = 0;
	virtual void			SaveReadFields( CSaveRestoreData *, const char *, void *, datamap_t *, typedescription_t *, int ) = 0;
	virtual void			PreSave( CSaveRestoreData * ) = 0;
	virtual void			Save( CSaveRestoreData * ) = 0;
	virtual void			WriteSaveHeaders( CSaveRestoreData * ) = 0;
	virtual void			ReadRestoreHeaders( CSaveRestoreData * ) = 0;
	virtual void			Restore( CSaveRestoreData *, bool ) = 0;
	virtual void			DispatchOnRestore() = 0;

	// Hand over the StandardRecvProxies in the client DLL's module.
	virtual CStandardRecvProxies* GetStandardRecvProxies() = 0;

	// save game screenshot writing
	virtual void			WriteSaveGameScreenshot( const char *pFilename ) = 0;

	// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
	//  the appropriate close caption if running with closecaption = 1
	virtual void			EmitSentenceCloseCaption( char const *tokenstream ) = 0;
	// Emits a regular close caption by token name
	virtual void			EmitCloseCaption( char const *captionname, float duration ) = 0;

	// Returns true if the client can start recording a demo now.  If the client returns false,
	// an error message of up to length bytes should be returned in errorMsg.
	virtual bool			CanRecordDemo( char *errorMsg, int length ) const = 0;

	// Give the Client a chance to do setup/cleanup.
	virtual void			OnDemoRecordStart( char const* pDemoBaseName ) = 0;
	virtual void			OnDemoRecordStop() = 0;
	virtual void			OnDemoPlaybackStart( char const* pDemoBaseName ) = 0;
	virtual void			OnDemoPlaybackStop() = 0;

	// Draw the console overlay?
	virtual bool			ShouldDrawDropdownConsole() = 0;

	// Get client screen dimensions
	virtual int				GetScreenWidth() = 0;
	virtual int				GetScreenHeight() = 0;

	// Added interface

	// save game screenshot writing
	virtual void			WriteSaveGameScreenshotOfSize( const char *pFilename, int width, int height, bool bCreatePowerOf2Padded = false, bool bWriteVTF = false ) = 0;

	// Gets the current view
	virtual bool			GetPlayerView( CViewSetup &playerView ) = 0;

	// Matchmaking
	virtual void			SetupGameProperties( CUtlVector< XUSER_CONTEXT > &contexts, CUtlVector< XUSER_PROPERTY > &properties ) = 0;
	virtual unsigned int	GetPresenceID( const char *pIDName ) = 0;
	virtual const char		*GetPropertyIdString( const unsigned int id ) = 0;
	virtual void			GetPropertyDisplayString(unsigned int id, unsigned int value, char *pOutput, int nBytes ) = 0;

#ifdef WIN32
	virtual void			StartStatsReporting( HANDLE handle, bool bArbitrated ) = 0;
#endif
	
	virtual void			InvalidateMdlCache() = 0;

	virtual void			IN_SetSampleTime( float frametime ) = 0;


	// For sv_pure mode. The filesystem figures out which files the client needs to reload to be "pure" ala the server's preferences.
	virtual void			ReloadFilesInList( IFileList *pFilesToReload ) = 0;
#ifdef POSIX
	// AR: Same as above win32 defn but down here at the end of the vtable for back compat
	virtual void			StartStatsReporting( HANDLE handle, bool bArbitrated ) = 0;
#endif

	// Let the client handle UI toggle - if this function returns false, the UI will toggle, otherwise it will not.
	virtual bool			HandleUiToggle() = 0;

	// Allow the console to be shown?
	virtual bool			ShouldAllowConsole() = 0;

	// Get renamed recv tables
	virtual CRenamedRecvTableInfo	*GetRenamedRecvTableInfos() = 0;

	// Get the mouthinfo for the sound being played inside UI panels
	virtual CMouthInfo		*GetClientUIMouthInfo() = 0;

	// Notify the client that a file has been received from the game server
	virtual void			FileReceived( const char * fileName, unsigned int transferID ) = 0;
	
	virtual const char* TranslateEffectForVisionFilter( const char *pchEffectType, const char *pchEffectName ) = 0;

	// Give the client a chance to modify sound settings however they want before the sound plays. This is used for
	// things like adjusting pitch of voice lines in Pyroland in TF2.
	virtual void			ClientAdjustStartSoundParams( struct StartSoundParams_t& params ) = 0;

	// Returns true if the disconnect command has been handled by the client
	virtual bool DisconnectAttempt( void ) = 0;

	virtual bool IsConnectedUserInfoChangeAllowed( IConVar *pCvar ) = 0;
};