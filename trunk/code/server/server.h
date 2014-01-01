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
// server.h

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../game/g_public.h"
#include "../game/bg_public.h"

//=============================================================================

#define	PERS_SCORE				0		// !!! MUST NOT CHANGE, SERVER AND
										// GAME BOTH REFERENCE !!!

#define	MAX_ENT_CLUSTERS	16

#ifdef USE_VOIP
#define VOIP_QUEUE_LENGTH 64

typedef struct voipServerPacket_s
{
	int generation;
	int sequence;
	int frames;
	int len;
	int sender;
	int flags;
	byte data[1024];
} voipServerPacket_t;
#endif

typedef struct svEntity_s {
	struct worldSector_s *worldSector;
	struct svEntity_s *nextEntityInWorldSector;
	
	entityState_s	baseline;		// for delta compression of initial sighting
	int			numClusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			lastCluster;		// if all the clusters don't fit in clusternums
	int			areanum, areanum2;
	int			snapshotCounter;	// used to prevent double adding from portal views
} svEntity_t;

typedef enum {
	SS_DEAD,			// no map loaded
	SS_LOADING,			// spawning level entities
	SS_GAME				// actively running
} serverState_t;

typedef struct {
	serverState_t	state;
	bool		restarting;			// if true, send configstring changes during SS_LOADING
	int				serverId;			// changes each server start
	int				restartedServerId;	// serverId before a map_restart
	int				checksumFeed;		// the feed key that we use to compute the pure checksum strings
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=475
	// the serverId associated with the current checksumFeed (always <= serverId)
	int       checksumFeedServerId;	
	int				snapshotCounter;	// incremented for each snapshot built
	int				timeResidual;		// <= 1000 / sv_frame->value
	int				nextFrameTime;		// when time > nextFrameTime, process world
	struct cmodel_s	*models[MAX_MODELS];
	char			*configstrings[MAX_CONFIGSTRINGS];
	svEntity_t		svEntities[MAX_GENTITIES];

	// the game virtual machine will update these on init and changes
	edict_s	*gentities;
	int				num_entities;		// current number, <= MAX_GENTITIES

	int				restartTime;
	int				time;
} server_t;





typedef struct {
	int				areabytes;
	byte			areabits[MAX_MAP_AREA_BYTES];		// portalarea visibility bits
	playerState_s	ps;
	int				num_entities;
	int				first_entity;		// into the circular sv_packet_entities[]
										// the entities MUST be in increasing state number
										// order, otherwise the delta compression will fail
	int				messageSent;		// time the message was transmitted
	int				messageAcked;		// time the message was acked
	int				messageSize;		// used to rate drop packets
} clientSnapshot_t;

typedef enum {
	CS_FREE,		// can be reused for a new connection
	CS_ZOMBIE,		// client has been disconnected, but don't reuse
					// connection for a couple seconds
	CS_CONNECTED,	// has been assigned to a client_t, but no gamestate yet
	CS_PRIMED,		// gamestate has been sent, but client hasn't sent a usercmd
	CS_ACTIVE		// client is fully in game
} clientState_t;

typedef struct netchan_buffer_s {
	msg_t           msg;
	byte            msgBuffer[MAX_MSGLEN];
#ifdef LEGACY_PROTOCOL
	char		clientCommandString[MAX_STRING_CHARS];	// valid command string for SV_Netchan_Encode
#endif
	struct netchan_buffer_s *next;
} netchan_buffer_t;

typedef struct client_s {
	clientState_t	state;
	char			userinfo[MAX_INFO_STRING];		// name, etc

	char			reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
	int				reliableSequence;		// last added reliable message, not necesarily sent or acknowledged yet
	int				reliableAcknowledge;	// last acknowledged reliable message
	int				reliableSent;			// last sent reliable message, not necesarily acknowledged yet
	int				messageAcknowledge;

	int				gamestateMessageNum;	// netchan->outgoingSequence of gamestate
	int				challenge;

	usercmd_s		lastUsercmd;
	int				lastMessageNum;		// for delta compression
	int				lastClientCommand;	// reliable client message sequence
	char			lastClientCommandString[MAX_STRING_CHARS];
	edict_s	*gentity;			// SV_GentityNum(clientnum)
	char			name[MAX_NAME_LENGTH];			// extracted from userinfo, high bits masked

	// downloading
	char			downloadName[MAX_QPATH]; // if not empty string, we are downloading
	fileHandle_t	download;			// file being downloaded
 	int				downloadSize;		// total bytes (can't use EOF because of paks)
 	int				downloadCount;		// bytes sent
	int				downloadClientBlock;	// last block we sent to the client, awaiting ack
	int				downloadCurrentBlock;	// current block number
	int				downloadXmitBlock;	// last block we xmited
	unsigned char	*downloadBlocks[MAX_DOWNLOAD_WINDOW];	// the buffers for the download blocks
	int				downloadBlockSize[MAX_DOWNLOAD_WINDOW];
	bool		downloadEOF;		// We have sent the EOF block
	int				downloadSendTime;	// time we last got an ack from the client

	int				deltaMessage;		// frame last client usercmd message
	int				nextReliableTime;	// svs.time when another reliable command will be allowed
	int				lastPacketTime;		// svs.time when packet was last received
	int				lastConnectTime;	// svs.time when connection started
	int				lastSnapshotTime;	// svs.time of last sent snapshot
	bool		rateDelayed;		// true if nextSnapshotTime was set based on rate instead of snapshotMsec
	int				timeoutCount;		// must timeout a few frames in a row so debugging doesn't break
	clientSnapshot_t	frames[PACKET_BACKUP];	// updates can be delta'd from here
	int				ping;
	int				rate;				// bytes / second
	int				snapshotMsec;		// requests a snapshot every snapshotMsec unless rate choked
	int				pureAuthentic;
	bool  gotCP; // TTimo - additional flag to distinguish between a bad pure checksum, and no cp command at all
	netchan_t		netchan;
	// TTimo
	// queuing outgoing fragmented messages to send them properly, without udp packet bursts
	// in case large fragmented messages are stacking up
	// buffer them into this queue, and hand them out to netchan as needed
	netchan_buffer_t *netchan_start_queue;
	netchan_buffer_t **netchan_end_queue;

#ifdef USE_VOIP
	bool hasVoip;
	bool muteAllVoip;
	bool ignoreVoipFromClient[MAX_CLIENTS];
	voipServerPacket_t *voipPacket[VOIP_QUEUE_LENGTH];
	int queuedVoipPackets;
	int queuedVoipIndex;
#endif

	int				oldServerTime;
	bool		csUpdated[MAX_CONFIGSTRINGS+1];	
} client_t;

//=============================================================================


// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	2048
// Allow a certain amount of challenges to have the same IP address
// to make it a bit harder to DOS one single IP address from connecting
// while not allowing a single ip to grab all challenge resources
#define MAX_CHALLENGES_MULTI (MAX_CHALLENGES / 2)

#define	AUTHORIZE_TIMEOUT	5000

typedef struct {
	netadr_t	adr;
	int			challenge;
	int			clientChallenge;		// challenge number coming from the client
	int			time;				// time the last packet was sent to the autherize server
	int			pingTime;			// time the challenge response was sent to client
	int			firstTime;			// time the adr was first used, for authorize timeout checks
	bool	wasrefused;
	bool	connected;
} challenge_t;

// this structure will be cleared only when the game dll changes
typedef struct {
	bool	initialized;				// sv_init has completed

	int			time;						// will be strictly increasing across level changes

	int			snapFlagServerBit;			// ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()

	client_t	*clients;					// [sv_maxclients->integer];
	int			numSnapshotEntities;		// sv_maxclients->integer*PACKET_BACKUP*MAX_PACKET_ENTITIES
	int			nextSnapshotEntities;		// next snapshotEntities to use
	entityState_s	*snapshotEntities;		// [numSnapshotEntities]
	int			nextHeartbeatTime;
	challenge_t	challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting
	netadr_t	redirectAddress;			// for rcon return messages

	netadr_t	authorizeAddress;			// for rcon return messages
} serverStatic_t;

#define SERVER_MAXBANS	1024
// Structure for managing bans
typedef struct
{
	netadr_t ip;
	// For a CIDR-Notation type suffix
	int subnet;
	
	bool isexception;
} serverBan_t;

//=============================================================================

extern	serverStatic_t	svs;				// persistant server info across maps
extern	server_t		sv;					// cleared each map

// ===== serverside entity culling =====
// ...for .bsp world maps (Quake3, ET, RTCW, MoHAA, CoD)
extern class svBSP_c *sv_bsp;
// ...for .proc world maps (Doom3 and Quake4)
extern class portalizedBSPTree_c *sv_procTree;
extern class idPVS *sv_procVis;

extern	cvar_s	*sv_fps;
extern	cvar_s	*sv_timeout;
extern	cvar_s	*sv_zombietime;
extern	cvar_s	*sv_rconPassword;
extern	cvar_s	*sv_privatePassword;
extern	cvar_s	*sv_allowDownload;
extern	cvar_s	*sv_maxclients;

extern	cvar_s	*sv_privateClients;
extern	cvar_s	*sv_hostname;
extern	cvar_s	*sv_master[MAX_MASTER_SERVERS];
extern	cvar_s	*sv_reconnectlimit;
extern	cvar_s	*sv_showloss;
extern	cvar_s	*sv_padPackets;
extern	cvar_s	*sv_killserver;
extern	cvar_s	*sv_mapname;
extern	cvar_s	*sv_mapChecksum;
extern	cvar_s	*sv_serverid;
extern	cvar_s	*sv_minRate;
extern	cvar_s	*sv_maxRate;
extern	cvar_s	*sv_dlRate;
extern	cvar_s	*sv_minPing;
extern	cvar_s	*sv_maxPing;
extern	cvar_s	*sv_gametype;
extern	cvar_s	*sv_pure;
extern	cvar_s	*sv_floodProtect;
extern	cvar_s	*sv_lanForceRate;
#ifndef STANDALONE
extern	cvar_s	*sv_strictAuth;
#endif
extern	cvar_s	*sv_banFile;

extern	serverBan_t serverBans[SERVER_MAXBANS];
extern	int serverBansCount;

#ifdef USE_VOIP
extern	cvar_s	*sv_voip;
#endif


//===========================================================

//
// sv_main.c
//
void SV_FinalMessage (char *message);
void QDECL SV_SendServerCommand( client_t *cl, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));


void SV_AddOperatorCommands (void);
void SV_RemoveOperatorCommands (void);


void SV_MasterShutdown (void);
int SV_RateMsec(client_t *client);



//
// sv_init.c
//
void SV_SetConfigstring( int index, const char *val );
void SV_GetConfigstring( int index, char *buffer, int bufferSize );
void SV_UpdateConfigstrings( client_t *client );

void SV_SetUserinfo( int index, const char *val );
void SV_GetUserinfo( int index, char *buffer, int bufferSize );

void SV_ChangeMaxClients( void );
void SV_SpawnServer( char *server, bool killBots );

void SV_LoadMapVis(const char *mapName);
void SV_FreeMap();

//
// sv_client.c
//
void SV_GetChallenge(netadr_t from);

void SV_DirectConnect( netadr_t from );

#ifndef STANDALONE
void SV_AuthorizeIpPacket( netadr_t from );
#endif

void SV_ExecuteClientMessage( client_t *cl, msg_t *msg );
void SV_UserinfoChanged( client_t *cl );

void SV_ClientEnterWorld( client_t *client, usercmd_s *cmd );
void SV_FreeClient(client_t *client);
void SV_DropClient( client_t *drop, const char *reason );

void SV_ExecuteClientCommand( client_t *cl, const char *s, bool clientOK );
void SV_ClientThink (client_t *cl, usercmd_s *cmd);

int SV_WriteDownloadToClient(client_t *cl , msg_t *msg);
int SV_SendDownloadMessages(void);
int SV_SendQueuedMessages(void);


//
// sv_ccmds.c
//
void SV_Heartbeat_f( void );

//
// sv_snapshot.c
//
void SV_AddServerCommand( client_t *client, const char *cmd );
void SV_UpdateServerCommandsToClient( client_t *client, msg_t *msg );
void SV_WriteFrameToClient (client_t *client, msg_t *msg);
void SV_SendMessageToClient( msg_t *msg, client_t *client );
void SV_SendClientMessages( void );
void SV_SendClientSnapshot( client_t *client );

//
// sv_game.c
//
int	SV_NumForGentity( edict_s *ent );
edict_s *SV_GentityNum( int num );
playerState_s *SV_GameClientNum( int num );
svEntity_t	*SV_SvEntityForGentity( edict_s *gEnt );
edict_s *SV_GEntityForSvEntity( svEntity_t *svEnt );
void		SV_InitGameProgs ( void );
void		SV_ShutdownGameProgs ( void );
void		SV_RestartGameProgs( void );
void SV_GameSendServerCommand( int clientNum, const char *text );
void SV_GameDropClient( int clientNum, const char *reason );
void SV_LocateGameData( edict_s *gEnts, int numGEntities );
void SV_GetUsercmd( int clientNum, usercmd_s *cmd );
void SV_LinkEntity(edict_s *ed);
void SV_UnlinkEntity(edict_s *ed);
void SV_AdjustAreaPortalState(int area0, int area1, bool open);

//============================================================

//
// sv_net_chan.c
//
void SV_Netchan_Transmit( client_t *client, msg_t *msg);
int SV_Netchan_TransmitNextFragment(client_t *client);
bool SV_Netchan_Process( client_t *client, msg_t *msg );
void SV_Netchan_FreeQueue(client_t *client);
