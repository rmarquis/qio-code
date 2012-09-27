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
// world.c -- world query functions

#include "server.h"

/*
===============================================================================

ENTITY CHECKING

To avoid linearly searching through lists of entities during environment testing,
the world is carved up with an evenly spaced, axially aligned bsp tree.  Entities
are kept in chains either at the final leafs, or at the first node that splits
them, which prevents having to deal with multiple fragments of a single entity.

===============================================================================
*/

typedef struct worldSector_s {
	int		axis;		// -1 = leaf node
	float	dist;
	struct worldSector_s	*children[2];
	svEntity_t	*entities;
} worldSector_t;

#define	AREA_DEPTH	4
#define	AREA_NODES	64

worldSector_t	sv_worldSectors[AREA_NODES];
int			sv_numworldSectors;


/*
===============
SV_SectorList_f
===============
*/
void SV_SectorList_f( void ) {
	int				i, c;
	worldSector_t	*sec;
	svEntity_t		*ent;

	for ( i = 0 ; i < AREA_NODES ; i++ ) {
		sec = &sv_worldSectors[i];

		c = 0;
		for ( ent = sec->entities ; ent ; ent = ent->nextEntityInWorldSector ) {
			c++;
		}
		Com_Printf( "sector %i: %i entities\n", i, c );
	}
}

/*
===============
SV_CreateworldSector

Builds a uniformly subdivided tree for the given world size
===============
*/
static worldSector_t *SV_CreateworldSector( int depth, vec3_t mins, vec3_t maxs ) {
	worldSector_t	*anode;
	vec3_t		size;
	vec3_t		mins1, maxs1, mins2, maxs2;

	anode = &sv_worldSectors[sv_numworldSectors];
	sv_numworldSectors++;

	if (depth == AREA_DEPTH) {
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}
	
	VectorSubtract (maxs, mins, size);
	if (size[0] > size[1]) {
		anode->axis = 0;
	} else {
		anode->axis = 1;
	}

	anode->dist = 0.5 * (maxs[anode->axis] + mins[anode->axis]);
	VectorCopy (mins, mins1);	
	VectorCopy (mins, mins2);	
	VectorCopy (maxs, maxs1);	
	VectorCopy (maxs, maxs2);	
	
	maxs1[anode->axis] = mins2[anode->axis] = anode->dist;
	
	anode->children[0] = SV_CreateworldSector (depth+1, mins2, maxs2);
	anode->children[1] = SV_CreateworldSector (depth+1, mins1, maxs1);

	return anode;
}

/*
===============
SV_ClearWorld

===============
*/
void SV_ClearWorld( void ) {
	clipHandle_t	h;
	vec3_t			mins, maxs;

	Com_Memset( sv_worldSectors, 0, sizeof(sv_worldSectors) );
	sv_numworldSectors = 0;

	// get world map bounds
	//h = CM_InlineModel( 0 );
	//CM_ModelBounds( h, mins, maxs );
	VectorSet(mins,-8192,-8192,-8192);
	VectorSet(maxs,8192,8192,8192);
	SV_CreateworldSector( 0, mins, maxs );
}


/*
===============
SV_UnlinkEntity

===============
*/
void SV_UnlinkEntity( sharedEntity_t *gEnt ) {
	svEntity_t		*ent;
	svEntity_t		*scan;
	worldSector_t	*ws;

	ent = SV_SvEntityForGentity( gEnt );

	gEnt->r.linked = qfalse;

	ws = ent->worldSector;
	if ( !ws ) {
		return;		// not linked in anywhere
	}
	ent->worldSector = NULL;

	if ( ws->entities == ent ) {
		ws->entities = ent->nextEntityInWorldSector;
		return;
	}

	for ( scan = ws->entities ; scan ; scan = scan->nextEntityInWorldSector ) {
		if ( scan->nextEntityInWorldSector == ent ) {
			scan->nextEntityInWorldSector = ent->nextEntityInWorldSector;
			return;
		}
	}

	Com_Printf( "WARNING: SV_UnlinkEntity: not found in worldSector\n" );
}


/*
===============
SV_LinkEntity

===============
*/
#define MAX_TOTAL_ENT_LEAFS		128
void SV_LinkEntity( sharedEntity_t *gEnt ) {
	worldSector_t	*node;
//	int			leafs[MAX_TOTAL_ENT_LEAFS];
//	int			cluster;
//	int			num_leafs;
	int			i;
//	int			area;
//	int			lastLeaf;
	float		*origin, *angles;
	svEntity_t	*ent;

	ent = SV_SvEntityForGentity( gEnt );

	if ( ent->worldSector ) {
		SV_UnlinkEntity( gEnt );	// unlink from old position
	}

	// encode the size into the entityState_t for client prediction
	//if ( gEnt->r.bmodel ) {
	//	gEnt->s.solid = SOLID_BMODEL;		// a solid_box will never create this value
	//} else
	//	if ( gEnt->r.contents & ( CONTENTS_SOLID | CONTENTS_BODY ) ) {
	//	// assume that x/y are equal and symetric
	//	i = gEnt->r.maxs[0];
	//	if (i<1)
	//		i = 1;
	//	if (i>255)
	//		i = 255;

	//	// z is not symetric
	//	j = (-gEnt->r.mins[2]);
	//	if (j<1)
	//		j = 1;
	//	if (j>255)
	//		j = 255;

	//	// and z maxs can be negative...
	//	k = (gEnt->r.maxs[2]+32);
	//	if (k<1)
	//		k = 1;
	//	if (k>255)
	//		k = 255;

	//	gEnt->s.solid = (k<<16) | (j<<8) | i;
	//} else 
	{
		gEnt->s.solid = 0;
	}

	// get the position
	origin = gEnt->s.origin;
	angles = gEnt->s.angles;

	// set the abs box
	if ( gEnt->r.bmodel && (angles[0] || angles[1] || angles[2]) ) {
		// expand for rotation
		float		max;
		int			i;

		max = RadiusFromBounds( gEnt->r.mins, gEnt->r.maxs );
		for (i=0 ; i<3 ; i++) {
			gEnt->r.absmin[i] = origin[i] - max;
			gEnt->r.absmax[i] = origin[i] + max;
		}
	} else {
		// normal
		VectorAdd (origin, gEnt->r.mins, gEnt->r.absmin);	
		VectorAdd (origin, gEnt->r.maxs, gEnt->r.absmax);
	}

	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	gEnt->r.absmin[0] -= 1;
	gEnt->r.absmin[1] -= 1;
	gEnt->r.absmin[2] -= 1;
	gEnt->r.absmax[0] += 1;
	gEnt->r.absmax[1] += 1;
	gEnt->r.absmax[2] += 1;

	// link to PVS leafs
	ent->numClusters = 0;
	ent->lastCluster = 0;
	ent->areanum = -1;
	ent->areanum2 = -1;

	//get all leafs, including solids
	//num_leafs = CM_BoxLeafnums( gEnt->r.absmin, gEnt->r.absmax,
	//	leafs, MAX_TOTAL_ENT_LEAFS, &lastLeaf );

	//// if none of the leafs were inside the map, the
	//// entity is outside the world and can be considered unlinked
	//if ( !num_leafs ) {
	//	return;
	//}

	//// set areas, even from clusters that don't fit in the entity array
	//for (i=0 ; i<num_leafs ; i++) {
	//	area = CM_LeafArea (leafs[i]);
	//	if (area != -1) {
	//		// doors may legally straggle two areas,
	//		// but nothing should evern need more than that
	//		if (ent->areanum != -1 && ent->areanum != area) {
	//			if (ent->areanum2 != -1 && ent->areanum2 != area && sv.state == SS_LOADING) {
	//				Com_DPrintf ("Object %i touching 3 areas at %f %f %f\n",
	//				gEnt->s.number,
	//				gEnt->r.absmin[0], gEnt->r.absmin[1], gEnt->r.absmin[2]);
	//			}
	//			ent->areanum2 = area;
	//		} else {
	//			ent->areanum = area;
	//		}
	//	}
	//}

	//// store as many explicit clusters as we can
	//ent->numClusters = 0;
	//for (i=0 ; i < num_leafs ; i++) {
	//	cluster = CM_LeafCluster( leafs[i] );
	//	if ( cluster != -1 ) {
	//		ent->clusternums[ent->numClusters++] = cluster;
	//		if ( ent->numClusters == MAX_ENT_CLUSTERS ) {
	//			break;
	//		}
	//	}
	//}

	//// store off a last cluster if we need to
	//if ( i != num_leafs ) {
	//	ent->lastCluster = CM_LeafCluster( lastLeaf );
	//}

	gEnt->r.linkcount++;

	// find the first world sector node that the ent's box crosses
	node = sv_worldSectors;
	while (1)
	{
		if (node->axis == -1)
			break;
		if ( gEnt->r.absmin[node->axis] > node->dist)
			node = node->children[0];
		else if ( gEnt->r.absmax[node->axis] < node->dist)
			node = node->children[1];
		else
			break;		// crosses the node
	}
	
	// link it in
	ent->worldSector = node;
	ent->nextEntityInWorldSector = node->entities;
	node->entities = ent;

	gEnt->r.linked = qtrue;
}

/*
============================================================================

AREA QUERY

Fills in a list of all entities who's absmin / absmax intersects the given
bounds.  This does NOT mean that they actually touch in the case of bmodels.
============================================================================
*/

typedef struct {
	const float	*mins;
	const float	*maxs;
	int			*list;
	int			count, maxcount;
} areaParms_t;


/*
====================
SV_AreaEntities_r

====================
*/
static void SV_AreaEntities_r( worldSector_t *node, areaParms_t *ap ) {
	svEntity_t	*check, *next;
	sharedEntity_t *gcheck;

	for ( check = node->entities  ; check ; check = next ) {
		next = check->nextEntityInWorldSector;

		gcheck = SV_GEntityForSvEntity( check );

		if ( gcheck->r.absmin[0] > ap->maxs[0]
		|| gcheck->r.absmin[1] > ap->maxs[1]
		|| gcheck->r.absmin[2] > ap->maxs[2]
		|| gcheck->r.absmax[0] < ap->mins[0]
		|| gcheck->r.absmax[1] < ap->mins[1]
		|| gcheck->r.absmax[2] < ap->mins[2]) {
			continue;
		}

		if ( ap->count == ap->maxcount ) {
			Com_Printf ("SV_AreaEntities: MAXCOUNT\n");
			return;
		}

		ap->list[ap->count] = check - sv.svEntities;
		ap->count++;
	}
	
	if (node->axis == -1) {
		return;		// terminal node
	}

	// recurse down both sides
	if ( ap->maxs[node->axis] > node->dist ) {
		SV_AreaEntities_r ( node->children[0], ap );
	}
	if ( ap->mins[node->axis] < node->dist ) {
		SV_AreaEntities_r ( node->children[1], ap );
	}
}

/*
================
SV_AreaEntities
================
*/
int SV_AreaEntities( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount ) {
	areaParms_t		ap;

	ap.mins = mins;
	ap.maxs = maxs;
	ap.list = entityList;
	ap.count = 0;
	ap.maxcount = maxcount;

	SV_AreaEntities_r( sv_worldSectors, &ap );

	return ap.count;
}



//===========================================================================


typedef struct {
	vec3_t		boxmins, boxmaxs;// enclose the test object along entire move
	const float	*mins;
	const float *maxs;	// size of the moving object
	const float	*start;
	vec3_t		end;
	trace_t		trace;
	int			passEntityNum;
	int			contentmask;
	int			capsule;
} moveclip_t;


/*
====================
SV_ClipToEntity

====================
*/
void SV_ClipToEntity( trace_t *trace, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentmask, int capsule ) {
	
}


/*
====================
SV_ClipMoveToEntities

====================
*/
static void SV_ClipMoveToEntities( moveclip_t *clip ) {

}


/*
==================
SV_Trace

Moves the given mins/maxs volume through the world from start to end.
passEntityNum and entities owned by passEntityNum are explicitly not checked.
==================
*/
void SV_Trace( trace_t *results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int capsule ) {

}



/*
=============
SV_PointContents
=============
*/
int SV_PointContents( const vec3_t p, int passEntityNum ) {

	return 0;
}


