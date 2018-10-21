#pragma once
#include "../Utils/utlvector.h"
#include <Windows.h>

//-----------------------------------------------------------------------------
// forward declaration
//-----------------------------------------------------------------------------
class Vector;

// Handy defines for EmitSound
#define SOUND_FROM_UI_PANEL			-2		// Sound being played inside a UI panel on the client
#define SOUND_FROM_LOCAL_PLAYER		-1
#define SOUND_FROM_WORLD			0

// These are used to feed a soundlevel to the sound system and have it use
// goldsrc-type attenuation. We should use this as little as possible and 
// phase it out as soon as possible.

// Take a regular sndlevel and convert it to compatibility mode.
#define SNDLEVEL_TO_COMPATIBILITY_MODE( x )		((SoundLevel_t)(int)( (x) + 256 ))

// Take a compatibility-mode sndlevel and get the REAL sndlevel out of it.
#define SNDLEVEL_FROM_COMPATIBILITY_MODE( x )	((SoundLevel_t)(int)( (x) - 256 ))

// Tells if the given sndlevel is marked as compatibility mode.
#define SNDLEVEL_IS_COMPATIBILITY_MODE( x )		( (x) >= 256 )

//-----------------------------------------------------------------------------
// Client-server neutral effects interface
//-----------------------------------------------------------------------------
#define IENGINESOUND_CLIENT_INTERFACE_VERSION	"IEngineSoundClient003"
#define IENGINESOUND_SERVER_INTERFACE_VERSION	"IEngineSoundServer003"

#define SNDVOL_NORMAL		1.0		/**< Normal volume */
#define SNDPITCH_NORMAL		100		/**< Normal pitch */
#define SNDPITCH_LOW		95		/**< A low pitch */
#define SNDPITCH_HIGH		120		/**< A high pitch */
#define SNDATTN_NONE		0.0		/**< No attenuation */
#define SNDATTN_NORMAL		0.8		/**< Normal attenuation */
#define SNDATTN_STATIC		1.25	/**< Static attenuation? */
#define SNDATTN_RICOCHET	1.5		/**< Ricochet effect */
#define SNDATTN_IDLE		2.0		/**< Idle attenuation? */

//-----------------------------------------------------------------------------
// channels
//-----------------------------------------------------------------------------
enum
{
	SNDCHAN_REPLACE = -1,
	SNDCHAN_AUTO = 0,
	SNDCHAN_WEAPON = 1,
	SNDCHAN_VOICE = 2,
	SNDCHAN_ITEM = 3,
	SNDCHAN_BODY = 4,
	SNDCHAN_STREAM = 5,								// allocate stream channel from the static or dynamic area
	SNDCHAN_STATIC = 6,								// allocate channel from the static area 
	SNDCHAN_VOICE2 = 7,
	SNDCHAN_VOICE_BASE = 8,							// allocate channel for network voice data
	SNDCHAN_USER_BASE = (SNDCHAN_VOICE_BASE + 128)	// Anything >= this number is allocated to game code.
};

//-----------------------------------------------------------------------------
// Flags to be or-ed together for the iFlags field
//-----------------------------------------------------------------------------
enum SoundFlags_t
{
	SND_NOFLAGS = 0,				// to keep the compiler happy
	SND_CHANGE_VOL = (1 << 0),		// change sound vol
	SND_CHANGE_PITCH = (1 << 1),	// change sound pitch
	SND_STOP = (1 << 2),			// stop the sound
	SND_SPAWNING = (1 << 3),		// we're spawning, used in some cases for ambients
									// not sent over net, only a param between dll and server.
	SND_DELAY = (1 << 4),			// sound has an initial delay
	SND_STOP_LOOPING = (1 << 5),	// stop all looping sounds on the entity.
	SND_SPEAKER = (1 << 6),			// being played again by a microphone through a speaker

	SND_SHOULDPAUSE = (1 << 7),		// this sound should be paused if the game is paused
	SND_IGNORE_PHONEMES = (1 << 8),
	SND_IGNORE_NAME = (1 << 9),		// used to change all sounds emitted by an entity, regardless of scriptname

	SND_DO_NOT_OVERWRITE_EXISTING_ON_CHANNEL = (1 << 10),
};

/**
* Various predefined sound levels in dB.
*/
enum SoundLevel_t
{
	SNDLEVEL_NONE = 0,			/**< None */
	SNDLEVEL_RUSTLE = 20,		/**< Rustling leaves */
	SNDLEVEL_WHISPER = 25,		/**< Whispering */
	SNDLEVEL_LIBRARY = 30,		/**< In a library */
	SNDLEVEL_FRIDGE = 45,		/**< Refrigerator */
	SNDLEVEL_HOME = 50,			/**< Average home (3.9 attn) */
	SNDLEVEL_CONVO = 60,		/**< Normal conversation (2.0 attn) */
	SNDLEVEL_DRYER = 60,		/**< Clothes dryer */
	SNDLEVEL_DISHWASHER = 65,	/**< Dishwasher/washing machine (1.5 attn) */
	SNDLEVEL_CAR = 70,			/**< Car or vacuum cleaner (1.0 attn) */
	SNDLEVEL_NORMAL = 75,		/**< Normal sound level */
	SNDLEVEL_TRAFFIC = 75,		/**< Busy traffic (0.8 attn) */
	SNDLEVEL_MINIBIKE = 80,		/**< Mini-bike, alarm clock (0.7 attn) */
	SNDLEVEL_SCREAMING = 90,	/**< Screaming child (0.5 attn) */
	SNDLEVEL_TRAIN = 100,		/**< Subway train, pneumatic drill (0.4 attn) */
	SNDLEVEL_HELICOPTER = 105,	/**< Helicopter */
	SNDLEVEL_SNOWMOBILE = 110,	/**< Snow mobile */
	SNDLEVEL_AIRCRAFT = 120,	/**< Auto horn, aircraft */
	SNDLEVEL_RAIDSIREN = 130,	/**< Air raid siren */
	SNDLEVEL_GUNFIRE = 140,		/**< Gunshot, jet engine (0.27 attn) */
	SNDLEVEL_ROCKET = 180,		/**< Rocket launching (0.2 attn) */
};

class IRecipientFilter
{
public:
	virtual			~IRecipientFilter() {}

	virtual bool	IsReliable(void) const = 0;
	virtual bool	IsInitMessage(void) const = 0;

	virtual int		GetRecipientCount(void) const = 0;
	virtual int		GetRecipientIndex(int slot) const = 0;
};

// The handle is a CUtlSymbol for the dirname and the same for the filename, the accessor
//  copies them into a static char buffer for return.
typedef void* FileNameHandle_t;
#define FILENAMEHANDLE_INVALID 0

//-----------------------------------------------------------------------------
// Purpose:  Client side only 
//-----------------------------------------------------------------------------
struct SndInfo_t
{
	// Sound Guid
	int			m_nGuid;
	FileNameHandle_t m_filenameHandle;		// filesystem filename handle - call IFilesystem to conver this to a string
	int			m_nSoundSource;
	int			m_nChannel;
	// If a sound is being played through a speaker entity (e.g., on a monitor,), this is the
	//  entity upon which to show the lips moving, if the sound has sentence data
	int			m_nSpeakerEntity;
	float		m_flVolume;
	float		m_flLastSpatializedVolume;
	// Radius of this sound effect (spatialization is different within the radius)
	float		m_flRadius;
	int			m_nPitch;
	Vector		*m_pOrigin;
	Vector		*m_pDirection;

	// if true, assume sound source can move and update according to entity
	bool		m_bUpdatePositions;
	// true if playing linked sentence
	bool		m_bIsSentence;
	// if true, bypass all dsp processing for this sound (ie: music)	
	bool		m_bDryMix;
	// true if sound is playing through in-game speaker entity.
	bool		m_bSpeaker;
	// true if sound is playing with special DSP effect
	bool		m_bSpecialDSP;
	// for snd_show, networked sounds get colored differently than local sounds
	bool		m_bFromServer;
};

class IEngineSound
{
public:
	// Precache a particular sample
	virtual bool PrecacheSound(const char *pSample, bool bPreload = false, bool bIsUISound = false) = 0;
	virtual bool IsSoundPrecached(const char *pSample) = 0;
	virtual void PrefetchSound(const char *pSample) = 0;
	virtual bool IsLoopingSound(const char *pSample) = 0;

	// Just loads the file header and checks for duration (not hooked up for .mp3's yet)
	// Is accessible to server and client though
	virtual float GetSoundDuration(const char *pSample) = 0;

	// Pitch of 100 is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
	// down to 1 is a lower pitch.   150 to 70 is the realistic range.
	// EmitSound with pitch != 100 should be used sparingly, as it's not quite as
	// fast (the pitchshift mixer is not native coded).

	// NOTE: setting iEntIndex to -1 will cause the sound to be emitted from the local
	// player (client-side only)
	virtual void EmitSound(IRecipientFilter& filter, int iEntIndex, int iChannel, const char *pSample,
		float flVolume, SoundLevel_t iSoundlevel, int iFlags = 0, int iPitch = SNDPITCH_NORMAL,
		const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void EmitSound(IRecipientFilter& filter, int iEntIndex, int iChannel, const char *pSample,
		float flVolume, float flAttenuation, int iFlags = 0, int iPitch = SNDPITCH_NORMAL,
		const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void EmitSentenceByIndex(IRecipientFilter& filter, int iEntIndex, int iChannel, int iSentenceIndex,
		float flVolume, SoundLevel_t iSoundlevel, int iFlags = 0, int iPitch = SNDPITCH_NORMAL,
		const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1) = 0;

	virtual void StopSound(int iEntIndex, int iChannel, const char *pSample) = 0;

	// stop all active sounds (client only)
	virtual void StopAllSounds(bool bClearBuffers) = 0;

	// Set the room type for a player (client only)
	virtual void SetRoomType(IRecipientFilter& filter, int roomType) = 0;

	// Set the dsp preset for a player (client only)
	virtual void SetPlayerDSP(IRecipientFilter& filter, int dspType, bool fastReset) = 0;

	// emit an "ambient" sound that isn't spatialized
	// only available on the client, assert on server
	virtual void EmitAmbientSound(const char *pSample, float flVolume, int iPitch = SNDPITCH_NORMAL, int flags = 0, float soundtime = 0.0f) = 0;


	//	virtual EntChannel_t	CreateEntChannel() = 0;

	virtual float GetDistGainFromSoundLevel(SoundLevel_t soundlevel, float dist) = 0;

	// Client .dll only functions
	virtual int		GetGuidForLastSoundEmitted() = 0;
	virtual bool	IsSoundStillPlaying(int guid) = 0;
	virtual void	StopSoundByGuid(int guid) = 0;
	// Set's master volume (0.0->1.0)
	virtual void	SetVolumeByGuid(int guid, float fvol) = 0;

	// Retrieves list of all active sounds
	virtual void	GetActiveSounds(CUtlVector< SndInfo_t >& sndlist) = 0;

	virtual void	PrecacheSentenceGroup(const char *pGroupName) = 0;
	virtual void	NotifyBeginMoviePlayback() = 0;
	virtual void	NotifyEndMoviePlayback() = 0;

	virtual bool	GetSoundChannelVolume(const char* sound, float &flVolumeLeft, float &flVolumeRight) = 0;
	virtual float	GetElapsedTimeByGuid(int guid) = 0;
};
