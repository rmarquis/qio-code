/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file


#define C_ONLY

#define STANDALONE 1

#define PRODUCT_NAME			"Qio"
#define BASEGAME			"baseqio"
#define CLIENT_WINDOW_TITLE     	"Qio"
#define CLIENT_WINDOW_MIN_TITLE 	"Qio"
#define GAMENAME_FOR_MASTER		"QioTestGame"	// must NOT contain whitespace

// Heartbeat for dpmaster protocol. You shouldn't change this unless you know what you're doing
#define HEARTBEAT_FOR_MASTER		"DarkPlaces"

#ifndef PRODUCT_VERSION
  #define PRODUCT_VERSION "0.1"
#endif

#define Q3_VERSION PRODUCT_NAME " " PRODUCT_VERSION

#define MAX_TEAMNAME		32
#define MAX_MASTER_SERVERS      5	// number of supported master servers

#define DEMOEXT	"dm_"			// standard demo extension

#ifdef _MSC_VER

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)		// benign redefinition
//#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable:  4505) 	// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
//#pragma intrinsic( memset, memcpy )
#endif

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#ifdef __GNUC__
#define UNUSED_VAR __attribute__((unused))
#else
#define UNUSED_VAR
#endif

#if (defined _MSC_VER)
#define Q_EXPORT __declspec(dllexport)
#elif (defined __SUNPRO_C)
#define Q_EXPORT __global
#elif ((__GNUC__ >= 3) && (!__EMX__) && (!sun))
#define Q_EXPORT __attribute__((visibility("default")))
#else
#define Q_EXPORT
#endif

/**********************************************************************
  VM Considerations

  The VM can not use the standard system headers because we aren't really
  using the compiler they were meant for.  We use bg_lib.h which contains
  prototypes for the functions we define for our own use in bg_lib.c.

  When writing mods, please add needed headers HERE, do not start including
  stuff like <stdio.h> in the various .c files that make up each of the VMs
  since you will be including system headers files can will have issues.

  Remember, if you use a C library function that is not defined in bg_lib.c,
  you will have to add your own version for support in the VM.

 **********************************************************************/

#ifdef Q3_VM

#include "../game/bg_lib.h"

typedef int intptr_t;

#else

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#ifdef _MSC_VER
  #include <io.h>

  typedef __int64 int64_t;
  typedef __int32 int32_t;
  typedef __int16 int16_t;
//  typedef __int8 int8_t;
  typedef unsigned __int64 uint64_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int8 uint8_t;

  // vsnprintf is ISO/IEC 9899:1999
  // abstracting this to make it portable
  int Q_vsnprintf(char *str, size_t size, const char *format, va_list ap);
#else
  #include <stdint.h>

  #define Q_vsnprintf vsnprintf
#endif

#endif


#include "q_platform.h"

//=============================================================

#include "../shared/typedefs.h"

typedef int	qboolean;
#define qtrue 1 
#define qfalse 0

typedef union {
	float f;
	int i;
	unsigned int ui;
} floatint_t;

#define PAD(base, alignment)	(((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment)	(PAD((base), (alignment)) - (base))

#define PADP(base, alignment)	((void *) PAD((intptr_t) (base), (alignment)))

#ifdef __GNUC__
#define QALIGN(x) __attribute__((aligned(x)))
#else
#define QALIGN(x)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define STRING(s)			#s
// expand constants before stringifying them
#define XSTRING(s)			STRING(s)

#define ARRAY_LEN(x)			(sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x)			(ARRAY_LEN(x) - 1)

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192


#define	MAX_QPATH			256		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

#define	MAX_NAME_LENGTH		32		// max length of a client name

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility

#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
} errorParm_t;


#if !defined(NDEBUG) && !defined(BSPC)
	#define HUNK_DEBUG
#endif

typedef enum {
	h_high,
	h_low,
	h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#define Hunk_Alloc( size, preference )				Hunk_AllocDebug(size, preference, #size, __FILE__, __LINE__)
void *Hunk_AllocDebug( int size, ha_pref preference, char *label, char *file, int line );
#else
void *Hunk_Alloc( int size, ha_pref preference );
#endif

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

/*
==============================================================

MATHLIB

==============================================================
*/

#include "../math/math.h"

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT/2)

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define BIGCHAR_WIDTH		16
#define BIGCHAR_HEIGHT		16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48


#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

float Q_rsqrt( float f );		// reciprocal square root

#define Square(x) ((x)*(x))

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif

signed char ClampChar( int i );

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

float	AngleMod(float a);
float	LerpAngle (float from, float to, float frac);
float	AngleSubtract( float a1, float a2 );
void	AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );

float AngleNormalize360 ( float angle );
float AngleNormalize180 ( float angle );
float AngleDelta ( float angle1, float angle2 );


//#ifndef MAX
//#define MAX(x,y) ((x)>(y)?(x):(y))
//#endif
//
//#ifndef MIN
//#define MIN(x,y) ((x)<(y)?(x):(y))
//#endif

//=============================================

float Com_Clamp( float min, float max, float value );

char	*COM_SkipPath( char *pathname );
const char	*COM_GetExtension( const char *name );
void	COM_StripExtension(const char *in, char *out, int destsize);
qboolean COM_CompareExtension(const char *in, const char *ext);
void	COM_DefaultExtension( char *path, int maxSize, const char *extension );

void	COM_BeginParseSession( const char *name );
int		COM_GetCurrentParseLine( void );
char	*COM_Parse( char **data_p );
char	*COM_ParseExt( char **data_p, qboolean allowLineBreak );
int		COM_Compress( char *data_p );
void	COM_ParseError( char *format, ... ) __attribute__ ((format (printf, 1, 2)));
void	COM_ParseWarning( char *format, ... ) __attribute__ ((format (printf, 1, 2)));
//int		COM_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] );

void	COM_MatchToken( char**buf_p, char *match );

void SkipBracedSection (char **program);
void SkipRestOfLine ( char **data );

void Parse1DMatrix (char **buf_p, int x, float *m);
void Parse2DMatrix (char **buf_p, int y, int x, float *m);
void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m);
int Com_HexStrToInt( const char *str );

int QDECL Com_sprintf (char *dest, int size, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

char *Com_SkipTokens( char *s, int numTokens, char *sep );
char *Com_SkipCharset( char *s, char *sep );

void Com_RandomBytes( byte *string, int len );

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );
qboolean Q_isanumber( const char *s );
qboolean Q_isintegral( float f );

// portable case insensitive compare
int		Q_stricmp (const char *s1, const char *s2);
int		Q_strncmp (const char *s1, const char *s2, int n);
int		Q_stricmpn (const char *s1, const char *s2, int n);
char	*Q_strlwr( char *s1 );
char	*Q_strupr( char *s1 );
const char	*Q_stristr( const char *s, const char *find);

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );
// Count the number of char tocount encountered in string
int Q_CountChar(const char *string, char tocount);

//=============================================

// 64-bit integers for global rankings interface
// implemented as a struct for qvm compatibility
typedef struct
{
	byte	b0;
	byte	b1;
	byte	b2;
	byte	b3;
	byte	b4;
	byte	b5;
	byte	b6;
	byte	b7;
} qint64;

//=============================================

char	* QDECL va(char *format, ...) __attribute__ ((format (printf, 1, 2)));

#define TRUNCATE_LENGTH	64
void Com_TruncateLongString( char *buffer, const char *s );

//=============================================

//
// key / value info strings
//
char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define	CVAR_ARCHIVE		0x0001	// set to cause it to be saved to vars.rc
					// used for system variables, not for player
					// specific configurations
#define	CVAR_USERINFO		0x0002	// sent to server on connect or change
#define	CVAR_SERVERINFO		0x0004	// sent in response to front end requests
#define	CVAR_SYSTEMINFO		0x0008	// these cvars will be duplicated on all clients
#define	CVAR_INIT		0x0010	// don't allow change from console at all,
					// but can be set from the command line
#define	CVAR_LATCH		0x0020	// will only change when C code next does
					// a Cvar_Get(), so it can't be changed
					// without proper initialization.  modified
					// will be set, even though the value hasn't
					// changed yet
#define	CVAR_ROM		0x0040	// display only, cannot be set by user at all
#define	CVAR_USER_CREATED	0x0080	// created by a set command
#define	CVAR_TEMP		0x0100	// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT		0x0200	// can not be changed if cheats are disabled
#define CVAR_NORESTART		0x0400	// do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED	0x0800	// cvar was created by a server the client connected to.
#define CVAR_VM_CREATED		0x1000	// cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED		0x2000	// prevent modifying this var from VMs or the server
// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED		0x40000000	// Cvar was modified
#define CVAR_NONEXISTENT	0x80000000	// Cvar doesn't exist.

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s cvar_s;

struct cvar_s {
	char			*name;
	char			*string;
	char			*resetString;		// cvar_restart will reset to this value
	char			*latchedString;		// for CVAR_LATCH vars
	int				flags;
	qboolean	modified;			// set each time the cvar is changed
	int				modificationCount;	// incremented each time the cvar is changed
	float			value;				// atof( string )
	int				integer;			// atoi( string )
	qboolean	validate;
	qboolean	integral;
	float			min;
	float			max;

	class cvarModifyCallback_i *modificationCallback;

	cvar_s *next;
	cvar_s *prev;
	cvar_s *hashNext;
	cvar_s *hashPrev;
	int			hashIndex;
};

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_s directly,
// so they must ask for structured updates
typedef struct {
	cvarHandle_t	handle;
	int			modificationCount;
	float		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;


/*
==============================================================

VoIP

==============================================================
*/

// if you change the count of flags be sure to also change VOIP_FLAGNUM
#define VOIP_SPATIAL		0x01		// spatialized voip message
#define VOIP_DIRECT		0x02		// non-spatialized voip message

// number of flags voip knows. You will have to bump protocol version number if you
// change this.
#define VOIP_FLAGCNT		2

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI					0x0002
#define	KEYCATCH_MESSAGE		0x0004
#define	KEYCATCH_CGAME			0x0008

/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT	4	// toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64

#define	GENTITYNUM_BITS		12		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)

// NOTE: 9 modelnum bits is not enough for Prey's game/feedingtowera
// (because of very large func_static .proc model count)
#define	MODELNUM_BITS		10		// don't need to send any more
#define	MAX_MODELS			(1<<MODELNUM_BITS)

#define TAGNUM_BITS			8
#define MAX_BONES			(1<<TAGNUM_BITS)

#define ANIMNUM_BITS		8
#define MAX_ANIMATIONS		(1<<ANIMNUM_BITS)

#define RAGDOLLDEFNUM_BITS	8
#define MAX_RAGDOLLDEFS		(1<<RAGDOLLDEFNUM_BITS)

#define SOUNDNUM_BITS		8
#define MAX_SOUNDS			(1<<SOUNDNUM_BITS)

#define SKINNUM_BITS	8
#define MAX_SKINS		(1<<SKINNUM_BITS)

#define MATERIALNUM_BITS	8
#define MAX_MATERIALS		(1<<MATERIALNUM_BITS)

#define	MAX_CONFIGSTRINGS	4096 // 2048

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_GAMESTATE_CHARS	16000
typedef struct {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
} gameState_t;

//=========================================================

#include "../math/vec3.h" // needed by entityState_s and playerState_s

// entity flags
enum entityFlags_e {
	EF_HIDDEN = 1,
	EF_NUM_FLAG_BITS = 1
};

// flags for light objects - LF_NOSHADOWS, etc
#include "../shared/lightFlags.h"

// compressed bone orientation
struct netBoneOr_s {
	vec3_c xyz;
	vec3_c quatXYZ; // W can be easily recalculated
};
#define MAX_NETWORKED_BONES 64

// entityState_s is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

struct entityState_s {
	int		number;			// entity index
	int		eType;			// entityType_t
	int		eFlags;			// entityFlags_e

	class vec3_c	origin;
	class vec3_c	angles;

	int		groundEntityNum;	// ENTITYNUM_NONE = in air

	int	rModelIndex; // only for clientside rendering
	int rSkinIndex;
	int colModelIndex; // for collision detection

	int parentNum; // ENTITYNUM_NONE = not attached
	int parentTagNum;
	vec3_c parentOffset; // in local space
	vec3_c localAttachmentAngles;

	//int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses

//	int		solid;			// for client side prediction, trap_linkentity sets this properly

	// base animation index
	// (full body animation)
	int animIndex;
	// torso animation index
	// (overrides the full body animation)
	int torsoAnim;

	int lightFlags;
	float lightRadius; // only for ET_LIGHT 
	int lightTarget; // for spotlights (light target's entity number)
	float spotLightRadius; // for spotlights

	// trail emitter data for all entity types (including BaseEntity)
	int trailEmitterMaterial; 
	float trailEmitterSpriteRadius;
	int trailEmitterInterval; // in msec

	// for networked ragdolls (and bone controllers?)
	netBoneOr_s boneOrs[MAX_NETWORKED_BONES];

	// index of ragdoll def for ACTIVE ragdoll entities (CS_RAGDOLLDEFS)
	int activeRagdollDefNameIndex; // Doom3 ArticulatedFigure decl name

	void setDefaults() {
		number = -1;
		eType = 0;
		origin.clear();
		angles.clear();
		groundEntityNum = ENTITYNUM_NONE;
		rModelIndex = 0;
		colModelIndex = 0;
		parentNum = ENTITYNUM_NONE;
		parentTagNum = -1;
		animIndex = 0;
		activeRagdollDefNameIndex = 0;
		rSkinIndex = 0;
		eFlags = 0;
		lightFlags = 0;
		lightRadius = 0.f;
		lightTarget = ENTITYNUM_NONE;
		spotLightRadius = 0.f;
		trailEmitterMaterial = 0;
		trailEmitterSpriteRadius = 0.f;
		trailEmitterInterval = 0;
	}
	entityState_s() {
		setDefaults();
	}
	inline bool isEmitterActive() const {
		if(trailEmitterMaterial == 0)
			return false;
		if(trailEmitterSpriteRadius <= 0)
			return false;
		return true;
	}
	inline bool isHidden() const {
		return (eFlags & EF_HIDDEN);
	}
	inline void hideEntity() {
		eFlags |= EF_HIDDEN;
	}
	inline void showEntity() {
		eFlags &= ~EF_HIDDEN;
	}
};

enum animFlags_e {
	// don't restart animation when it's finished, 
	// just stop at the last frame
	ANIMFLAG_STOPATLASTFRAME = 1,
	ANIMFLAG_BITS = 1,
};

// bit field limits
//#define	MAX_STATS				16
//#define	MAX_PERSISTANT			16
//#define	MAX_POWERUPS			16
//#define	MAX_WEAPONS				16		

// playerState_s is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_s is a full superset of entityState_s as it is used by players,
// so if a playerState_s is transmitted, the entityState_s can be fully derived
// from it.
struct playerState_s : public entityState_s {
	int			commandTime;	// cmd->serverTime of last executed command

	int			clientNum;		// ranges from 0 to MAX_CLIENTS-1

	vec3_c		velocity;

	int			delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

//	int			groundEntityNum;// ENTITYNUM_NONE = in air

	// current mainhand weapon / item
	int			curWeaponEntNum;

	// index in CS_MODELS
	int			customViewRModelIndex; // 0 means that there is no custom view render model
	// index in CS_ANIMATIONS
	int			viewModelAnim;
	// animation flags
	int			viewModelAnimFlags;

	vec3_c		viewangles;		// for fixed views
	int			viewheight;
	// viewmodel (weapon) offset/angles,
	// relative to player eye
	// (this is set through "def_viewStyle" key in weapon .def)
	vec3_c		viewModelAngles;
	vec3_c		viewModelOffset;
	//  view weapon ammo data (only for display)
	int			viewWeaponMaxClipSize;
	int			viewWeaponCurClipSize;
	// int		viewWeaponAmmoTypeIndex;

	// not communicated over the net at all
	int			ping;			// server to game info for scoreboard

	playerState_s() {
		delta_angles[0] = delta_angles[1] = delta_angles[2] = 0;
		curWeaponEntNum = ENTITYNUM_NONE;
		customViewRModelIndex = 0;
		viewangles.set(0,0,0);
		viewheight = 0;
		ping = 0;
		viewWeaponMaxClipSize = 0;
		viewWeaponCurClipSize = 0;
		viewModelAngles.set(0,0,0);
		viewModelOffset.set(0,0,0);
	}
	bool isOnGround() const {
		return (groundEntityNum != ENTITYNUM_NONE);
	}
};


//====================================================================

#include "../shared/usercmd.h"

// real time
//=============================================


typedef struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
} qtime_t;


// server browser sources
// TTimo: AS_MPLAYER is no longer used
#define AS_LOCAL			0
#define AS_MPLAYER		1
#define AS_GLOBAL			2
#define AS_FAVORITES	3


// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

#define	MAX_GLOBAL_SERVERS				4096
#define	MAX_OTHER_SERVERS					128
#define MAX_PINGREQUESTS					32
#define MAX_SERVERSTATUSREQUESTS	16

#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2
//
//#define LERP( a, b, w ) ( ( a ) * ( 1.0f - ( w ) ) + ( b ) * ( w ) )
//#define LUMA( red, green, blue ) ( 0.2126f * ( red ) + 0.7152f * ( green ) + 0.0722f * ( blue ) )



#endif	// __Q_SHARED_H
