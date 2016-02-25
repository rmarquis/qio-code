/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 
Copyright (C) 2013 V. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/
// doom3ProcPVSClass.cpp - realtime visibility checks for Doom3 .proc files
// (note that .proc files dont have PVS precomputed!)
// Based upon idPVS class.

#include "doom3ProcPVSClass.h"
#include <math/aabb.h>
#include <math/plane.h>
#include <shared/cmWinding.h>
#include <api/coreAPI.h>
#include <api/portalizedWorldAPI.h>

#define	D3_PVS_ON_EPSILON		0.1f

#define MAX_BOUNDS_AREAS	128 //16

// TODO: move it somewhere else?
static const int NUM_PORTAL_ATTRIBUTES = 3;

typedef enum {
	PS_BLOCK_NONE = 0,

	PS_BLOCK_VIEW = 1,
	PS_BLOCK_LOCATION = 2,		// game map location strings often stop in hallways
	PS_BLOCK_AIR = 4,			// windows between pressurized and unpresurized areas

	PS_BLOCK_ALL = (1<<NUM_PORTAL_ATTRIBUTES)-1
} portalConnection_t;

typedef struct pvsPassage_s {
	byte *				canSee;		// bit set for all portals that can be seen through this passage
} pvsPassage_t;


typedef struct pvsPortal_s {
	int					areaNum;	// area this portal leads to
	cmWinding_c *			w;			// winding goes counter clockwise seen from the area this portal is part of
	aabb			bounds;		// winding bounds
	plane_c				plane;		// winding plane, normal points towards the area this portal leads to
	pvsPassage_t *		passages;	// passages to portals in the area this portal leads to
	bool				done;		// true if pvs is calculated for this portal
	byte *				vis;		// PVS for this portal
	byte *				mightSee;	// used during construction
} pvsPortal_t;


typedef struct pvsArea_s {
	int					numPortals;	// number of portals in this area
	aabb			bounds;		// bounds of the whole area
	pvsPortal_t **		portals;	// array with pointers to the portals of this area
} pvsArea_t;


typedef struct pvsStack_s {
	struct pvsStack_s *	next;		// next stack entry
	byte *				mightSee;	// bit set for all portals that might be visible through this passage/portal stack
} pvsStack_t;


/*
================
idPVS::idPVS
================
*/
idPVS::idPVS() {
	int i;

	portalWorld = 0;

	numAreas = 0;
	numPortals = 0;

	connectedAreas = NULL;
	areaQueue = NULL;
	areaPVS = NULL;

	for ( i = 0; i < MAX_CURRENT_PVS; i++ ) {
		currentPVS[i].handle.i = -1;
		currentPVS[i].handle.h = 0;
		currentPVS[i].pvs = NULL;
	}

	pvsAreas = NULL;
	pvsPortals = NULL;
}

/*
================
idPVS::~idPVS
================
*/
idPVS::~idPVS() {
	Shutdown();
}

/*
================
idPVS::GetPortalCount
================
*/
int idPVS::GetPortalCount() const {
	int i, na, np;

	na = portalWorld->getNumAreas();
	np = 0;
	for ( i = 0; i < na; i++ ) {
		np += portalWorld->getNumPortalsInArea( i );
	}
	return np;
}

/*
================
idPVS::CreatePVSData
================
*/
void idPVS::CreatePVSData() {
	int i, j, n, cp;
	const portalAPI_i *portal;
	pvsArea_t *area;
	pvsPortal_t *p, **portalPtrs;

	if ( !numPortals ) {
		return;
	}

	pvsPortals = new pvsPortal_t[numPortals];
	pvsAreas = new pvsArea_t[numAreas];
	memset( pvsAreas, 0, numAreas * sizeof( *pvsAreas ) );

	cp = 0;
	portalPtrs = new pvsPortal_t*[numPortals];

	for ( i = 0; i < numAreas; i++ ) {

		area = &pvsAreas[i];
		area->bounds.clear();
		area->portals = portalPtrs + cp;

		n = portalWorld->getNumPortalsInArea( i );

		for ( j = 0; j < n; j++ ) {

			portal = portalWorld->getPortal( i, j );

			p = &pvsPortals[cp++];
			// the winding goes counter clockwise seen from this area
			p->w = portal->getWindingP()->copy();
			// V: reverse the winding for the second area.
			// Our portals are not doubled like in D3.
			if(i != portal->getBackArea()) {
				*p->w = p->w->getReversed();
			}
			p->areaNum = portal->getOtherAreaNum(i);
			assert(p->areaNum != i);

			p->vis = new byte[portalVisBytes];
			memset( p->vis, 0, portalVisBytes );
			p->mightSee = new byte[portalVisBytes];
			memset( p->mightSee, 0, portalVisBytes );
			p->w->getBounds( p->bounds );
			p->w->getPlane( p->plane );
			// plane normal points to outside the area
			p->plane = p->plane.getOpposite();
			// no PVS calculated for this portal yet
			p->done = false;

			area->portals[area->numPortals] = p;
			area->numPortals++;

			area->bounds.addBox(p->bounds);
		}
	}
}

/*
================
idPVS::DestroyPVSData
================
*/
void idPVS::DestroyPVSData() {
	int i;

	if ( !pvsAreas ) {
		return;
	}

	// delete portal pointer array
	delete[] pvsAreas[0].portals;

	// delete all areas
	delete[] pvsAreas;
	pvsAreas = NULL;

	// delete portal data
	for ( i = 0; i < numPortals; i++ ) {
		delete[] pvsPortals[i].vis;
		delete[] pvsPortals[i].mightSee;
		delete pvsPortals[i].w;
	}

	// delete portals
	delete[] pvsPortals;
	pvsPortals = NULL;
}

/*
================
idPVS::FloodFrontPortalPVS_r
================
*/
void idPVS::FloodFrontPortalPVS_r( pvsPortal_t *portal, int areaNum ) const {
	int i, n;
	pvsArea_t *area;
	pvsPortal_t *p;

	area = &pvsAreas[ areaNum ];

	for ( i = 0; i < area->numPortals; i++ ) {
		p = area->portals[i];
		n = p - pvsPortals;
		// don't flood through if this portal is not at the front
		if ( !( portal->mightSee[ n>>3 ] & (1 << (n&7)) ) ) {
			continue;
		}
		// don't flood through if already visited this portal
		if ( portal->vis[ n>>3 ] & (1 << (n&7)) ) {
			continue;
		}
		// this portal might be visible
		portal->vis[ n>>3 ] |= (1 << (n&7));
		// flood through the portal
		FloodFrontPortalPVS_r( portal, p->areaNum );
	}
}

/*
================
idPVS::FrontPortalPVS
================
*/
void idPVS::FrontPortalPVS() const {
	int i, j, k, n, p, side1, side2, areaSide;
	pvsPortal_t *p1, *p2;
	pvsArea_t *area;

	for ( i = 0; i < numPortals; i++ ) {
		p1 = &pvsPortals[i];

		for ( j = 0; j < numAreas; j++ ) {

			area = &pvsAreas[j];

			areaSide = side1 = p1->plane.onSide(area->bounds);

			// if the whole area is at the back side of the portal
			if ( areaSide == SIDE_BACK ) {
				continue;
			}

			for ( p = 0; p < area->numPortals; p++ ) {
	
				p2 = area->portals[p];

				// if we the whole area is not at the front we need to check
				if ( areaSide != SIDE_FRONT ) {
					// if the second portal is completely at the back side of the first portal
					side1 = p1->plane.onSide(p2->bounds);
					if ( side1 == SIDE_BACK ) {
						continue;
					}
				}

				// if the first portal is completely at the front of the second portal
				side2 = p2->plane.onSide(p1->bounds);
				if ( side2 == SIDE_FRONT ) {
					continue;
				}

				// if the second portal is not completely at the front of the first portal
				if ( side1 != SIDE_FRONT ) {
					// more accurate check
					for ( k = 0; k < p2->w->size(); k++ ) {
						// if more than an epsilon at the front side
						if ( p1->plane.onSide( (*p2->w)[k], D3_PVS_ON_EPSILON ) == SIDE_FRONT ) {
							break;
						}
					}
					if ( k >= p2->w->size() ) {
						continue;	// second portal is at the back of the first portal
					}
				}

				// if the first portal is not completely at the back side of the second portal
				if ( side2 != SIDE_BACK ) {
					// more accurate check
					for ( k = 0; k < p1->w->size(); k++ ) {
						// if more than an epsilon at the back side
						if ( p2->plane.onSide( (*p1->w)[k], D3_PVS_ON_EPSILON ) == SIDE_BACK ) {
							break;
						}
					}
					if ( k >= p1->w->size() ) {
						continue;	// first portal is at the front of the second portal
					}
				}

				// the portal might be visible at the front
				n = p2 - pvsPortals;
				p1->mightSee[ n >> 3 ] |= 1 << (n&7);
			}
		}
	}

	// flood the front portal pvs for all portals
	for ( i = 0; i < numPortals; i++ ) {
		p1 = &pvsPortals[i];
		FloodFrontPortalPVS_r( p1, p1->areaNum );
	}
}

/*
===============
idPVS::FloodPassagePVS_r
===============
*/
pvsStack_t *idPVS::FloodPassagePVS_r( pvsPortal_t *source, const pvsPortal_t *portal, pvsStack_t *prevStack ) const {
	int i, j, n, m;
	pvsPortal_t *p;
	pvsArea_t *area;
	pvsStack_t *stack;
	pvsPassage_t *passage;
	long *sourceVis, *passageVis, *portalVis, *mightSee, *prevMightSee, more;

	area = &pvsAreas[portal->areaNum];

	stack = prevStack->next;
	// if no next stack entry allocated
	if ( !stack ) {
		stack = reinterpret_cast<pvsStack_t*>(new byte[sizeof(pvsStack_t) + portalVisBytes]);
		stack->mightSee = (reinterpret_cast<byte *>(stack)) + sizeof(pvsStack_t);
		stack->next = NULL;
		prevStack->next = stack;
	}

	// check all portals for flooding into other areas
	for ( i = 0; i < area->numPortals; i++ ) {

		passage = &portal->passages[i];

		// if this passage is completely empty
		if ( !passage->canSee ) {
			continue;
		}

		p = area->portals[i];
		n = p - pvsPortals;

		// if this portal cannot be seen through our current portal/passage stack
		if ( !( prevStack->mightSee[n >> 3] & (1 << (n & 7)) ) ) {
			continue;
		}

		// mark the portal as visible
		source->vis[n >> 3] |= (1 << (n & 7));

		// get pointers to vis data
		prevMightSee = reinterpret_cast<long *>(prevStack->mightSee);
		passageVis = reinterpret_cast<long *>(passage->canSee);
		sourceVis = reinterpret_cast<long *>(source->vis);
		mightSee = reinterpret_cast<long *>(stack->mightSee);

		more = 0;
		// use the portal PVS if it has been calculated
		if ( p->done ) {
			portalVis = reinterpret_cast<long *>(p->vis);
			for ( j = 0; j < portalVisLongs; j++ ) {
				// get new PVS which is decreased by going through this passage
				m = *prevMightSee++ & *passageVis++ & *portalVis++;
				// check if anything might be visible through this passage that wasn't yet visible
				more |= (m & ~(*sourceVis++));
				// store new PVS
				*mightSee++ = m;
			}
		}
		else {
			// the p->mightSee is implicitely stored in the passageVis
			for ( j = 0; j < portalVisLongs; j++ ) {
				// get new PVS which is decreased by going through this passage
				m = *prevMightSee++ & *passageVis++;
				// check if anything might be visible through this passage that wasn't yet visible
				more |= (m & ~(*sourceVis++));
				// store new PVS
				*mightSee++ = m;
			}
		}

		// if nothing more can be seen
		if ( !more ) {
			continue;
		}

		// go through the portal
		stack->next = FloodPassagePVS_r( source, p, stack );
	}

	return stack;
}

/*
===============
idPVS::PassagePVS
===============
*/
void idPVS::PassagePVS() const {
	int i;
	pvsPortal_t *source;
	pvsStack_t *stack, *s;

	// create the passages
	CreatePassages();

	// allocate first stack entry
	stack = reinterpret_cast<pvsStack_t*>(new byte[sizeof(pvsStack_t) + portalVisBytes]);
	stack->mightSee = (reinterpret_cast<byte *>(stack)) + sizeof(pvsStack_t);
	stack->next = NULL;

	// calculate portal PVS by flooding through the passages
	for ( i = 0; i < numPortals; i++ ) {
		source = &pvsPortals[i];
		memset( source->vis, 0, portalVisBytes );
		memcpy( stack->mightSee, source->mightSee, portalVisBytes );
		FloodPassagePVS_r( source, source, stack );
		source->done = true;
	}

	// free the allocated stack
	for ( s = stack; s; s = stack ) {
		stack = stack->next;
		delete[] s;
	}

	// destroy the passages
	DestroyPassages();
}

/*
===============
idPVS::AddPassageBoundaries
===============
*/
void idPVS::AddPassageBoundaries( const cmWinding_c &source, const cmWinding_c &pass, bool flipClip, plane_c *bounds, int &numBounds, int maxBounds ) const {
	int			i, j, k, l;
	vec3_c		v1, v2, normal;
	float		d, dist;
	bool		flipTest, front;
	plane_c		plane;


	// check all combinations	
	for ( i = 0; i < source.size(); i++ ) {

		l = (i + 1) % source.size();
		v1 = source[l] - source[i];

		// find a vertex of pass that makes a plane that puts all of the
		// vertices of pass on the front side and all of the vertices of
		// source on the back side
		for ( j = 0; j < pass.size(); j++ ) {

			v2 = pass[j] - source[i];

			normal = v1.crossProduct( v2 );
			if ( normal.normalize2() < 0.01f ) {
				continue;
			}
			dist = normal.dotProduct(pass[j]);

			//
			// find out which side of the generated seperating plane has the
			// source portal
			//
			flipTest = false;
			for ( k = 0; k < source.size(); k++ ) {
				if ( k == i || k == l ) {
					continue;
				}
				d = source[k].dotProduct(normal) - dist;
				if ( d < -D3_PVS_ON_EPSILON ) {
					// source is on the negative side, so we want all
					// pass and target on the positive side
					flipTest = false;
					break;
				}
				else if ( d > D3_PVS_ON_EPSILON ) {
					// source is on the positive side, so we want all
					// pass and target on the negative side
					flipTest = true;
					break;
				}
			}
			if ( k == source.size() ) {
				continue;		// planar with source portal
			}

			// flip the normal if the source portal is backwards
			if (flipTest) {
				normal = -normal;
				dist = -dist;
			}

			// if all of the pass portal points are now on the positive side,
			// this is the seperating plane
			front = false;
			for ( k = 0; k < pass.size(); k++ ) {
				if ( k == j ) {
					continue;
				}
				d = pass[k].dotProduct(normal) - dist;
				if ( d < -D3_PVS_ON_EPSILON ) {
					break;
				}
				else if ( d > D3_PVS_ON_EPSILON ) {
					front = true;
				}
			}
			if ( k < pass.size() ) {
				continue;	// points on negative side, not a seperating plane
			}
			if ( !front ) {
				continue;	// planar with seperating plane
			}

			// flip the normal if we want the back side
			if ( flipClip ) {
				plane.setNormal( -normal );
				plane.setDist( dist );
			}
			else {
				plane.setNormal( normal );
				plane.setDist( -dist );
			}

			// check if the plane is already a passage boundary
			for ( k = 0; k < numBounds; k++ ) {
				if ( plane.compare( bounds[k], 0.001f, 0.01f ) ) {
					break;
				}
			}
			if ( k < numBounds ) {
				break;
			}

			if ( numBounds >= maxBounds ) {
				g_core->RedWarning( "max passage boundaries." );
				break;
			}
			bounds[numBounds] = plane;
			numBounds++;
			break;
		}
	}
}

/*
================
idPVS::CreatePassages
================
*/
#define MAX_PASSAGE_BOUNDS		128

void idPVS::CreatePassages() const {
	int i, j, l, n, numBounds, front, passageMemory, byteNum, bitNum;
	int sides[MAX_PASSAGE_BOUNDS];
	plane_c passageBounds[MAX_PASSAGE_BOUNDS];
	pvsPortal_t *source, *target, *p;
	pvsArea_t *area;
	pvsPassage_t *passage;
	cmWinding_c winding;
	byte canSee, mightSee, bit;

	passageMemory = 0;
	for ( i = 0; i < numPortals; i++ ) {
		source = &pvsPortals[i];
		area = &pvsAreas[source->areaNum];

		source->passages = new pvsPassage_t[area->numPortals];

		for ( j = 0; j < area->numPortals; j++ ) {
			target = area->portals[j];
			n = target - pvsPortals;

			passage = &source->passages[j];

			// if the source portal cannot see this portal
			if ( !( source->mightSee[ n>>3 ] & (1 << (n&7)) ) ) {
				// not all portals in the area have to be visible because areas are not necesarily convex
				// also no passage has to be created for the portal which is the opposite of the source
				passage->canSee = NULL;
				continue;
			}

			passage->canSee = new byte[portalVisBytes];
			passageMemory += portalVisBytes;

			// boundary plane normals point inwards
			numBounds = 0;
			AddPassageBoundaries( *(source->w), *(target->w), false, passageBounds, numBounds, MAX_PASSAGE_BOUNDS );
			AddPassageBoundaries( *(target->w), *(source->w), true, passageBounds, numBounds, MAX_PASSAGE_BOUNDS );

			// get all portals visible through this passage
			for ( byteNum = 0; byteNum < portalVisBytes; byteNum++) {

				canSee = 0;
				mightSee = source->mightSee[byteNum] & target->mightSee[byteNum];

				// go through eight portals at a time to speed things up
				for ( bitNum = 0; bitNum < 8; bitNum++ ) {

					bit = 1 << bitNum;

					if ( !( mightSee & bit ) ) {
						continue;
					}

					p = &pvsPortals[(byteNum << 3) + bitNum];
	
					if ( p->areaNum == source->areaNum ) {
						continue;
					}

					for ( front = 0, l = 0; l < numBounds; l++ ) {
						sides[l] = passageBounds[l].onSide(p->bounds);
						// if completely at the back of the passage bounding plane
						if ( sides[l] == SIDE_BACK ) {
							break;
						}
						// if completely at the front
						if ( sides[l] == SIDE_FRONT ) {
							front++;
						}
					}
					// if completely outside the passage
					if ( l < numBounds ) {
						continue;
					}

					// if not at the front of all bounding planes and thus not completely inside the passage
					if ( front != numBounds ) {

						winding = *p->w;

						for ( l = 0; l < numBounds; l++ ) {
							// only clip if the winding possibly crosses this plane
							if ( sides[l] != SIDE_CROSS ) {
								continue;
							}
							// clip away the part at the back of the bounding plane
							winding.clipWindingByPlane( passageBounds[l] );
							// if completely clipped away
							if ( !winding.size() ) {
								break;
							}
						}
						// if completely outside the passage
						if ( l < numBounds ) {
							continue;
						}
					}

					canSee |= bit;
				}

				// store results of all eight portals
				passage->canSee[byteNum] = canSee;
			}

			// can always see the target portal
			passage->canSee[n >> 3] |= (1 << (n&7));
		}
	}
	if ( passageMemory < 1024 ) {
		g_core->Print( "%5d bytes passage memory used to build PVS\n", passageMemory );
	}
	else {
		g_core->Print( "%5d KB passage memory used to build PVS\n", passageMemory>>10 );
	}
}

/*
================
idPVS::DestroyPassages
================
*/
void idPVS::DestroyPassages() const {
	int i, j;
	pvsPortal_t *p;
	pvsArea_t *area;

	for ( i = 0; i < numPortals; i++ ) {
		p = &pvsPortals[i];
		area = &pvsAreas[p->areaNum];
		for ( j = 0; j < area->numPortals; j++ ) {
			if ( p->passages[j].canSee ) {
				delete[] p->passages[j].canSee;
			}
		}
		delete[] p->passages;
	}
}

/*
================
idPVS::CopyPortalPVSToMightSee
================
*/
void idPVS::CopyPortalPVSToMightSee() const {
	int i;
	pvsPortal_t *p;

	for ( i = 0; i < numPortals; i++ ) {
		p = &pvsPortals[i];
		memcpy( p->mightSee, p->vis, portalVisBytes );
	}
}

/*
================
idPVS::AreaPVSFromPortalPVS
================
*/
int idPVS::AreaPVSFromPortalPVS() const {
	int i, j, k, areaNum, totalVisibleAreas;
	long *p1, *p2;
	byte *pvs, *portalPVS;
	pvsArea_t *area;

	totalVisibleAreas = 0;

	if ( !numPortals ) {
		return totalVisibleAreas;
	}

	memset( areaPVS, 0, numAreas * areaVisBytes );

	for ( i = 0; i < numAreas; i++ ) {
		area = &pvsAreas[i];
		pvs = areaPVS + i * areaVisBytes;

		// the area is visible to itself
		pvs[ i >> 3 ] |= 1 << (i & 7);

		if ( !area->numPortals ) {
			continue;
		}

		// store the PVS of all portals in this area at the first portal
		for ( j = 1; j < area->numPortals; j++ ) {
			p1 = reinterpret_cast<long *>(area->portals[0]->vis);
			p2 = reinterpret_cast<long *>(area->portals[j]->vis);
			for ( k = 0; k < portalVisLongs; k++ ) {
				*p1++ |= *p2++;
			}
		}

		// the portals of this area are always visible
		for ( j = 0; j < area->numPortals; j++ ) {
			k = area->portals[j] - pvsPortals;
			area->portals[0]->vis[ k >> 3 ] |= 1 << (k & 7);
		}

		// set all areas to visible that can be seen from the portals of this area
		portalPVS = area->portals[0]->vis;
		for ( j = 0; j < numPortals; j++ ) {
			// if this portal is visible
			if ( portalPVS[j>>3] & (1 << (j&7)) ) {
				areaNum = pvsPortals[j].areaNum;
				pvs[ areaNum >> 3 ] |= 1 << (areaNum & 7);
			}
		}

		// count the number of visible areas
		for ( j = 0; j < numAreas; j++ ) {
			if ( pvs[j>>3] & (1 << (j&7)) ) {
				totalVisibleAreas++;
			}
		}
	}
	return totalVisibleAreas;
}

/*
================
idPVS::Init
================
*/
void idPVS::Init(const class portalizedWorldAPI_i *newPortalWorld) {
	int totalVisibleAreas;

	Shutdown();

	portalWorld = newPortalWorld;

	numAreas = portalWorld->getNumAreas();
	if ( numAreas <= 0 ) {
		return;
	}

	connectedAreas = new bool[numAreas];
	areaQueue = new int[numAreas];

	areaVisBytes = ( ((numAreas+31)&~31) >> 3);
	areaVisLongs = areaVisBytes/sizeof(long);

	areaPVS = new byte[numAreas * areaVisBytes];
	memset( areaPVS, 0xFF, numAreas * areaVisBytes );

	numPortals = GetPortalCount();

	portalVisBytes = ( ((numPortals+31)&~31) >> 3);
	portalVisLongs = portalVisBytes/sizeof(long);

	for ( int i = 0; i < MAX_CURRENT_PVS; i++ ) {
		currentPVS[i].handle.i = -1;
		currentPVS[i].handle.h = 0;
		currentPVS[i].pvs = new byte[areaVisBytes];
		memset( currentPVS[i].pvs, 0, areaVisBytes );
	}

	//idTimer timer;
	//timer.Start();

	CreatePVSData();

	FrontPortalPVS();

	CopyPortalPVSToMightSee();

	PassagePVS();

	totalVisibleAreas = AreaPVSFromPortalPVS();

	DestroyPVSData();

//	timer.Stop();

//	g_core->Print( "%5.0f msec to calculate PVS\n", timer.Milliseconds() );
	g_core->Print( "%5d areas\n", numAreas );
	g_core->Print( "%5d portals\n", numPortals );
	g_core->Print( "%5d areas visible on average\n", totalVisibleAreas / numAreas );
	if ( numAreas * areaVisBytes < 1024 ) {
		g_core->Print( "%5d bytes PVS data\n", numAreas * areaVisBytes );
	}
	else {
		g_core->Print( "%5d KB PVS data\n", (numAreas * areaVisBytes) >> 10 );
	}
}

/*
================
idPVS::Shutdown
================
*/
void idPVS::Shutdown() {
	if ( connectedAreas ) {
		delete connectedAreas;
		connectedAreas = NULL;
	}
	if ( areaQueue ) {
		delete areaQueue;
		areaQueue = NULL;
	}
	if ( areaPVS ) {
		delete areaPVS;
		areaPVS = NULL;
	}
	if ( currentPVS ) {
		for ( int i = 0; i < MAX_CURRENT_PVS; i++ ) {
			delete currentPVS[i].pvs;
			currentPVS[i].pvs = NULL;
		}
	}
}

/*
================
idPVS::GetConnectedAreas

  assumes the 'areas' array is initialized to false
================
*/
void idPVS::GetConnectedAreas( int srcArea, bool *areas ) const {
	int curArea, nextArea;
	int queueStart, queueEnd;
	int i, n;
	const portalAPI_i *portal;

	queueStart = -1;
	queueEnd = 0;
	areas[srcArea] = true;

	for ( curArea = srcArea; queueStart < queueEnd; curArea = areaQueue[++queueStart] ) {

		n = portalWorld->getNumPortalsInArea( curArea );

		for ( i = 0; i < n; i++ ) {
			portal = portalWorld->getPortal( curArea, i );

			if ( portal->getBlockingBits() & PS_BLOCK_VIEW ) {
				continue;
			}

			nextArea = portal->getOtherAreaNum(curArea);
			assert(curArea != nextArea);


			// if already visited this area
			if ( areas[nextArea] ) {
				continue;
			}

			// add area to queue
			areaQueue[queueEnd++] = nextArea;
			areas[nextArea] = true;
		}
	}
}

/*
================
idPVS::GetPVSArea
================
*/
int idPVS::GetPVSArea( const vec3_c &point ) const {
	return portalWorld->pointAreaNum( point );
}

/*
================
idPVS::GetPVSAreas
================
*/
int idPVS::GetPVSAreas( const aabb &bounds, int *areas, int maxAreas ) const {
	return portalWorld->boxAreaNums( bounds, areas, maxAreas );
}

/*
================
idPVS::SetupCurrentPVS
================
*/
pvsHandle_t idPVS::SetupCurrentPVS( const vec3_c &source, const pvsType_t type ) const {
	int sourceArea;

	sourceArea = portalWorld->pointAreaNum( source );

	return SetupCurrentPVS( sourceArea, type );
}

/*
================
idPVS::SetupCurrentPVS
================
*/
pvsHandle_t idPVS::SetupCurrentPVS( const aabb &source, const pvsType_t type ) const {
	int numSourceAreas, sourceAreas[MAX_BOUNDS_AREAS];

	numSourceAreas = portalWorld->boxAreaNums( source, sourceAreas, MAX_BOUNDS_AREAS );

	return SetupCurrentPVS( sourceAreas, numSourceAreas, type );
}

/*
================
idPVS::SetupCurrentPVS
================
*/
pvsHandle_t idPVS::SetupCurrentPVS( const int sourceArea, const pvsType_t type ) const {
	int i;
	pvsHandle_t handle;

	handle = AllocCurrentPVS( *reinterpret_cast<const unsigned int *>(&sourceArea) );

	if ( sourceArea < 0 || sourceArea >= numAreas ) {
		memset( currentPVS[handle.i].pvs, 0, areaVisBytes );
		return handle;
	}

	if ( type != PVS_CONNECTED_AREAS ) {
		memcpy( currentPVS[handle.i].pvs, areaPVS + sourceArea * areaVisBytes, areaVisBytes );
	} else {
		memset( currentPVS[handle.i].pvs, -1, areaVisBytes );
	}

	if ( type == PVS_ALL_PORTALS_OPEN ) {
		return handle;
	}

	memset( connectedAreas, 0, numAreas * sizeof( *connectedAreas ) );

	GetConnectedAreas( sourceArea, connectedAreas );

	for ( i = 0; i < numAreas; i++ ) {
		if ( !connectedAreas[i] ) {
			currentPVS[handle.i].pvs[i>>3] &= ~(1 << (i&7));
		}
	}

	return handle;
}

/*
================
idPVS::SetupCurrentPVS
================
*/
pvsHandle_t idPVS::SetupCurrentPVS( const int *sourceAreas, const int numSourceAreas, const pvsType_t type ) const {
	int i, j;
	unsigned int h;
	long *vis, *pvs;
	pvsHandle_t handle;

	h = 0;
	for ( i = 0; i < numSourceAreas; i++ ) {
		h ^= *reinterpret_cast<const unsigned int *>(&sourceAreas[i]);
	}
	handle = AllocCurrentPVS( h );

	if ( !numSourceAreas || sourceAreas[0] < 0 || sourceAreas[0] >= numAreas) {
		memset( currentPVS[handle.i].pvs, 0, areaVisBytes );
		return handle;
	}

	if ( type != PVS_CONNECTED_AREAS ) {
		// merge PVS of all areas the source is in
		memcpy( currentPVS[handle.i].pvs, areaPVS + sourceAreas[0] * areaVisBytes, areaVisBytes );
		for ( i = 1; i < numSourceAreas; i++ ) {

			assert( sourceAreas[i] >= 0 && sourceAreas[i] < numAreas );

			vis = reinterpret_cast<long*>(areaPVS + sourceAreas[i] * areaVisBytes);
			pvs = reinterpret_cast<long*>(currentPVS[handle.i].pvs);
			for ( j = 0; j < areaVisLongs; j++ ) {
				*pvs++ |= *vis++;
			}
		}
	} else {
		memset( currentPVS[handle.i].pvs, -1, areaVisBytes );
	}

	if ( type == PVS_ALL_PORTALS_OPEN ) {
		return handle;
	}

	memset( connectedAreas, 0, numAreas * sizeof( *connectedAreas ) );

	// get all areas connected to any of the source areas
	for ( i = 0; i < numSourceAreas; i++ ) {
		if ( !connectedAreas[sourceAreas[i]] ) {
			GetConnectedAreas( sourceAreas[i], connectedAreas );
		}
	}

	// remove unconnected areas from the PVS
	for ( i = 0; i < numAreas; i++ ) {
		if ( !connectedAreas[i] ) {
			currentPVS[handle.i].pvs[i>>3] &= ~(1 << (i&7));
		}
	}

	return handle;
}

/*
================
idPVS::MergeCurrentPVS
================
*/
pvsHandle_t idPVS::MergeCurrentPVS( pvsHandle_t pvs1, pvsHandle_t pvs2 ) const {
	int i;
	long *pvs1Ptr, *pvs2Ptr, *ptr;
	pvsHandle_t handle;

	if ( pvs1.i < 0 || pvs1.i >= MAX_CURRENT_PVS || pvs1.h != currentPVS[pvs1.i].handle.h ||
		pvs2.i < 0 || pvs2.i >= MAX_CURRENT_PVS || pvs2.h != currentPVS[pvs2.i].handle.h ) {
		g_core->DropError( "idPVS::MergeCurrentPVS: invalid handle" );
	}

	handle = AllocCurrentPVS( pvs1.h ^ pvs2.h );

	ptr = reinterpret_cast<long*>(currentPVS[handle.i].pvs);
	pvs1Ptr = reinterpret_cast<long*>(currentPVS[pvs1.i].pvs);
	pvs2Ptr = reinterpret_cast<long*>(currentPVS[pvs2.i].pvs);

	for ( i = 0; i < areaVisLongs; i++ ) {
		*ptr++ = *pvs1Ptr++ | *pvs2Ptr++;
	}

	return handle;
}

/*
================
idPVS::AllocCurrentPVS
================
*/
pvsHandle_t idPVS::AllocCurrentPVS( unsigned int h ) const {
	int i;
	pvsHandle_t handle;

	for ( i = 0; i < MAX_CURRENT_PVS; i++ ) {
		if ( currentPVS[i].handle.i == -1 ) {
			currentPVS[i].handle.i = i;
			currentPVS[i].handle.h = h;
			return currentPVS[i].handle;
		}
	}

	g_core->DropError( "idPVS::AllocCurrentPVS: no free PVS left" );

	handle.i = -1;
	handle.h = 0;
	return handle;
}

/*
================
idPVS::FreeCurrentPVS
================
*/
void idPVS::FreeCurrentPVS( pvsHandle_t handle ) const {
	if ( handle.i < 0 || handle.i >= MAX_CURRENT_PVS || handle.h != currentPVS[handle.i].handle.h ) {
		g_core->DropError( "idPVS::FreeCurrentPVS: invalid handle" );
	}
	currentPVS[handle.i].handle.i = -1;
}

/*
================
idPVS::InCurrentPVS
================
*/
bool idPVS::InCurrentPVS( const pvsHandle_t handle, const vec3_c &target ) const {
	int targetArea;

	if ( handle.i < 0 || handle.i >= MAX_CURRENT_PVS ||
		handle.h != currentPVS[handle.i].handle.h ) {
		g_core->DropError( "idPVS::InCurrentPVS: invalid handle" );
	}

	targetArea = portalWorld->pointAreaNum( target );

	if ( targetArea == -1 ) {
		return false;
	}

	return ( ( currentPVS[handle.i].pvs[targetArea>>3] & (1 << (targetArea&7)) ) != 0 );
}

/*
================
idPVS::InCurrentPVS
================
*/
bool idPVS::InCurrentPVS( const pvsHandle_t handle, const aabb &target ) const {
	int i, numTargetAreas, targetAreas[MAX_BOUNDS_AREAS];

	if ( handle.i < 0 || handle.i >= MAX_CURRENT_PVS ||
		handle.h != currentPVS[handle.i].handle.h ) {
		g_core->DropError( "idPVS::InCurrentPVS: invalid handle" );
	}

	numTargetAreas = portalWorld->boxAreaNums( target, targetAreas, MAX_BOUNDS_AREAS );

	for ( i = 0; i < numTargetAreas; i++ ) {
		if ( currentPVS[handle.i].pvs[targetAreas[i]>>3] & (1 << (targetAreas[i]&7)) ) {
			return true;
		}
	}
	return false;
}

/*
================
idPVS::InCurrentPVS
================
*/
bool idPVS::InCurrentPVS( const pvsHandle_t handle, const int targetArea ) const {

	if ( handle.i < 0 || handle.i >= MAX_CURRENT_PVS ||
		handle.h != currentPVS[handle.i].handle.h ) {
		g_core->DropError( "idPVS::InCurrentPVS: invalid handle" );
	}

	if ( targetArea < 0 || targetArea >= numAreas ) {
		return false;
	}

	return ( ( currentPVS[handle.i].pvs[targetArea>>3] & (1 << (targetArea&7)) ) != 0 );
}

/*
================
idPVS::InCurrentPVS
================
*/
bool idPVS::InCurrentPVS( const pvsHandle_t handle, const int *targetAreas, int numTargetAreas ) const {
	int i;

	if ( handle.i < 0 || handle.i >= MAX_CURRENT_PVS ||
		handle.h != currentPVS[handle.i].handle.h ) {
		g_core->DropError( "idPVS::InCurrentPVS: invalid handle" );
	}

	for ( i = 0; i < numTargetAreas; i++ ) {
		if ( targetAreas[i] < 0 || targetAreas[i] >= numAreas ) {
			continue;
		}
		if ( currentPVS[handle.i].pvs[targetAreas[i]>>3] & (1 << (targetAreas[i]&7)) ) {
			return true;
		}
	}
	return false;
}

/*
================
idPVS::DrawPVS
================
*/
void idPVS::DrawPVS( const vec3_c &source, const pvsType_t type ) const {
	//int i, j, k, numPoints, n, sourceArea;
	//sharedPortal_s portal;
	//plane_c plane;
	//vec3_c offset;
	//idVec4 *color;
	//pvsHandle_t handle;

	//sourceArea = portalWorld->pointAreaNum( source );

	//if ( sourceArea == -1 ) {
	//	return;
	//}

	//handle = SetupCurrentPVS( source, type );

	//for ( j = 0; j < numAreas; j++ ) {

	//	if ( !( currentPVS[handle.i].pvs[j>>3] & (1 << (j&7)) ) ) {
	//		continue;
	//	}

	//	if ( j == sourceArea ) {
	//		color = &colorRed;
	//	}
	//	else {
	//		color = &colorCyan;
	//	}

	//	n = portalWorld->getNumPortalsInArea( j );

	//	// draw all the portals of the area
	//	for ( i = 0; i < n; i++ ) {
	//		portal = gameRenderWorld->GetPortal( j, i );

	//		numPoints = portal.w->size();

	//		portal.w->GetPlane( plane );
	//		offset = plane.Normal() * 4.0f;
	//		for ( k = 0; k < numPoints; k++ ) {
	//			gameRenderWorld->DebugLine( *color, (*portal.w)[k] + offset, (*portal.w)[(k+1)%numPoints] + offset );
	//		}
	//	}
	//}

	//FreeCurrentPVS( handle );
}

/*
================
idPVS::DrawPVS
================
*/
void idPVS::DrawPVS( const aabb &source, const pvsType_t type ) const {
//	int i, j, k, numPoints, n, num, areas[MAX_BOUNDS_AREAS];
//	sharedPortal_s portal;
//	plane_c plane;
//	vec3_c offset;
//	idVec4 *color;
//	pvsHandle_t handle;
//
//	num = portalWorld->boxAreaNums( source, areas, MAX_BOUNDS_AREAS );
//
//	if ( !num ) {
//		return;
//	}
//
//	handle = SetupCurrentPVS( source, type );
//
//	for ( j = 0; j < numAreas; j++ ) {
//
//		if ( !( currentPVS[handle.i].pvs[j>>3] & (1 << (j&7)) ) ) {
//			continue;
//		}
//
//		for ( i = 0; i < num; i++ ) {
//			if ( j == areas[i] ) {
//				break;
//			}
//		}
//		if ( i < num ) {
//			color = &colorRed;
//		}
//		else {
//			color = &colorCyan;
//		}
//
//		n = portalWorld->getNumPortalsInArea( j );
//
//		// draw all the portals of the area
//		for ( i = 0; i < n; i++ ) {
//			portal = gameRenderWorld->GetPortal( j, i );
//
//			numPoints = portal.w->size();
//
//			portal.w->GetPlane( plane );
//			offset = plane.Normal() * 4.0f;
//			for ( k = 0; k < numPoints; k++ ) {
//				gameRenderWorld->DebugLine( *color, (*portal.w)[k] + offset, (*portal.w)[(k+1)%numPoints] + offset );
//			}
//		}
//	}
//
//	FreeCurrentPVS( handle );
}

/*
================
idPVS::DrawPVS
================
*/
void idPVS::DrawCurrentPVS( const pvsHandle_t handle, const vec3_c &source ) const {
//	int i, j, k, numPoints, n, sourceArea;
//	sharedPortal_s portal;
//	plane_c plane;
//	vec3_c offset;
//	idVec4 *color;
//
//	if ( handle.i < 0 || handle.i >= MAX_CURRENT_PVS ||
//		handle.h != currentPVS[handle.i].handle.h ) {
//		g_core->DropError( "idPVS::DrawCurrentPVS: invalid handle" );
//	}
//
//	sourceArea = portalWorld->pointAreaNum( source );
//
//	if ( sourceArea == -1 ) {
//		return;
//	}
//
//	for ( j = 0; j < numAreas; j++ ) {
//
//		if ( !( currentPVS[handle.i].pvs[j>>3] & (1 << (j&7)) ) ) {
//			continue;
//		}
//
//		if ( j == sourceArea ) {
//			color = &colorRed;
//		}
//		else {
//			color = &colorCyan;
//		}
//
//		n = portalWorld->getNumPortalsInArea( j );
//
//		// draw all the portals of the area
//		for ( i = 0; i < n; i++ ) {
//			portal = gameRenderWorld->GetPortal( j, i );
//
//			numPoints = portal.w->size();
//
//			portal.w->GetPlane( plane );
//			offset = plane.Normal() * 4.0f;
//			for ( k = 0; k < numPoints; k++ ) {
//				gameRenderWorld->DebugLine( *color, (*portal.w)[k] + offset, (*portal.w)[(k+1)%numPoints] + offset );
//			}
//		}
//	}
}
