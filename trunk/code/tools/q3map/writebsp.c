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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "qbsp.h"

/*
============
EmitShader
============
*/
int	EmitShader( const char *shader ) {
	int				i;
	shaderInfo_t	*si;

	if ( !shader ) {
		shader = "noshader";
	}

	for ( i = 0 ; i < numShaders ; i++ ) {
		if ( !Q_stricmp( shader, dshaders[i].shader ) ) {
			return i;
		}
	}

	if ( i == MAX_MAP_SHADERS ) {
		Error( "MAX_MAP_SHADERS" );
	}
	numShaders++;
	strcpy( dshaders[i].shader, shader );

	si = ShaderInfoForShader( shader );
	dshaders[i].surfaceFlags = si->surfaceFlags;
	dshaders[i].contentFlags = si->contents;

	return i;
}


/*
============
EmitPlanes

There is no oportunity to discard planes, because all of the original
brushes will be saved in the map.
============
*/
void EmitPlanes (void)
{
	int			i;
	dplane_t	*dp;
	plane_t		*mp;

	mp = mapplanes;
	for (i=0 ; i<nummapplanes ; i++, mp++)
	{
		dp = &dplanes[numplanes];
		VectorCopy ( mp->normal, dp->normal);
		dp->dist = mp->dist;
		numplanes++;
	}
}



/*
==================
EmitLeaf
==================
*/
void EmitLeaf (node_t *node)
{
	dleaf_t				*leaf_p;
	bspbrush_t			*b;
	drawSurfRef_t		*dsr;

	// emit a leaf
	if (numleafs >= MAX_MAP_LEAFS)
		Error ("MAX_MAP_LEAFS");

	leaf_p = &dleafs[numleafs];
	numleafs++;

	leaf_p->cluster = node->cluster;
	leaf_p->area = node->area;

	//
	// write bounding box info
	//	
	VectorCopy (node->mins, leaf_p->mins);
	VectorCopy (node->maxs, leaf_p->maxs);
	
	//
	// write the leafbrushes
	//
	leaf_p->firstLeafBrush = numleafbrushes;
	for ( b = node->brushlist ; b ; b = b->next ) {
		if ( numleafbrushes >= MAX_MAP_LEAFBRUSHES ) {
			Error( "MAX_MAP_LEAFBRUSHES" );
		}
		dleafbrushes[numleafbrushes] = b->original->outputNumber;
		numleafbrushes++;
	}
	leaf_p->numLeafBrushes = numleafbrushes - leaf_p->firstLeafBrush;

	//
	// write the surfaces visible in this leaf
	//
	if ( node->opaque ) {
		return;		// no leaffaces in solids
	}
	
	// add the drawSurfRef_t drawsurfs
	leaf_p->firstLeafSurface = numleafsurfaces;
	for ( dsr = node->drawSurfReferences ; dsr ; dsr = dsr->nextRef ) {
		if ( numleafsurfaces >= MAX_MAP_LEAFFACES)
			Error ("MAX_MAP_LEAFFACES");
		dleafsurfaces[numleafsurfaces] = dsr->outputNumber;
		numleafsurfaces++;			
	}


	leaf_p->numLeafSurfaces = numleafsurfaces - leaf_p->firstLeafSurface;
}


/*
============
EmitDrawNode_r
============
*/
int EmitDrawNode_r (node_t *node)
{
	dnode_t	*n;
	int		i;

	if (node->planenum == PLANENUM_LEAF)
	{
		EmitLeaf (node);
		return -numleafs;
	}

	// emit a node	
	if (numnodes == MAX_MAP_NODES)
		Error ("MAX_MAP_NODES");
	n = &dnodes[numnodes];
	numnodes++;

	VectorCopy (node->mins, n->mins);
	VectorCopy (node->maxs, n->maxs);

	if (node->planenum & 1)
		Error ("WriteDrawNodes_r: odd planenum");
	n->planeNum = node->planenum;

	//
	// recursively output the other nodes
	//	
	for (i=0 ; i<2 ; i++)
	{
		if (node->children[i]->planenum == PLANENUM_LEAF)
		{
			n->children[i] = -(numleafs + 1);
			EmitLeaf (node->children[i]);
		}
		else
		{
			n->children[i] = numnodes;	
			EmitDrawNode_r (node->children[i]);
		}
	}

	return n - dnodes;
}

//=========================================================



/*
============
SetModelNumbers
============
*/
void SetModelNumbers (void)
{
	int		i;
	int		models;
	char	value[10];

	models = 1;
	for ( i=1 ; i<num_entities ; i++ ) {
		if ( entities[i].brushes || entities[i].patches ) {
			sprintf ( value, "*%i", models );
			models++;
			SetKeyValue (&entities[i], "model", value);
		}
	}

}

/*
============
SetLightStyles
============
*/
#define	MAX_SWITCHED_LIGHTS	32
void SetLightStyles (void)
{
	int		stylenum;
	const char	*t;
	entity_t	*e;
	int		i, j;
	char	value[10];
	char	lighttargets[MAX_SWITCHED_LIGHTS][64];


	// any light that is controlled (has a targetname)
	// must have a unique style number generated for it

	stylenum = 0;
	for (i=1 ; i<num_entities ; i++)
	{
		e = &entities[i];

		t = ValueForKey (e, "classname");
		if (Q_strncasecmp (t, "light", 5))
			continue;
		t = ValueForKey (e, "targetname");
		if (!t[0])
			continue;
		
		// find this targetname
		for (j=0 ; j<stylenum ; j++)
			if (!strcmp (lighttargets[j], t))
				break;
		if (j == stylenum)
		{
			if (stylenum == MAX_SWITCHED_LIGHTS)
				Error ("stylenum == MAX_SWITCHED_LIGHTS");
			strcpy (lighttargets[j], t);
			stylenum++;
		}
		sprintf (value, "%i", 32 + j);
		SetKeyValue (e, "style", value);
	}

}

//===========================================================

/*
==================
BeginBSPFile
==================
*/
void BeginBSPFile( void ) {
	// these values may actually be initialized
	// if the file existed when loaded, so clear them explicitly
	nummodels = 0;
	numnodes = 0;
	numbrushsides = 0;
	numleafsurfaces = 0;
	numleafbrushes = 0;

	// leave leaf 0 as an error, because leafs are referenced as
	// negative number nodes
	numleafs = 1;
}


/*
============
EndBSPFile
============
*/
void EndBSPFile( void ) {
	char	path[1024];

	EmitPlanes ();
	UnparseEntities ();

	// write the map
	sprintf (path, "%s.bsp", source);
	_printf ("Writing %s\n", path);
	WriteBSPFile (path);
}


//===========================================================

/*
============
EmitBrushes
============
*/
void EmitBrushes ( bspbrush_t *brushes ) {
	int				j;
	dbrush_t		*db;
	bspbrush_t		*b;
	dbrushside_t	*cp;

	for ( b = brushes ; b ; b = b->next ) {
		if ( numbrushes == MAX_MAP_BRUSHES ) {
			Error( "MAX_MAP_BRUSHES" );
		}
		// never write areaPortal brushes to bsp;
		// they are always non-solid
		if(b->contents & CONTENTS_AREAPORTAL) {
			continue;
		}

		b->outputNumber = numbrushes;
		db = &dbrushes[numbrushes];
		numbrushes++;

		db->shaderNum = EmitShader( b->contentShader->shader );
		db->firstSide = numbrushsides;

		// don't emit any generated backSide sides
		db->numSides = 0;
		for ( j=0 ; j<b->numsides ; j++ ) {
			if ( b->sides[j].backSide ) {
				continue;
			}
			if ( numbrushsides == MAX_MAP_BRUSHSIDES ) {
				Error( "MAX_MAP_BRUSHSIDES ");
			}
			cp = &dbrushsides[numbrushsides];
			db->numSides++;
			numbrushsides++;
			cp->planeNum = b->sides[j].planenum;
			cp->shaderNum = EmitShader( b->sides[j].shaderInfo->shader );
		}
	}

}


/*
==================
BeginModel
==================
*/
void BeginModel( void ) {
	dmodel_t	*mod;
	bspbrush_t	*b;
	entity_t	*e;
	vec3_t		mins, maxs;
	parseMesh_t	*p;
	int			i;

	if ( nummodels == MAX_MAP_MODELS ) {
		Error( "MAX_MAP_MODELS" );
	}
	mod = &dmodels[nummodels];

	//
	// bound the brushes
	//
	e = &entities[entity_num];

	ClearBounds (mins, maxs);
	for ( b = e->brushes ; b ; b = b->next ) {
		if ( !b->numsides ) {
			continue;	// not a real brush (origin brush, etc)
		}
		AddPointToBounds (b->mins, mins, maxs);
		AddPointToBounds (b->maxs, mins, maxs);
	}

	for ( p = e->patches ; p ; p = p->next ) {
		for ( i = 0 ; i < p->mesh.width * p->mesh.height ; i++ ) {
			AddPointToBounds( p->mesh.verts[i].xyz, mins, maxs );
		}
	}

	VectorCopy (mins, mod->mins);
	VectorCopy (maxs, mod->maxs);

	mod->firstSurface = numDrawSurfaces;
	mod->firstBrush = numbrushes;

	EmitBrushes( e->brushes );
}

void EmitPortals_r(node_t * node, node_t *headnode)
{
	bspbrush_t        *b;
	int i,s;
	portal_t *p;
	portal_t *ap;
	winding_t *pw;
	vec3_t checkPoint;
	plane_t *apPlane;

	if(node->planenum != PLANENUM_LEAF)
	{
		EmitPortals_r(node->children[0],headnode);
		EmitPortals_r(node->children[1],headnode);
		return;
	}

	if(node->opaque)
		return;

	if(node->cluster != -1)
		if(node->area == -1)
			printf("EmitPortals_r: cluster %d has area set to -1\n", node->cluster);
	if(node->areaportal)
	{
		b = node->brushlist->original;

		// check if the areaportal touches two areas
		if(b->portalareas[0] == -1 || b->portalareas[1] == -1) {
			printf("EmitPortals_r: areaportal brush %i doesn't touch two areas\n", b->brushnum);
			return;
		}

#if 0
		// it doesnt work, the ap winding does not match with real portal winding
		ap = 0;
		for(p = node->portals; p; p = p->next[s]) {
			s = (p->nodes[1] == node);

			if(p->onnode == node->parent) {
				if(ap) {
					// this should never happen,
					// there cant be two different portals on single node?
					printf("Areaportal already found!\n");
				}
				ap = p;
			}
		}
		if(ap) {
			pw = ap->winding;
			apPlane = &ap->plane;
#else
		pw = 0;
		apPlane = 0;
		// let's assume that areaPortal brushes has only one areaPortal side
		for(i = 0; i < b->numsides; i++) {
			side_t *s = &b->sides[i];
			if((s->contents & CONTENTS_AREAPORTAL) == qfalse) {
				continue;
			}
			pw = s->winding;
			apPlane = &mapplanes[s->planenum];
			if(pw == 0) {
				pw = s->visibleHull;
			}
		}
		{
#endif
			if(pw == 0) {
				printf("EmitPortals_r: areaPortal %i has null winding\n",0);
				return;
			}
			if(numDPoints + pw->numpoints >= MAX_MAP_POINTS) {
				Error("MAX_MAP_POINTS (increase this macro and recompile or remove some areaportals\n");
				return;
			}
			if(numAreaPortals + 1 >= MAX_MAP_AREAPORTALS) {
				Error("MAX_MAP_AREAPORTALS (increase this macro and recompile or remove some areaportals\n");
				return;
			}
			printf("EmitPortals_r: emiting portal %i\n",0);
			ClearBounds(dareaPortals[numAreaPortals].bounds[1],dareaPortals[numAreaPortals].bounds[0]);
			for(i = 0; i < pw->numpoints; i++) {
				VectorCopy(pw->p[i],dpoints[numDPoints+i]);
				AddPointToBounds(pw->p[i],dareaPortals[numAreaPortals].bounds[1],dareaPortals[numAreaPortals].bounds[0]);
			}		
			// extend areaPortal bounds a little, 
			// otherwise there might be some diseapperaing 
			// areas while camera eye is inside portal winding
			for(i = 0; i < 3; i++) {
				const float pEps = 1.f;
				dareaPortals[numAreaPortals].bounds[0][i] += pEps;
				dareaPortals[numAreaPortals].bounds[1][i] -= pEps;
			}
#if 1
			WindingCenter(pw,checkPoint);
			VectorMA(checkPoint,1,apPlane->normal,checkPoint);
			node = NodeForPoint(headnode,checkPoint);
#else
			
#endif
			if(node->area == b->portalareas[0]){
				dareaPortals[numAreaPortals].areas[0] = b->portalareas[0];
				dareaPortals[numAreaPortals].areas[1] = b->portalareas[1];
			} else if(node->area = b->portalareas[1]) {
				dareaPortals[numAreaPortals].areas[0] = b->portalareas[1];
				dareaPortals[numAreaPortals].areas[1] = b->portalareas[0];
			} else {
				Error("Node area mismatch\n");
			}
			dareaPortals[numAreaPortals].planeNum = FindFloatPlane(apPlane->normal,apPlane->dist);
			dareaPortals[numAreaPortals].firstPoint = numDPoints;
			dareaPortals[numAreaPortals].numPoints = pw->numpoints;
			numAreaPortals ++;
			numDPoints += pw->numpoints;
			return;
		}
		printf("EmitPortals: no matching portal found!\n");
	}
}

static void EmitPortals (node_t *headnode) {
	if (nummodels)
		return;		// only for world model

	EmitPortals_r(headnode,headnode);
}

/*
==================
EndModel
==================
*/
void EndModel( node_t *headnode ) {
	dmodel_t	*mod;

	qprintf ("--- EndModel ---\n");

	mod = &dmodels[nummodels];
	EmitDrawNode_r (headnode);
	// Vodin: areaPortal points, areas and planes (Doom3 style)
	EmitPortals(headnode);

	mod->numSurfaces = numDrawSurfaces - mod->firstSurface;
	mod->numBrushes = numbrushes - mod->firstBrush;

	nummodels++;
}

