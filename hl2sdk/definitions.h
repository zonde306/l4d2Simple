#pragma once

typedef void* (*CreateInterfaceFn)(const char *Name, int *ReturnCode);
#define _AssertMsg( _exp, _msg, _executeExp, _bFatal ) do { __analysis_assume( !!(_exp) ); _msg; } while (0)
#define _AssertMsgOnce( _exp, _msg, _bFatal ) do { __analysis_assume( !!(_exp) ); _msg; } while (0)
#define DBGFLAG_ASSERT
#define DBGFLAG_ASSERTFATAL
#define DBGFLAG_ASSERTDEBUG

#define  Assert( _exp )										((void)0)
#define  AssertOnce( _exp )									((void)0)
#define  AssertMsg( _exp, _msg, ... )						((void)0)
#define  AssertMsgOnce( _exp, _msg )						((void)0)
#define  AssertFunc( _exp, _f )								((void)0)
#define  AssertEquals( _exp, _expectedValue )				((void)0)
#define  AssertFloatEquals( _exp, _expectedValue, _tol )	((void)0)
#define  Verify( _exp )										(_exp)
#define	 VerifyMsg1( _exp, _msg, a1 )						(_exp)
#define	 VerifyMsg2( _exp, _msg, a1, a2 )					(_exp)
#define	 VerifyMsg3( _exp, _msg, a1, a2, a3 )				(_exp)
#define  VerifyEquals( _exp, _expectedValue )           	(_exp)
#define  DbgVerify( _exp )									(_exp)

#define  AssertMsg1( _exp, _msg, a1 )									((void)0)
#define  AssertMsg2( _exp, _msg, a1, a2 )								((void)0)
#define  AssertMsg3( _exp, _msg, a1, a2, a3 )							((void)0)
#define  AssertMsg4( _exp, _msg, a1, a2, a3, a4 )						((void)0)
#define  AssertMsg5( _exp, _msg, a1, a2, a3, a4, a5 )					((void)0)
#define  AssertMsg6( _exp, _msg, a1, a2, a3, a4, a5, a6 )				((void)0)
#define  AssertMsg6( _exp, _msg, a1, a2, a3, a4, a5, a6 )				((void)0)
#define  AssertMsg7( _exp, _msg, a1, a2, a3, a4, a5, a6, a7 )			((void)0)
#define  AssertMsg8( _exp, _msg, a1, a2, a3, a4, a5, a6, a7, a8 )		((void)0)
#define  AssertMsg9( _exp, _msg, a1, a2, a3, a4, a5, a6, a7, a8, a9 )	((void)0)

inline void AssertValidReadPtr(const void* ptr, int count = 1) { }
inline void AssertValidWritePtr(const void* ptr, int count = 1) { }
inline void AssertValidReadWritePtr(const void* ptr, int count = 1) { }
#define AssertValidStringPtr AssertValidReadPtr
#define AssertValidThis() AssertValidReadWritePtr(this,sizeof(*this))
#define GET_MEMBER_OFFSET(c,m)		(DWORD)(&((c*)0)->m)

#define COLORCODE(r, g, b, a) ((DWORD)((((r)&0xff) << 24) | (((g)&0xff) << 16) | (((b)&0xff) << 8) | ((a)&0xff)))
#define RED(COLORCODE) ((int)(COLORCODE >> 24))
#define BLUE(COLORCODE) ((int)(COLORCODE >> 8) & 0xFF)
#define GREEN(COLORCODE) ((int)(COLORCODE >> 16) & 0xFF)
#define ALPHA(COLORCODE) ((int)COLORCODE & 0xFF)

// NOTE: This macro is the same as windows uses; so don't change the guts of it
#define DECLARE_HANDLE_16BIT(name)	typedef CIntHandle16< struct name##__handle * > name;
#define DECLARE_HANDLE_32BIT(name)	typedef CIntHandle32< struct name##__handle * > name;

#define DECLARE_POINTER_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#define FORWARD_DECLARE_HANDLE(name) typedef struct name##__ *name

#define TIME_TO_TICKS(dt)		((int)(0.5f + (float)(dt) / g_pInterface->GlobalVars->interval_per_tick))
#define TICKS_TO_TIME(t)		(g_pInterface->GlobalVars->interval_per_tick * t)
#define FORWARD_TRACK			(TIME_TO_TICKS(g_pNetChannelInfo->GetLatency(NetFlow_Incoming) + g_pNetChannelInfo->GetLatency(NetFlow_Outgoing)))
#define MAX_PLAYER_NAME_LENGTH		32

// a client can have up to 4 customization files (logo, sounds, models, txt).
#define MAX_CUSTOM_FILES		4		// max 4 files
#define MAX_CUSTOM_FILE_SIZE	524288	// Half a megabyte

// These defines are for client button presses
// CUserCommand::buttons
#define IN_ATTACK					(1 << 0)
#define IN_JUMP						(1 << 1)
#define IN_DUCK						(1 << 2)
#define IN_FORWARD					(1 << 3)
#define IN_BACK						(1 << 4)
#define IN_USE						(1 << 5)
#define IN_CANCEL					(1 << 6)
#define IN_LEFT						(1 << 7)
#define IN_RIGHT					(1 << 8)
#define IN_MOVELEFT					(1 << 9)
#define IN_MOVERIGHT				(1 << 10)
#define IN_ATTACK2					(1 << 11)
#define IN_RUN						(1 << 12)
#define IN_RELOAD					(1 << 13)
#define IN_ALT1						(1 << 14)
#define IN_ALT2						(1 << 15)
#define IN_SCORE					(1 << 16)   /**< Used by client.dll for when scoreboard is held down */
#define IN_SPEED					(1 << 17)	/**< Player is holding the speed key */
#define IN_WALK						(1 << 18)	/**< Player holding walk key */
#define IN_ZOOM						(1 << 19)	/**< Zoom key for HUD zoom */
#define IN_WEAPON1					(1 << 20)	/**< weapon defines these bits */
#define IN_WEAPON2					(1 << 21)	/**< weapon defines these bits */
#define IN_BULLRUSH					(1 << 22)
#define IN_GRENADE1					(1 << 23)	/**< grenade 1 */
#define IN_GRENADE2					(1 << 24)	/**< grenade 2 */
#define IN_ATTACK3					(1 << 25)

// Note: these are only for use with GetEntityFlags and SetEntityFlags
//       and may not match the game's actual, internal m_fFlags values.
// PLAYER SPECIFIC FLAGS FIRST BECAUSE WE USE ONLY A FEW BITS OF NETWORK PRECISION
#define	FL_ONGROUND					(1 << 0)	/**< At rest / on the ground */
#define FL_DUCKING					(1 << 1)	/**< Player flag -- Player is fully crouched */
#define	FL_WATERJUMP				(1 << 2)	/**< player jumping out of water */
#define FL_ONTRAIN					(1 << 3)	/**< Player is _controlling_ a train, so movement commands should be ignored on client during prediction. */
#define FL_INRAIN					(1 << 4)	/**< Indicates the entity is standing in rain */
#define FL_FROZEN					(1 << 5)	/**< Player is frozen for 3rd person camera */
#define FL_ATCONTROLS				(1 << 6)	/**< Player can't move, but keeps key inputs for controlling another entity */
#define	FL_CLIENT					(1 << 7)	/**< Is a player */
#define FL_FAKECLIENT				(1 << 8)	/**< Fake client, simulated server side; don't send network messages to them */
// NOTE if you move things up, make sure to change this value
#define PLAYER_FLAG_BITS		9
// NON-PLAYER SPECIFIC (i.e., not used by GameMovement or the client .dll ) -- Can still be applied to players, though
#define	FL_INWATER					(1 << 9)	/**< In water */
#define	FL_FLY						(1 << 10)	/**< Changes the SV_Movestep() behavior to not need to be on ground */
#define	FL_SWIM						(1 << 11)	/**< Changes the SV_Movestep() behavior to not need to be on ground (but stay in water) */
#define	FL_CONVEYOR					(1 << 12)
#define	FL_NPC						(1 << 13)
#define	FL_GODMODE					(1 << 14)
#define	FL_NOTARGET					(1 << 15)
#define	FL_AIMTARGET				(1 << 16)	/**< set if the crosshair needs to aim onto the entity */
#define	FL_PARTIALGROUND			(1 << 17)	/**< not all corners are valid */
#define FL_STATICPROP				(1 << 18)	/**< Eetsa static prop!		 */
#define FL_GRAPHED					(1 << 19)	/**< worldgraph has this ent listed as something that blocks a connection */
#define FL_GRENADE					(1 << 20)
#define FL_STEPMOVEMENT				(1 << 21)	/**< Changes the SV_Movestep() behavior to not do any processing */
#define FL_DONTTOUCH				(1 << 22)	/**< Doesn't generate touch functions, generates Untouch() for anything it was touching when this flag was set */
#define FL_BASEVELOCITY				(1 << 23)	/**< Base velocity has been applied this frame (used to convert base velocity into momentum) */
#define FL_WORLDBRUSH				(1 << 24)	/**< Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something) */
#define FL_OBJECT					(1 << 25)	/**< Terrible name. This is an object that NPCs should see. Missiles, for example. */
#define FL_KILLME					(1 << 26)	/**< This entity is marked for death -- will be freed by game DLL */
#define FL_ONFIRE					(1 << 27)	/**< You know... */
#define FL_DISSOLVING				(1 << 28)	/**< We're dissolving! */
#define FL_TRANSRAGDOLL				(1 << 29)	/**< In the process of turning into a client side ragdoll. */
#define FL_UNBLOCKABLE_BY_PLAYER	(1 << 30)	/**< pusher that can't be blocked by the player */
#define FL_FREEZING					(1 << 31)	/**< We're becoming frozen! */

// CBaseEntity::m_fEffects
#define EF_BONEMERGE				(1 << 0)	// Merges bones of names shared with a parent entity to the position and direction of the parent's.
#define EF_BRIGHTLIGHT				(1 << 1)	// Emits a dynamic light of RGB(250,250,250) and a random radius of 400 to 431 from the origin.
#define EF_DIMLIGHT					(1 << 2)	// Emits a dynamic light of RGB(100,100,100) and a random radius of 200 to 231 from the origin.
#define EF_NOINTERP					(1 << 3)	// Don't interpolate on the next frame.
#define EF_NOSHADOW					(1 << 4)	// Don't cast a shadow. To do: Does this also apply to shadow maps?
#define EF_NODRAW					(1 << 5)	// Entity is completely ignored by the client. Can cause prediction errors if a player proceeds to collide with it on the server.
#define EF_NORECEIVESHADOW			(1 << 6)	// Don't receive dynamic shadows.
#define EF_BONEMERGE_FASTCULL		(1 << 7)	// For use with EF_BONEMERGE. If set, the entity will use its parent's origin to calculate whether it is visible; if not set, it will set up parent's bones every frame even if the parent is not in the PVS.
#define EF_ITEM_BLINK				(1 << 8)	// Blink an item so that the user notices it. Added for Xbox 1, and really not very subtle.
#define EF_PARENT_ANIMATES			(1 << 9)	// Assume that the parent entity is always animating. Causes it to realign every frame.

// Unknown
#define DISPSURF_FLAG_SURFACE		(1 << 0)
#define DISPSURF_FLAG_WALKABLE		(1 << 1)
#define DISPSURF_FLAG_BUILDABLE		(1 << 2)
#define DISPSURF_FLAG_SURFPROP1		(1 << 3)
#define DISPSURF_FLAG_SURFPROP2		(1 << 4)

// TraceRay Mask
#define	CONTENTS_EMPTY			0		/**< No contents. */
#define	CONTENTS_SOLID			0x1		/**< an eye is never valid in a solid . */
#define	CONTENTS_WINDOW			0x2		/**< translucent, but not watery (glass). */
#define	CONTENTS_AUX			0x4
#define	CONTENTS_GRATE			0x8		/**< alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't. */
#define	CONTENTS_SLIME			0x10
#define	CONTENTS_WATER			0x20
#define	CONTENTS_MIST			0x40
#define CONTENTS_OPAQUE			0x80		/**< things that cannot be seen through (may be non-solid though). */
#define	LAST_VISIBLE_CONTENTS	0x80
#define ALL_VISIBLE_CONTENTS 	(LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS - 1))
#define CONTENTS_TESTFOGVOLUME	0x100
#define CONTENTS_UNUSED5		0x200
#define CONTENTS_UNUSED6		0x4000
#define CONTENTS_TEAM1			0x800		/**< per team contents used to differentiate collisions. */
#define CONTENTS_TEAM2			0x1000		/**< between players and objects on different teams. */
#define CONTENTS_IGNORE_NODRAW_OPAQUE	0x2000		/**< ignore CONTENTS_OPAQUE on surfaces that have SURF_NODRAW. */
#define CONTENTS_MOVEABLE		0x4000		/**< hits entities which are MOVETYPE_PUSH (doors, plats, etc) */
#define	CONTENTS_AREAPORTAL		0x8000		/**< remaining contents are non-visible, and don't eat brushes. */
#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

/**
* @section currents can be added to any other contents, and may be mixed
*/
#define	CONTENTS_CURRENT_0		0x40000
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000

/**
* @endsection
*/

#define	CONTENTS_ORIGIN			0x1000000	/**< removed before bsp-ing an entity. */
#define	CONTENTS_MONSTER		0x2000000	/**< should never be on a brush, only in game. */
#define	CONTENTS_DEBRIS			0x4000000
#define	CONTENTS_DETAIL			0x8000000	/**< brushes to be added after vis leafs. */
#define	CONTENTS_TRANSLUCENT	0x10000000	/**< auto set if any surface has trans. */
#define	CONTENTS_LADDER			0x20000000
#define CONTENTS_HITBOX			0x40000000	/**< use accurate hitboxes on trace. */

/**
* @section Trace masks.
*/
#define	MASK_ALL				(0xFFFFFFFF)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 			/**< everything that is normally solid */
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 	/**< everything that blocks player movement */
#define	MASK_NPCSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) /**< blocks npc movement */
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME) 							/**< water physics in these contents */
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE) 							/**< everything that blocks line of sight for AI, lighting, etc */
#define MASK_OPAQUE_AND_NPCS	(MASK_OPAQUE|CONTENTS_MONSTER)										/**< everything that blocks line of sight for AI, lighting, etc, but with monsters added. */
#define	MASK_VISIBLE			(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE) 								/**< everything that blocks line of sight for players */
#define MASK_VISIBLE_AND_NPCS	(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE) 							/**< everything that blocks line of sight for players, but with monsters added. */
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX) 	/**< bullets see these as solid */
#define MASK_SHOT_HULL			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE) 	/**< non-raycasted weapons see this as solid (includes grates) */
#define MASK_SHOT_PORTAL		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW) 							/**< hits solids (not grates) and passes through everything else */
#define MASK_SOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE) 					/**< everything normally solid, except monsters (world+brush only) */
#define MASK_PLAYERSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for player movement, except monsters (world+brush only) */
#define MASK_NPCSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for npc movement, except monsters (world+brush only) */
#define MASK_NPCWORLDSTATIC		(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 					/**< just the world, used for route rebuilding */
#define MASK_SPLITAREAPORTAL	(CONTENTS_WATER|CONTENTS_SLIME) 									/**< These are things that can split areaportals */

// CBaseEntity::movetype
enum MoveType_t
{
	MOVETYPE_NONE = 0,			/**< never moves */
	MOVETYPE_ISOMETRIC,			/**< For players */
	MOVETYPE_WALK,				/**< Player only - moving on the ground */
	MOVETYPE_STEP,				/**< gravity, special edge handling -- monsters use this */
	MOVETYPE_FLY,				/**< No gravity, but still collides with stuff */
	MOVETYPE_FLYGRAVITY,		/**< flies through the air + is affected by gravity */
	MOVETYPE_VPHYSICS,			/**< uses VPHYSICS for simulation */
	MOVETYPE_PUSH,				/**< no clip to world, push and crush */
	MOVETYPE_NOCLIP,			/**< No gravity, no collisions, still do velocity/avelocity */
	MOVETYPE_LADDER,			/**< Used by players only when going onto a ladder */
	MOVETYPE_OBSERVER,			/**< Observer movement, depends on player's observer mode */
	MOVETYPE_CUSTOM				/**< Allows the entity to describe its own physics */
};

// CBaseEntity::m_nRenderMode
enum RenderMode
{
	RENDER_NORMAL,				/**< src */
	RENDER_TRANSCOLOR,			/**< c*a+dest*(1-a) */
	RENDER_TRANSTEXTURE,		/**< src*a+dest*(1-a) */
	RENDER_GLOW,				/**< src*a+dest -- No Z buffer checks -- Fixed size in screen space */
	RENDER_TRANSALPHA,			/**< src*srca+dest*(1-srca) */
	RENDER_TRANSADD,			/**< src*a+dest */
	RENDER_ENVIRONMENTAL,		/**< not drawn, used for environmental effects */
	RENDER_TRANSADDFRAMEBLEND,	/**< use a fractional frame value to blend between animation frames */
	RENDER_TRANSALPHAADD,		/**< src + dest*(1-a) */
	RENDER_WORLDGLOW,			/**< Same as kRenderGlow but not fixed size in screen space */
	RENDER_NONE					/**< Don't render. */
};

// CBaseEntity::m_nRenderFX
enum RenderFx
{
	RENDERFX_NONE = 0,
	RENDERFX_PULSE_SLOW,
	RENDERFX_PULSE_FAST,
	RENDERFX_PULSE_SLOW_WIDE,
	RENDERFX_PULSE_FAST_WIDE,
	RENDERFX_FADE_SLOW,
	RENDERFX_FADE_FAST,
	RENDERFX_SOLID_SLOW,
	RENDERFX_SOLID_FAST,
	RENDERFX_STROBE_SLOW,
	RENDERFX_STROBE_FAST,
	RENDERFX_STROBE_FASTER,
	RENDERFX_FLICKER_SLOW,
	RENDERFX_FLICKER_FAST,
	RENDERFX_NO_DISSIPATION,
	RENDERFX_DISTORT,			/**< Distort/scale/translate flicker */
	RENDERFX_HOLOGRAM,			/**< kRenderFxDistort + distance fade */
	RENDERFX_EXPLODE,			/**< Scale up really big! */
	RENDERFX_GLOWSHELL,			/**< Glowing Shell */
	RENDERFX_CLAMP_MIN_SCALE,	/**< Keep this sprite from getting very small (SPRITES only!) */
	RENDERFX_ENV_RAIN,			/**< for environmental rendermode, make rain */
	RENDERFX_ENV_SNOW,			/**<  "        "            "    , make snow */
	RENDERFX_SPOTLIGHT,			/**< TEST CODE for experimental spotlight */
	RENDERFX_RAGDOLL,			/**< HACKHACK: TEST CODE for signalling death of a ragdoll character */
	RENDERFX_PULSE_FAST_WIDER,
	RENDERFX_MAX
};

// CBaseEntity::m_CollisionGroup
enum CollisionGroup_t
{
	CG_NONE = 0,
	CG_DEBRIS,					// Collides with nothing but world and static stuff
	CG_DEBRIS_TRIGGER,			// Same as debris, but hits triggers
	CG_INTERACTIVE_DEBRIS,		// Collides with everything except other interactive debris or debris
	CG_INTERACTIVE,				// Collides with everything except interactive debris or debris
	CG_PLAYER,
	CG_BREAKABLE_GLASS,
	CG_VEHICLE,
	CG_PLAYER_MOVEMENT,			// For HL2, same as CG_Player  
	CG_NPC,						// Generic NPC group
	CG_IN_VEHICLE,				// for any entity inside a vehicle
	CG_WEAPON,					// for any weapons that need collision detection
	CG_VEHICLE_CLIP,			// vehicle clip brush to restrict vehicle movement
	CG_PROJECTILE,				// Projectiles!
	CG_DOOR_BLOCKER,			// Blocks entities not permitted to get near moving doors
	CG_PASSABLE_DOOR,			// Doors that the player shouldn't collide with
	CG_DISSOLVING,				// Things that are dissolving are in this group
	CG_PUSHAWAY,				// Nonsolid on client and server, pushaway in player code
	CG_NPC_ACTOR,				// Used so NPCs in scripts ignore the player.  
	CG_NPC_SCRIPTED				// USed for NPCs in scripts that should not collide with each other
};

// CBaseEntity::m_Collision::m_nSolidType
enum SolidType_t
{
	SOLID_NONE = 0,				// no solid model
	SOLID_BSP,					// a BSP tree
	SOLID_BBOX,					// an AABB
	SOLID_OBB,					// an OBB (not implemented yet)
	SOLID_OBB_YAW,				// an OBB, constrained so that it can only yaw
	SOLID_CUSTOM,				// Always call into the entity for tests
	SOLID_VPHYSICS				// solid vphysics object, get vcollide from the model and collide with that
};

// CBaseEntity::m_Collision::m_usSolidFlags
enum SolidFlags_t
{
	SF_CUSTOMRAYTEST = 0x0001,			// Ignore solid type + always call into the entity for ray tests
	SF_CUSTOMBOXTEST = 0x0002,			// Ignore solid type + always call into the entity for swept box tests
	SF_NOT_SOLID = 0x0004,				// Are we currently not solid?
	SF_TRIGGER = 0x0008,				// This is something may be collideable but fires touch functions
	
	// even when it's not collideable (when the SF_NOT_SOLID flag is set)
	SF_NOT_STANDABLE = 0x0010,			// You can't stand on this
	SF_VOLUME_CONTENTS = 0x0020,		// Contains volumetric contents (like water)
	SF_FORCE_WORLD_ALIGNED = 0x0040,	// Forces the collision rep to be world-aligned even if it's SOLID_BSP or SOLID_VPHYSICS
	SF_USE_TRIGGER_BOUNDS = 0x0080,		// Uses a special trigger bounds separate from the normal OBB
	SF_ROOT_PARENT_ALIGNED = 0x0100,	// Collisions are defined in root parent's local coordinate space
	SF_TRIGGER_TOUCH_DEBRIS = 0x0200	// This trigger will touch debris objects
};

// entity flags, CBaseEntity::m_iEFlags
enum
{
	EFL_KILLME = (1 << 0),	// This entity is marked for death -- This allows the game to actually delete ents at a safe time
	EFL_DORMANT = (1 << 1),	// Entity is dormant, no updates to client
	EFL_NOCLIP_ACTIVE = (1 << 2),	// Lets us know when the noclip command is active.
	EFL_SETTING_UP_BONES = (1 << 3),	// Set while a model is setting up its bones.
	EFL_KEEP_ON_RECREATE_ENTITIES = (1 << 4), // This is a special entity that should not be deleted when we restart entities only

	EFL_HAS_PLAYER_CHILD = (1 << 4),	// One of the child entities is a player.

	EFL_DIRTY_SHADOWUPDATE = (1 << 5),	// Client only- need shadow manager to update the shadow...
	EFL_NOTIFY = (1 << 6),	// Another entity is watching events on this entity (used by teleport)

	// The default behavior in ShouldTransmit is to not send an entity if it doesn't
	// have a model. Certain entities want to be sent anyway because all the drawing logic
	// is in the client DLL. They can set this flag and the engine will transmit them even
	// if they don't have a model.
	EFL_FORCE_CHECK_TRANSMIT = (1 << 7),

	EFL_BOT_FROZEN = (1 << 8),	// This is set on bots that are frozen.
	EFL_SERVER_ONLY = (1 << 9),	// Non-networked entity.
	EFL_NO_AUTO_EDICT_ATTACH = (1 << 10), // Don't attach the edict; we're doing it explicitly

	// Some dirty bits with respect to abs computations
	EFL_DIRTY_ABSTRANSFORM = (1 << 11),
	EFL_DIRTY_ABSVELOCITY = (1 << 12),
	EFL_DIRTY_ABSANGVELOCITY = (1 << 13),
	EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS = (1 << 14),
	EFL_DIRTY_SPATIAL_PARTITION = (1 << 15),
	//	UNUSED						= (1<<16),

	EFL_IN_SKYBOX = (1 << 17),	// This is set if the entity detects that it's in the skybox.
	// This forces it to pass the "in PVS" for transmission.
	EFL_USE_PARTITION_WHEN_NOT_SOLID = (1 << 18),	// Entities with this flag set show up in the partition even when not solid
	EFL_TOUCHING_FLUID = (1 << 19),	// Used to determine if an entity is floating

	// FIXME: Not really sure where I should add this...
	EFL_IS_BEING_LIFTED_BY_BARNACLE = (1 << 20),
	EFL_NO_ROTORWASH_PUSH = (1 << 21),		// I shouldn't be pushed by the rotorwash
	EFL_NO_THINK_FUNCTION = (1 << 22),
	EFL_NO_GAME_PHYSICS_SIMULATION = (1 << 23),

	EFL_CHECK_UNTOUCH = (1 << 24),
	EFL_DONTBLOCKLOS = (1 << 25),		// I shouldn't block NPC line-of-sight
	EFL_DONTWALKON = (1 << 26),		// NPC;s should not walk on this entity
	EFL_NO_DISSOLVE = (1 << 27),		// These guys shouldn't dissolve
	EFL_NO_MEGAPHYSCANNON_RAGDOLL = (1 << 28),	// Mega physcannon can't ragdoll these guys.
	EFL_NO_WATER_VELOCITY_CHANGE = (1 << 29),	// Don't adjust this entity's velocity when transitioning into water
	EFL_NO_PHYSCANNON_INTERACTION = (1 << 30),	// Physcannon can't pick these up or punt them
	EFL_NO_DAMAGE_FORCES = (1 << 31)	// Doesn't accept forces from physics damage
};

// Hud Element hiding flags, CBaseEntity::m_Local::m_iHideHUD
#define	HIDEHUD_WEAPONSELECTION		( 1<<0 )	// Hide ammo count & weapon selection
#define	HIDEHUD_FLASHLIGHT			( 1<<1 )
#define	HIDEHUD_ALL					( 1<<2 )
#define HIDEHUD_HEALTH				( 1<<3 )	// Hide health & armor / suit battery
#define HIDEHUD_PLAYERDEAD			( 1<<4 )	// Hide when local player's dead
#define HIDEHUD_NEEDSUIT			( 1<<5 )	// Hide when the local player doesn't have the HEV suit
#define HIDEHUD_MISCSTATUS			( 1<<6 )	// Hide miscellaneous status elements (trains, pickup history, death notices, etc)
#define HIDEHUD_CHAT				( 1<<7 )	// Hide all communication elements (saytext, voice icon, etc)
#define	HIDEHUD_CROSSHAIR			( 1<<8 )	// Hide crosshairs
#define	HIDEHUD_VEHICLE_CROSSHAIR	( 1<<9 )	// Hide vehicle crosshair
#define HIDEHUD_INVEHICLE			( 1<<10 )
#define HIDEHUD_BONUS_PROGRESS		( 1<<11 )	// Hide bonus progress display (for bonus map challenges)

// ---------------------------
//  Hit Group standards
// ---------------------------
#define	HITGROUP_GENERIC	0
#define	HITGROUP_HEAD		1
#define	HITGROUP_CHEST		2
#define	HITGROUP_STOMACH	3
#define HITGROUP_LEFTARM	4	
#define HITGROUP_RIGHTARM	5
#define HITGROUP_LEFTLEG	6
#define HITGROUP_RIGHTLEG	7
#define HITGROUP_GEAR		10			// alerts NPC, but doesn't do damage or bleed (1/100th damage)

// settings for CBaseEntity::m_takedamage
#define	DAMAGE_NO				0
#define DAMAGE_EVENTS_ONLY		1		// Call damage functions, but don't modify health
#define	DAMAGE_YES				2
#define	DAMAGE_AIM				3

// Spectator Movement modes CBaseEntity::m_iObserverMode
enum
{
	OBS_MODE_NONE = 0,	// not in spectator mode
	OBS_MODE_DEATHCAM,	// special mode for death cam animation
	OBS_MODE_FREEZECAM,	// zooms to a target, and freeze-frames on them
	OBS_MODE_FIXED,		// view from a fixed camera position
	OBS_MODE_IN_EYE,	// follow a player in first person view
	OBS_MODE_CHASE,		// follow a player in third person view
	OBS_MODE_POI,		// PASSTIME point of interest - game objective, big fight, anything interesting; added in the middle of the enum due to tons of hard-coded "<ROAMING" enum compares
	OBS_MODE_ROAMING,	// free roaming

	NUM_OBSERVER_MODES
};

// CBaseEntity::m_lifeState
enum LifeStates_t
{
	LIFE_ALIVE = 0,
	LIFE_DYING,
	LIFE_DEAD,
	LIFE_RESPAWNABLE,
	LIFE_DISCARDBODY
};

// CBaseCombatWeapon::m_iPrimaryAmmoType / CBaseCombatWeapon::m_iSecondaryAmmoType
enum AmmoType_t
{
	AT_Pistol = 1,					// 小手枪弹药
	AT_Magnum,						// 马格南手枪弹药
	AT_Rifle,						// 步枪弹药
	AT_Minigun,						// 一代固定机枪（开枪有延迟那个）
	AT_Smg,							// 冲锋枪弹药
	AT_M60,							// 机枪弹药
	AT_Shotgun,						// 单喷弹药
	AT_AutoShotgun,					// 连喷弹药
	AT_Hunting,						// 猎枪（15发子弹）
	AT_Sniper,						// 狙击枪弹药
	AT_Turret,						// 二代固定机枪（射速慢点但无延迟）
	AT_PipeBomb,					// 土制炸弹
	AT_Molotov,						// 燃烧瓶
	AT_Vomitjar,					// 胆汁罐
	AT_PainPills,					// 止痛药
	AT_FirstAidKit,					// 医疗包
	AT_Grenade,						// 榴弹发射器的榴弹
	AT_Adrenline,					// 肾上腺素
	AT_Chainsaw						// 电锯
};

// CWeaponSpawn::m_weaponID
enum WeaponID_t
{
	Weapon_Pistol = 1,				// 小手枪(包括双持) 手枪
	Weapon_ShotgunPump = 3,			// 木喷 半自动
	Weapon_ShotgunAuto = 4,			// 连喷 连点加快射速
	Weapon_SniperHunting = 6,		// 猎枪 半自动
	Weapon_ShotgunChrome = 8,		// 铁喷 半自动
	Weapon_SniperMilitary = 10,		// 连狙 半自动
	Weapon_ShotgunSpas = 11,		// 高级连喷 连点加快射速
	Weapon_PistolMagnum = 32,		// 马格南 手枪
	Weapon_SniperAWP = 35,			// 大鸟 半自动
	Weapon_SniperScout = 36,		// 鸟狙 半自动

	Weapon_Melee = 19,				// 近战武器
	Weapon_GrenadeLauncher = 21,	// 榴弹发射器
	Weapon_Chainsaw = 20,			// 电锯

	Weapon_FirstAidKit = 12,		// 医疗包
	Weapon_PainPills = 15,			// 止痛药
	Weapon_Defibrillator = 24,		// 电击器
	Weapon_Adrenaline = 23,			// 肾上腺素
	Weapon_Molotov = 13,			// 燃烧瓶
	Weapon_PipeBomb = 14,			// 土制炸弹
	Weapon_Vomitjar = 25,			// 胆汁罐
	Weapon_FireAmmo = 30,			// 燃烧子弹盒
	Weapon_ExplodeAmmo = 31,		// 爆炸子弹盒
	Weapon_Gascan = 16,				// 油桶（红色和黄色）
	Weapon_Fireworkcrate = 29,		// 烟花盒
	Weapon_Propanetank = 17,		// 煤气罐
	Weapon_Oxygentank = 18,			// 氧气瓶
	Weapon_Gnome = 27,				// 侏儒
	Weapon_Cola = 28,				// 可乐

	WeaponId_WeaponCSBase = 0,
	WeaponId_AssaultRifle = 5,
	WeaponId_AutoShotgun = 4,
	WeaponId_BaseBackpackItem = 0,
	WeaponId_BoomerClaw = 41,
	WeaponId_CarriedProp = 16,
	WeaponId_Chainsaw = 20,
	WeaponId_ChargerClaw = 40,
	WeaponId_ColaBottles = 28,
	WeaponId_FireworkCrate = 29,
	WeaponId_FirstAidKit = 12,
	WeaponId_GasCan = 16,
	WeaponId_Gnome = 27,
	WeaponId_GrenadeLauncher = 21,
	WeaponId_HunterClaw = 39,
	WeaponId_Adrenaline = 23,
	WeaponId_ItemAmmoPack = 22,
	WeaponId_ItemDefibrillator = 24,
	WeaponId_ItemUpgradePackExplosive = 31,
	WeaponId_ItemUpgradePackIncendiary = 30,
	WeaponId_VomitJar = 25,
	WeaponId_JockeyClaw = 44,
	WeaponId_Molotov = 13,
	WeaponId_OxygenTank = 18,
	WeaponId_PainPills = 15,
	WeaponId_PipeBomb = 14,
	WeaponId_Pistol = 1,
	WeaponId_MagnumPistol = 32,
	WeaponId_PropaneTank = 17,
	WeaponId_PumpShotgun = 3,
	WeaponId_AK47 = 26,
	WeaponId_Desert = 9,
	WeaponId_M60 = 37,
	WeaponId_SG552 = 34,
	WeaponId_Chrome = 8,
	WeaponId_SPAS = 11,
	WeaponId_MP5 = 33,
	WeaponId_Silenced = 7,
	WeaponId_SmokerClaw = 42,
	WeaponId_SniperRifle = 6,
	WeaponId_AWP = 35,
	WeaponId_Military = 10,
	WeaponId_Scout = 36,
	WeaponId_SpitterClaw = 43,
	WeaponId_SubMachinegun = 2,
	WeaponId_TankClaw = 38,
	WeaponId_TerrorMeleeWeapon = 19,
	WeaponId_WeaponSpawn = 8
};

// ClientClass::m_m_ClassID
enum EntityType_t
{
	ET_INVALID = -1,
	ET_WORLD = 261,
	ET_TerrorGameRulesProxy = 229,
	ET_TerrorPlayerResource = 233,
	ET_PlayerResource = 133,
	ET_CSGameRulesProxy = 47,
	ET_GameRulesProxy = 93,
	ET_FogController = 75,

	// 生还者
	ET_CTERRORPLAYER = 232,
	ET_SURVIVORBOT = 275,

	// 普感
	ET_INFECTED = 264,
	ET_WITCH = 277,

	// 特感
	ET_BOOMER = 0,
	ET_TANK = 276,
	ET_JOCKEY = 265,
	ET_SPITTER = 272,
	ET_CHARGER = 99,
	ET_HUNTER = 263,
	ET_SMOKER = 270,

	// 飞行物
	ET_TankRock = 13,				// 克的石头(CBaseCSGrenadeProjectile)
	ET_VomitJarProjectile = 252,	// 胆汁
	ET_SpitterProjectile = 176,		// 口水的酸液球
	ET_PipeBombProjectile = 130,	// 土雷
	ET_MolotovProjectile = 119,		// 火瓶
	ET_GrenadeProjectile = 97,		// 榴弹发射器的榴弹

	// 杂物
	ET_DoorCheckpoint = 143,		// 安全门
	ET_SurvivorRescue = 186,		// 复活点
	ET_SurvivorDeathModel = 184,	// 死亡的生还者
	ET_PropHealthCabinet = 144,		// 医疗箱
	ET_CTongue = 246,
	ET_Particle = 124,

	// 武器 - 其他
	ET_WeaponMountedGun = 146,
	ET_WeaponMinigun = 145,
	ET_WeaponAmmoSpawn = 256,
	ET_WeaponSpawn = 260,
	ET_ScavengeItemSpawn = 259,
	ET_BaseUpgradeItem = 29,

	// 武器 - 冲锋枪
	ET_WeaponMP5 = 165,
	ET_WeaponSilenced = 166,

	// 武器 - 霰弹枪
	ET_WeaponAuto = 2,
	ET_WeaponSpas = 163,
	ET_WeaponChrome = 162,
	ET_WeaponPump = 148,

	// 武器 - 步枪
	ET_WeaponAK47 = 152,
	ET_WeaponDesert = 153,
	ET_WeaponSG552 = 155,
	ET_WeaponRifle = 1,
	ET_WeaponM60 = 154,
	ET_WeaponGrenadeLauncher = 96,

	// 武器 - 狙击枪
	ET_WeaponScout = 171,
	ET_WeaponMilitary = 170,
	ET_WeaponAWP = 169,

	// 武器 - 手枪
	ET_WeaponMagnum = 116,
	ET_WeaponPistol = 131,

	// 武器 - 近战武器
	ET_WeaponChainsaw = 39,
	ET_WeaponMelee = 231,

	// 武器 - 投掷武器
	ET_WeaponPipeBomb = 129,
	ET_WeaponMolotov = 118,
	ET_WeaponVomitjar = 106,
	ET_ProjectilePipeBomb = 130,
	ET_ProjectileMolotov = 119,
	ET_ProjectileVomitJar = 252,
	ET_ProjectileSpitter = 176,
	ET_ProjectileGrenadeLauncher = 97,
	ET_ProjectileGrenade = 13,

	// 武器 - 医疗品/升级包
	ET_WeaponIncendiary = 111,
	ET_WeaponExplosive = 110,
	ET_WeaponDefibrillator = 109,
	ET_WeaponFirstAidKit = 73,
	ET_WeaponAmmoPack = 107,

	// 武器 - 药物
	ET_WeaponPainPills = 121,
	ET_WeaponAdrenaline = 105,

	// 武器 - 携带物品
	ET_WeaponOxygen = 120,
	ET_WeaponGnome = 95,
	ET_WeaponGascan = 94,
	ET_WeaponFirework = 72,
	ET_WeaponCola = 44,
	ET_PhysicsProp = 127,
	ET_WeaponPropaneTank = 142,
};

// CTerrorPlayer::m_zombieClass
enum ZombieClass_t
{
	ZC_COMMON = 0,			// 普感
	ZC_INFECTED = 0,		// 同上
	ZC_SMOKER = 1,			// 舌头
	ZC_BOOMER = 2,			// 胖子
	ZC_HUNTER = 3,			// 猎人
	ZC_SPITTER = 4,			// 口水
	ZC_JOCKEY = 5,			// 猴子
	ZC_CHARGER = 6,			// 牛
	ZC_WITCH = 7,			// 萌妹
	ZC_TANK = 8,			// 坦克
	ZC_SURVIVORBOT = 9,		// 生还者
	ZC_ROCK = 10			// 坦克的石头
};
