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

#include "server.h"
#include <shared/autoCvar.h>
#include <api/coreAPI.h>
#include <shared/entityType.h>
#include <protocol/snapFlags.h>
#include <protocol/voipFlags.h>

static aCvar_c sv_debugPlayerSnapshotEntities("sv_debugPlayerSnapshotEntities","-1");

/*
=============================================================================

Delta encode a client frame onto the network channel

A normal server packet will look like:

4	sequence number (high bit set if an oversize fragment)
<optional reliable commands>
1	svc_snapshot
4	last client reliable command
4	serverTime
1	lastframe for delta compression
1	snapFlags
1	areaBytes
<areabytes>
<playerstate>
<packetentities>

=============================================================================
*/

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entityState_s list to the message.
=============
*/
static void SV_EmitPacketEntities( clientSnapshot_s *from, clientSnapshot_s *to, msg_c *msg ) {
	entityState_s	*oldent, *newent;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;

	// generate the delta update
	if ( !from ) {
		from_num_entities = 0;
	} else {
		from_num_entities = from->num_entities;
	}

	newent = NULL;
	oldent = NULL;
	newindex = 0;
	oldindex = 0;
	while ( newindex < to->num_entities || oldindex < from_num_entities ) {
		if ( newindex >= to->num_entities ) {
			newnum = 9999;
		} else {
			newent = &svs.snapshotEntities[(to->first_entity+newindex) % svs.numSnapshotEntities];
			newnum = newent->number;
		}

		if ( oldindex >= from_num_entities ) {
			oldnum = 9999;
		} else {
			oldent = &svs.snapshotEntities[(from->first_entity+oldindex) % svs.numSnapshotEntities];
			oldnum = oldent->number;
		}

		if ( newnum == oldnum ) {
			if(sv_debugPlayerSnapshotEntities.getInt() == to->ps.number) {
				//Com_Printf("SV_EmitPacketEntities: delting entity %i from player %i view\n",
				//	oldnum,from->ps.number);
			}
			// delta update from old position
			// because the force parm is false, this will not result
			// in any bytes being emited if the entity has not changed at all
			msg->writeDeltaEntity( oldent, newent, false );
			oldindex++;
			newindex++;
			continue;
		}

		if ( newnum < oldnum ) {
			if(sv_debugPlayerSnapshotEntities.getInt() == to->ps.number) {
				Com_Printf("SV_EmitPacketEntities: adding entity %i from player %i view\n",
					newnum,to->ps.number);
			}
			// this is a new entity, send it from the baseline
			msg->writeDeltaEntity(&sv.svEntities[newnum].baseline, newent, true );
			newindex++;
			continue;
		}

		if ( newnum > oldnum ) {
			if(sv_debugPlayerSnapshotEntities.getInt() == to->ps.number) {
				Com_Printf("SV_EmitPacketEntities: removing entity %i from player %i view\n",
					oldnum,to->ps.number);
			}
			// the old entity isn't present in the new message
			msg->writeDeltaEntity(oldent, NULL, true );
			oldindex++;
			continue;
		}
	}

	msg->writeBits((MAX_GENTITIES-1), GENTITYNUM_BITS );	// end of packetentities
}



/*
==================
SV_WriteSnapshotToClient
==================
*/
static void SV_WriteSnapshotToClient( client_t *client, msg_c *msg ) {
	clientSnapshot_s	*frame, *oldframe;
	int					lastframe;
	int					i;
	int					snapFlags;

	// this is the snapshot we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];

	// try to use a previous frame as the source for delta compressing the snapshot
	if ( client->deltaMessage <= 0 || client->state != CS_ACTIVE ) {
		// client is asking for a retransmit
		oldframe = NULL;
		lastframe = 0;
	} else if ( client->netchan.outgoingSequence - client->deltaMessage 
		>= (PACKET_BACKUP - 3) ) {
		// client hasn't gotten a good message through in a long time
		Com_DPrintf ("%s: Delta request from out of date packet.\n", client->name);
		oldframe = NULL;
		lastframe = 0;
	} else {
		// we have a valid snapshot to delta from
		oldframe = &client->frames[ client->deltaMessage & PACKET_MASK ];
		lastframe = client->netchan.outgoingSequence - client->deltaMessage;

		// the snapshot's entities may still have rolled off the buffer, though
		if ( oldframe->first_entity <= svs.nextSnapshotEntities - svs.numSnapshotEntities ) {
			Com_DPrintf ("%s: Delta request from out of date entities.\n", client->name);
			oldframe = NULL;
			lastframe = 0;
		}
	}

	msg->writeByte(svc_snapshot);

	// NOTE, MRE: now sent at the start of every message from server to client
	// let the client know which reliable clientCommands we have received
	//msg->writeLong(client->lastClientCommand );

	// send over the current server time so the client can drift
	// its view of time to try to match
	if( client->oldServerTime ) {
		// The server has not yet got an acknowledgement of the
		// new gamestate from this client, so continue to send it
		// a time as if the server has not restarted. Note from
		// the client's perspective this time is strictly speaking
		// incorrect, but since it'll be busy loading a map at
		// the time it doesn't really matter.
		msg->writeLong( sv.time + client->oldServerTime);
	} else {
		msg->writeLong( sv.time);
	}

	// what we are delta'ing from
	msg->writeByte(lastframe);

	snapFlags = svs.snapFlagServerBit;
	if ( client->rateDelayed ) {
		snapFlags |= SNAPFLAG_RATE_DELAYED;
	}
	if ( client->state != CS_ACTIVE ) {
		snapFlags |= SNAPFLAG_NOT_ACTIVE;
	}

	msg->writeByte(snapFlags);

	// send over the areabits
	msg->writeByte(frame->areabytes);
	msg->writeData(frame->areabits, frame->areabytes);

	// delta encode the playerstate
	if ( oldframe ) {
		msg->writeDeltaPlayerstate(&oldframe->ps, &frame->ps );
	} else {
		msg->writeDeltaPlayerstate(NULL, &frame->ps );
	}

	// delta encode the entities
	SV_EmitPacketEntities (oldframe, frame, msg);

	// padding for rate debugging
	if ( sv_padPackets->integer ) {
		for ( i = 0 ; i < sv_padPackets->integer ; i++ ) {
			msg->writeByte(svc_nop);
		}
	}
}


/*
==================
SV_UpdateServerCommandsToClient

(re)send all server commands the client hasn't acknowledged yet
==================
*/
void SV_UpdateServerCommandsToClient( client_t *client, msg_c *msg ) {
	int		i;

	// write any unacknowledged serverCommands
	for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
		msg->writeByte(svc_serverCommand );
		msg->writeLong(i );
		msg->writeString(client->reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
	}
	client->reliableSent = client->reliableSequence;
}

/*
=============================================================================

Build a client snapshot structure

=============================================================================
*/

#define	MAX_SNAPSHOT_ENTITIES	MAX_GENTITIES
typedef struct {
	int		numSnapshotEntities;
	int		snapshotEntities[MAX_SNAPSHOT_ENTITIES];	
} snapshotEntityNumbers_t;

/*
=======================
SV_QsortEntityNumbers
=======================
*/
static int QDECL SV_QsortEntityNumbers( const void *a, const void *b ) {
	int	*ea, *eb;

	ea = (int *)a;
	eb = (int *)b;

	if ( *ea == *eb ) {
		Com_Error( ERR_DROP, "SV_QsortEntityStates: duplicated entity" );
	}

	if ( *ea < *eb ) {
		return -1;
	}

	return 1;
}


/*
===============
SV_AddEntToSnapshot
===============
*/
static void SV_AddEntToSnapshot( svEntity_s *svEnt, edict_s *gEnt, snapshotEntityNumbers_t *eNums ) {
	// if we have already added this entity to this snapshot, don't add again
	if ( svEnt->snapshotCounter == sv.snapshotCounter ) {
		return;
	}
	svEnt->snapshotCounter = sv.snapshotCounter;

	// if we are full, silently discard entities
	if ( eNums->numSnapshotEntities == MAX_SNAPSHOT_ENTITIES ) {
		return;
	}

	eNums->snapshotEntities[ eNums->numSnapshotEntities ] = gEnt->s->number;
	eNums->numSnapshotEntities++;
}

/*
===============
SV_AddEntitiesVisibleFromPoint

use BSP PVS / proc portals data to determine
which entities are potentialy visible by player.
===============
*/
// for .bsp maps (Quake3, RTCW, ET, MoHAA, COD)
#include "sv_vis.h" 
#include <shared/bitset.h>
// for .proc maps (Doom3, Quake4)
#include <shared/doom3ProcPVSClass.h> 
#include <shared/autoCvar.h> 
static aCvar_c sv_cullEntities("sv_cullEntities","1");
static void SV_AddEntitiesVisibleFromPoint( vec3_t origin, clientSnapshot_s *frame, 
									snapshotEntityNumbers_t *eNums, bool portal, bitSet_c &areaBits ) {
	int		e;//, i;
	edict_s *ent;
	svEntity_s	*svEnt;
	//int		l;
	
	// during an error shutdown message we may need to transmit
	// the shutdown message after the server has shutdown, so
	// specfically check for it
	if ( !sv.state ) {
		return;
	}

	bspPointDesc_s eyeDesc; // for .bsp PVS
	pvsHandle_t procVisHandle; // for .proc vis
	if(sv_bsp) {
		sv_bsp->filterPoint(origin,eyeDesc);
		sv_bsp->appendCurrentAreaBits(eyeDesc.area,areaBits);
	} else if(sv_procVis) {
		procVisHandle = sv_procVis->SetupCurrentPVS(origin);		
	}


	for ( e = 0 ; e < sv.num_entities ; e++ ) {
		ent = SV_GentityNum(e);

		// never send entities that aren't active
		if ( ent->s == 0 ) {
			continue;
		}

		if (ent->s->number != e) {
			Com_DPrintf ("FIXING ENT->S.NUMBER!!!\n");
			ent->s->number = e;
		}

		// never send entities that aren't visible
		if(ent->s->isHidden()) {
			continue;
		}

		// never send ai waypoints (pathnodes)
		if(ent->s->eType == ET_PATHNODE) {
			continue;
		}

		edict_s *visEnt = ent;
		while(visEnt->s->parentNum != ENTITYNUM_NONE) {
			visEnt = SV_GentityNum(visEnt->s->parentNum);
		}
		if(visEnt->bspBoxDesc == 0) {
			continue; // not linked
		}

		svEnt = SV_SvEntityForGentity( ent );

		// don't double add an entity through portals
		if ( svEnt->snapshotCounter == sv.snapshotCounter ) {
			continue;
		}

		if(sv_cullEntities.getInt()) {
			if(sv_bsp && sv_bsp->checkVisibility(eyeDesc,*visEnt->bspBoxDesc) == false) {
				continue; // culled by .bsp PVS
			}
			if(sv_procVis && sv_procVis->InCurrentPVS(procVisHandle,visEnt->bspBoxDesc->areas.getArray(),visEnt->bspBoxDesc->areas.size()) == false) {
				continue; // culled by Doom3 .proc vis
			}
		}

		SV_AddEntToSnapshot( svEnt, ent, eNums );
	}	
	if(sv_procVis) {
		sv_procVis->FreeCurrentPVS(procVisHandle);		
	}
}

/*
=============
SV_BuildClientSnapshot

Decides which entities are going to be visible to the client, and
copies off the playerstate and areabits.

This properly handles multiple recursive portals, but the render
currently doesn't.

For viewing through other player's eyes, clent can be something other than client->gentity
=============
*/
static void SV_BuildClientSnapshot( client_t *client ) {
	vec3_c						org;
	clientSnapshot_s			*frame;
	snapshotEntityNumbers_t		entityNumbers;
	int							i;
	edict_s				*ent;
	entityState_s				*state;
	svEntity_s					*svEnt;
	edict_s				*clent;
	int							clientNum;
	playerState_s				*ps;

	// bump the counter used to prevent double adding
	sv.snapshotCounter++;

	// this is the frame we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];

	// clear everything in this snapshot
	entityNumbers.numSnapshotEntities = 0;
	memset( frame->areabits, 0, sizeof( frame->areabits ) );

  // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=62
	frame->num_entities = 0;
	
	clent = client->gentity;
	if ( !clent || client->state == CS_ZOMBIE ) {
		return;
	}

	// grab the current playerState_s
	ps = SV_GameClientNum( client - svs.clients );
	frame->ps = *ps;

	// never send client's own entity, because it can
	// be regenerated from the playerstate
	clientNum = frame->ps.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	svEnt = &sv.svEntities[ clientNum ];

	svEnt->snapshotCounter = sv.snapshotCounter;

	// find the client's viewpoint
	org = ps->origin;
	org[2] += ps->viewheight;

	// add all the entities directly visible to the eye, which
	// may include portal entities that merge other viewpoints
	bitSet_c areaBits;
	if(sv_bsp) {
		areaBits.init(sv_bsp->getNumAreas(),false);
	}
	SV_AddEntitiesVisibleFromPoint( org, frame, &entityNumbers, false, areaBits );

	memcpy(frame->areabits,areaBits.getArray(),areaBits.getSizeInBytes());
	frame->areabytes = areaBits.getSizeInBytes();

	// if there were portals visible, there may be out of order entities
	// in the list which will need to be resorted for the delta compression
	// to work correctly.  This also catches the error condition
	// of an entity being included twice.
	qsort( entityNumbers.snapshotEntities, entityNumbers.numSnapshotEntities, 
		sizeof( entityNumbers.snapshotEntities[0] ), SV_QsortEntityNumbers );

	// now that all viewpoint's areabits have been OR'd together, invert
	// all of them to make it a mask vector, which is what the renderer wants
	for ( i = 0 ; i < MAX_MAP_AREA_BYTES/4 ; i++ ) {
		((int *)frame->areabits)[i] = ((int *)frame->areabits)[i] ^ -1;
	}

	// copy the entity states out
	frame->num_entities = 0;
	frame->first_entity = svs.nextSnapshotEntities;
	for ( i = 0 ; i < entityNumbers.numSnapshotEntities ; i++ ) {
		ent = SV_GentityNum(entityNumbers.snapshotEntities[i]);
		state = &svs.snapshotEntities[svs.nextSnapshotEntities % svs.numSnapshotEntities];
		// copy entityState_s from entity/player
		*state = *ent->s;
		svs.nextSnapshotEntities++;
		// this should never hit, map should always be restarted first in SV_Frame
		if ( svs.nextSnapshotEntities >= 0x7FFFFFFE ) {
			Com_Error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
		}
		frame->num_entities++;
	}
}

#ifdef USE_VOIP
/*
==================
SV_WriteVoipToClient

Check to see if there is any VoIP queued for a client, and send if there is.
==================
*/
static void SV_WriteVoipToClient(client_t *cl, msg_c *msg)
{
	int totalbytes = 0;
	int i;
	voipServerPacket_t *packet;

	if(cl->queuedVoipPackets)
	{
		// Write as many VoIP packets as we reasonably can...
		for(i = 0; i < cl->queuedVoipPackets; i++)
		{
			packet = cl->voipPacket[(i + cl->queuedVoipIndex) % ARRAY_LEN(cl->voipPacket)];

			if(!*cl->downloadName)
			{
        			totalbytes += packet->len;
	        		if (totalbytes > (msg->maxsize - msg->cursize) / 2)
		        		break;

        			msg->writeByte(svc_voip);
        			msg->writeShort(packet->sender);
	        		msg->writeByte((byte) packet->generation);
		        	msg->writeLong(packet->sequence);
		        	msg->writeByte(packet->frames);
        			msg->writeShort(packet->len);
        			msg->writeBits(packet->flags, VOIP_FLAGCNT);
	        		msg->writeData(packet->data, packet->len);
                        }

			free(packet);
		}

		cl->queuedVoipPackets -= i;
		cl->queuedVoipIndex += i;
		cl->queuedVoipIndex %= ARRAY_LEN(cl->voipPacket);
	}
}
#endif

/*
=======================
SV_SendMessageToClient

Called by SV_SendClientSnapshot and SV_SendClientGameState
=======================
*/
void SV_SendMessageToClient(msg_c *msg, client_t *client)
{
	// record information about the message
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSize = msg->cursize;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSent = svs.time;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageAcked = -1;

	// send the datagram
	SV_Netchan_Transmit(client, msg);
}


/*
=======================
SV_SendClientSnapshot

Also called by SV_FinalMessage

=======================
*/
void SV_SendClientSnapshot( client_t *client ) {
	byte		msg_buf[MAX_MSGLEN];
	msg_c		msg;

	// build the snapshot
	SV_BuildClientSnapshot( client );

	// bots need to have their snapshots build, but
	// the query them directly without needing to be sent
	//if ( client->gentity && client->gentity->r.svFlags & SVF_BOT ) {
	//	return;
	//}

	msg.init(msg_buf, sizeof(msg_buf));
	msg.allowoverflow = true;

	// compression byte is the first byte in all server->client messages
	msg.oob = true;
	msg.writeByte(0);
	msg.oob = false;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	msg.writeLong(client->lastClientCommand );

	// (re)send any reliable server commands
	SV_UpdateServerCommandsToClient( client, &msg );

	// send over all the relevant entityState_s
	// and the playerState_s
	SV_WriteSnapshotToClient( client, &msg );

#ifdef USE_VOIP
	SV_WriteVoipToClient( client, &msg );
#endif

	// check for overflow
	if ( msg.overflowed ) {
		Com_Printf ("WARNING: msg overflowed for %s\n", client->name);
		msg.clear();
	}

	SV_SendMessageToClient( &msg, client );
}


/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages(void)
{
	int		i;
	client_t	*c;

	// send a message to each connected client
	for(i=0; i < sv_maxclients->integer; i++)
	{
		c = &svs.clients[i];
		
		if(!c->state)
			continue;		// not connected

		if(*c->downloadName)
			continue;		// Client is downloading, don't send snapshots

		if(c->netchan.unsentFragments || c->netchan_start_queue)
		{
			c->rateDelayed = true;
			continue;		// Drop this snapshot if the packet queue is still full or delta compression will break
		}

		if(!(c->netchan.remoteAddress.type == NA_LOOPBACK ||
		     (sv_lanForceRate->integer && Sys_IsLANAddress(c->netchan.remoteAddress))))
		{
			// rate control for clients not on LAN 
			
			if(svs.time - c->lastSnapshotTime < c->snapshotMsec * com_timescale->value)
				continue;		// It's not time yet

			if(SV_RateMsec(c) > 0)
			{
				// Not enough time since last packet passed through the line
				c->rateDelayed = true;
				continue;
			}
		}

		// generate and send a new message
		SV_SendClientSnapshot(c);
		c->lastSnapshotTime = svs.time;
		c->rateDelayed = false;
	}
}
