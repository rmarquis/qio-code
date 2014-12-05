/*
============================================================================
Copyright (C) 2013 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// g_pathNodes.cpp - node based pathfinding class
#include <math/aabb.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <api/coreAPI.h>
#include <api/ddAPI.h>
#include <api/rAPI.h>
#include <api/cmAPI.h>
#include <api/simplePathAPI.h>
#include "g_local.h"
#include "classes/InfoPathNode.h"

static aCvar_c g_debugDraw_pathNodes("g_debugDraw_pathNodes","0");
static aCvar_c g_debugDraw_pathLinks("g_debugDraw_pathLinks","0");
static aCvar_c g_debugDraw_activePaths("g_debugDraw_activePaths","0");

class pathNode_c {
	u32 index; // node index in "nodes" array
	vec3_c origin;
	arraySTD_c<pathNode_c*> links;
	class InfoPathNode *entity;
public:
	pathNode_c() {
		index = 0;
		entity = 0;
	}
	~pathNode_c() {
		for(u32 i = 0; i < links.size(); i++) {
			links[i]->removeLink(this);
		}
	}
	void setNodeEntity(class InfoPathNode *e) {
		entity = e;
	}
	void setOrigin(const vec3_c &no) {
		origin = no;
	}
	void setIndex(u32 newIndex) {
		index = newIndex;
	}
	void addLink(class pathNode_c *other) {
		if(links.isOnList(other))
			return;
		links.push_back(other);
	}
	void removeLink(class pathNode_c *other) {
		links.remove(other);
	}
	u32 getIndex() const {
		return index;
	}
	u32 getNumLinks() const {
		return links.size();
	}
	class pathNode_c *getLink(u32 i) const {
		return links[i];
	}
	bool hasLinkTo(class pathNode_c *other) const {
		if(links.isOnList(other))
			return true;
		return false;
	}
	const vec3_c &getOrigin() const {
		return origin;
	}
};

static arraySTD_c<class navPath_c*> g_activePaths;

class navPath_c : public simplePathAPI_i {
	arraySTD_c<pathNode_c*> path;
	
	// simplePathAPI_i
	virtual u32 getNumPathPoints() const {
		return path.size();
	}
	virtual const vec3_c &getPointOrigin(u32 idx) const { 
		return path[idx]->getOrigin();
	}
public:
	navPath_c() {
		g_activePaths.push_back(this);
	}
	virtual ~navPath_c() {
		g_activePaths.remove(this);
	}
	void addNode(class pathNode_c *n) {
		path.push_back(n);
	}
	void debugDrawPath(class rDebugDrawer_i *dd) const {
		u32 prev = 0;
		float hOfs = 48.f;
		for(u32 i = 1; i < path.size(); i++, prev++) {
			vec3_c p0 = path[i]->getOrigin();
			vec3_c p1 = path[prev]->getOrigin();
			p0.z += hOfs;
			p1.z += hOfs;
			dd->drawLineFromTo(p0,p1,vec3_c(1,1,0),8.f);
		}
	}
};

// defines a list of pathnodes for current world map.
// Used by A-Star algorithm.
// TODO: optimize node searching/adding/removing with an octtree?
class pathMap_c {
	arraySTD_c<pathNode_c*> nodes;

	u32 boxPathNodes(const aabb &bb, arraySTD_c<pathNode_c*> &out) {
		aabb extended = bb;
		extended.extend(16);
		for(u32 i = 0; i < nodes.size(); i++) {
			pathNode_c *n = nodes[i];
			if(n == 0)
				continue; // skip NULL entries in nodes array
			if(extended.isInside(n->getOrigin())) {
				out.push_back(n);
			}
		}
		return out.size();
	}
	bool nodesConnected(pathNode_c *n0, pathNode_c *n1) {
		if(n0->hasLinkTo(n1))
			return true;
		return false;
	}
	bool canConnectNodes(pathNode_c *n0, pathNode_c *n1) {
		trace_c tr;
		vec3_c zOfs(0,0,32);
		tr.setupRay(n0->getOrigin()+zOfs,n1->getOrigin()+zOfs);
		tr.setSphereRadius(32.f);
		if(cm == 0)
			return true;
		if(cm->traceWorldSphere(tr))
			return false;
		return true;
	}
	void connectNodes(pathNode_c *n0, pathNode_c *n1) {
		if(nodesConnected(n0,n1)) {
			g_core->RedWarning("connectNodes: already connected\n");
			return;
		}
		n0->addLink(n1);
		n1->addLink(n0);
	}
	void calcNodeLinks(class pathNode_c *n) {
		// create node reachability bounds
		aabb bb(n->getOrigin());
		bb.extend(128);
		// get nodes inside reachability bounds
		arraySTD_c<pathNode_c*> neighbours;
		boxPathNodes(bb,neighbours);
		// see if they are really reachable and if yes add links
		for(u32 i = 0; i < neighbours.size(); i++) {
			pathNode_c *other = neighbours[i];
			if(other == n)
				continue;
			if(nodesConnected(other,n))
				continue; // already connected
			if(canConnectNodes(other,n) == false)
				continue; // can't connect nodes because something is obstructing the path
			connectNodes(other,n);
		}
	}
	void calcLinks() {
		for(u32 i = 0; i < nodes.size(); i++) {
			calcNodeLinks(nodes[i]);
		}
	}
public:
	pathMap_c() {

	}
	~pathMap_c() {

	}
	pathNode_c *addPathNode(class InfoPathNode *newNode) {
		class pathNode_c *n = new pathNode_c;
		n->setIndex(nodes.size());
		n->setNodeEntity(newNode);
		n->setOrigin(newNode->getOrigin());
		calcNodeLinks(n);
		nodes.push_back(n);
		return n;
	}
	// reset the indexes of all nodes
	void setNodeNumbers() {
		for(u32 i = 0; i < nodes.size(); i++) {
			nodes[i]->setIndex(i);
		}
	}
	void freePathNode(class pathNode_c *n) {
#if 0
		nodes.remove(n);
		setNodeNumbers();
#else
		// don't destroy node indexes
		nodes[n->getIndex()] = 0;
#endif
		delete n;
	}
	pathNode_c *findNearestNode(const vec3_c &p, float maxDist) {
		aabb bb;
		bb.fromPointAndRadius(p,maxDist);
		arraySTD_c<pathNode_c*> list;
		boxPathNodes(bb,list);
		if(list.size() == 0)
			return 0;
		float bestDist = p.distSQ(list[0]->getOrigin());
		pathNode_c *out = list[0];
		for(u32 i = 1; i < list.size(); i++) {
			float d = list[i]->getOrigin().distSQ(p);
			if(d < bestDist) {
				bestDist = d;
				out = list[i];
			}
		}
		return out;
	}
	arraySTD_c<pathNode_c*> came_from;
	arraySTD_c<float> g_score, f_score;

	float estimateHeuristicCost(pathNode_c *f, pathNode_c *t) {
		return f->getOrigin().dist(t->getOrigin());
	}
	float getDistBetween(pathNode_c *f, pathNode_c *t) {
		return f->getOrigin().dist(t->getOrigin());
	}
	navPath_c *findPath(pathNode_c *start, pathNode_c *goal) {
		arraySTD_c<pathNode_c*> closed;
		arraySTD_c<pathNode_c*> open;

		open.push_back(start);

		if(g_score.size() < nodes.size())
			g_score.resize(nodes.size());
		if(f_score.size() < nodes.size())
			f_score.resize(nodes.size());
		if(came_from.size() < nodes.size())
			came_from.resize(nodes.size());

		g_score.nullMemory();
		f_score.nullMemory();
		came_from.nullMemory();

		g_score[start->getIndex()] = 0;
		f_score[start->getIndex()] = g_score[start->getIndex()] + estimateHeuristicCost(start, goal);

		while(open.size()) {
			pathNode_c *current = open[0];
			float lowest = f_score[current->getIndex()];
			for(u32 i = 1; i < open.size(); i++) {
				pathNode_c *n = open[i];
				float f = f_score[n->getIndex()];
				if(f < lowest) {
					lowest = f;
					current = n;
				}
			}
			if(current == goal) {
				// path found
				return reconstructPath(goal);
			}
			open.remove(current);
			closed.push_back(current);
			
			for(u32 i = 0; i < current->getNumLinks(); i++) {
				pathNode_c *neighbor = current->getLink(i);
				float tentative_g_score = g_score[current->getIndex()] + getDistBetween(current,neighbor);
				if(closed.isOnList(neighbor) && tentative_g_score >= g_score[neighbor->getIndex()]) 
					continue;
				if(!open.isOnList(neighbor) ||  tentative_g_score < g_score[neighbor->getIndex()]) {
					came_from[neighbor->getIndex()] = current;
					g_score[neighbor->getIndex()] = tentative_g_score;
					f_score[neighbor->getIndex()] = g_score[neighbor->getIndex()] + estimateHeuristicCost(neighbor, goal);
					open.add_unique(neighbor);
				}
			}
		}
		// failed
		return 0;
	}
	navPath_c *reconstructPath(pathNode_c *current_node) {
		navPath_c *p;
		if(came_from[current_node->getIndex()]) {
			p = reconstructPath(came_from[current_node->getIndex()]);
		} else {
			p = new navPath_c;
		}
		p->addNode(current_node);
		return p;
	}
	// returned path must be fried with 'delete'
	navPath_c *findPath(const vec3_c &from, const vec3_c &to) {
		class pathNode_c *start = findNearestNode(from,64.f);
		if(start == 0)
			return 0;
		class pathNode_c *goal = findNearestNode(to,64.f);
		if(goal == 0)
			return 0;
		return findPath(start, goal);
	}
	// debug drawing
	void debugDrawNode(class rDebugDrawer_i *dd, class pathNode_c *n) {
		dd->setColor(1,0,1,1);
		dd->drawBBExts(n->getOrigin()+vec3_c(0,0,16),vec3_c(0,0,0),vec3_c(16,16,16));
	}
	void debugDrawNodeLinks(class rDebugDrawer_i *dd, class pathNode_c *n) {
		for(u32 i = 0; i < n->getNumLinks(); i++) {
			class pathNode_c *d = n->getLink(i);
			dd->drawLineFromTo(d->getOrigin(),n->getOrigin(),vec3_c(0.1,1,0.1));
		}
	}
	void debugDrawNodes(class rDebugDrawer_i *dd) {
		if(g_debugDraw_pathNodes.getInt()) {
			for(u32 i = 0; i < nodes.size(); i++) {
				debugDrawNode(dd,nodes[i]);
			}
		}
		if(g_debugDraw_pathLinks.getInt()) {
			for(u32 i = 0; i < nodes.size(); i++) {
				debugDrawNodeLinks(dd,nodes[i]);
			}
		}
/*		navPath_c *p = findPath(nodes[0],nodes[nodes.size()-1]);
		if(p) {
			p->debugDrawPath(dd);
			delete p;
		}
		p = findPath(nodes[0],findNearestNode(G_GetPlayerOrigin(0),1000));
		if(p) {
			p->debugDrawPath(dd);
			delete p;
		}	*/	
	}
};

static pathMap_c *g_pathMap = 0;

void G_InitPathnodesSystem() {

}

void G_ShutdownPathnodesSystem() {
	if(g_pathMap) {
		delete g_pathMap;
		g_pathMap = 0;
	}
}

// this is called from InfoPathNode::postSpawn()
class pathNode_c *G_AddPathNode(class InfoPathNode *nodeEntity) {
	if(g_pathMap == 0) {
		g_pathMap = new pathMap_c;
	}
	return g_pathMap->addPathNode(nodeEntity);
}
// this is called from InfoPathNode destructor
void G_RemovePathNode(class pathNode_c *pathNode) {
	if(g_pathMap == 0)
		return;
	g_pathMap->freePathNode(pathNode);
}

// returned path must be fried with 'delete'
class simplePathAPI_i *G_FindPath(const vec3_c &from, const vec3_c &to) {
	if(g_pathMap == 0)
		return 0;
	return g_pathMap->findPath(from,to);
}

void G_DebugDrawPathNodes(class rDebugDrawer_i *dd) {
	if(g_pathMap == 0)
		return;
	// draw map nodes
	g_pathMap->debugDrawNodes(dd);
	// draw active (allocated) paths
	if(g_debugDraw_activePaths.getInt()) {
		for(u32 i = 0; i < g_activePaths.size(); i++) {
			g_activePaths[i]->debugDrawPath(dd);
		}
	}
}



