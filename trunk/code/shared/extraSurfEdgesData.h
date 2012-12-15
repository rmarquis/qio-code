/*
============================================================================
Copyright (C) 2012 V.

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
// extraSurfEdgesData.h - surface edges data for shadow volumes generation
#ifndef __EXTRASURFEDGESDATA_H__
#define __EXTRASURFEDGESDATA_H__

struct rEdge16_s {
	u16 verts[2];
	u16 tris[2];

	inline bool isMatched() const {
		return (tris[0] != tris[1]);
	}
};

struct extraSurfEdgesData_s {
	//arraySTD_c<u16> silIndexes;
	arraySTD_c<rEdge16_s> edges;
	u32 c_matchedEdges;

	extraSurfEdgesData_s() {
		c_matchedEdges = 0;
	}
	void addEdge(u32 triNum, u32 v0, u32 v1) {
		rEdge16_s *e = edges.getArray();
		for(u32 i = 0; i < edges.size(); i++, e++) {
			if(e->tris[0] != e->tris[1])
				continue; // already matched
			if(e->verts[0] == v0 && e->verts[1] == v1) {
				e->tris[1] = triNum;
				c_matchedEdges++;
				return;
			}
			if(e->verts[0] == v1 && e->verts[1] == v0) {
				e->tris[1] = triNum;////-triNum - 1;
				c_matchedEdges++;
				return;
			}
		}
		rEdge16_s &newEdge = edges.pushBack();
		newEdge.verts[0] = v0;
		newEdge.verts[1] = v1;
		newEdge.tris[0] = newEdge.tris[1] = triNum;
	}
	u32 getNumUnmatchedEdges() const {
		return edges.size() - c_matchedEdges;
	}
	const rEdge16_s *getArray() const {
		return edges.getArray();
	}
	u32 size() const {
		return edges.size();
	}
};

#endif // __EXTRASURFEDGESDATA_H__


