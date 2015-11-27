/*
============================================================================
Copyright (C) 2015 V.

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
// boxAreasCacher.h - hash table util for removing redundant boxAreas calls
// NOTE: the aabb->areas mapping is constant for loaded map, areas never change during gameplay
// Areas are created by proc/bsp compiler.
#include <math/aabb.h>
#include <shared/array.h>
#include <api/coreAPI.h>

class cachedBoxAreas_c {
public:
	arraySTD_c<u32> areas;
	aabb box;
	cachedBoxAreas_c *next;
	bool bInuse;
	u32 hash;
	u32 at;

	cachedBoxAreas_c() {
		bInuse = false;
		hash = 0;
		at = 0;
		next = 0;
	}
	bool hasAABB(const aabb &ob) const {
		return ob.compare(box);
	}
};
template <int MAX_CACHED_REQUESTS, int TABLE_SIZE>
class boxAreasCacher_c {
	cachedBoxAreas_c *hashTable[TABLE_SIZE];
	cachedBoxAreas_c stored[MAX_CACHED_REQUESTS];
	u32 lastHash;
	// loop buffer for storing latest requests
	cachedBoxAreas_c *ptrs[MAX_CACHED_REQUESTS];
	u32 atPtr;

	u32 calcHash(const aabb &bb) {
		return u32(bb.mins.getComponentsSum() + bb.maxs.getComponentsSum()) % TABLE_SIZE;
	}
	void removeFromHashTable(cachedBoxAreas_c *p) {
		// simple case
		if(hashTable[p->hash] == p) {
			hashTable[p->hash] = p->next;
			return;
		}
		cachedBoxAreas_c *o = hashTable[p->hash];
		while(o) {
			if(o->next == p) {
				// just omit the p
				o->next = o->next->next;
				return;
			}
			o = o->next;
		}
		g_core->RedWarning("boxAreasCacher_c::removeFromHashTable: not on table!\n");
	}
public:
	boxAreasCacher_c() {
		memset(hashTable,0,sizeof(hashTable));
		memset(ptrs,0,sizeof(ptrs));
		atPtr = 0;
	}
	cachedBoxAreas_c *create(const aabb &bb) {
		// find first unused
		cachedBoxAreas_c *useMe = 0;
		for(u32 i = 0; i < MAX_CACHED_REQUESTS; i++) {
			if(stored[i].bInuse == false) {
				useMe = &stored[i];
				//g_core->Print("Using unused slot %i\n",i);
				break;
			}
		}
		// if no free slots, then reuse the oldest one
		if(useMe == 0) {
			// this is a circular buffer, so the oldest entry is next to newest
			u32 next = (atPtr+1) % MAX_CACHED_REQUESTS;
			useMe = ptrs[next];
			// remove it from hash table
			removeFromHashTable(useMe);
		}
		// add to hash
		useMe->bInuse = true;
		useMe->next = hashTable[lastHash];
		hashTable[lastHash] = useMe;
		useMe->box = bb;
		useMe->hash = lastHash;

		atPtr = (atPtr+1) % MAX_CACHED_REQUESTS;
		ptrs[atPtr] = useMe;
		useMe->at = atPtr;

		return useMe;
	}
	cachedBoxAreas_c *find(const aabb &bb) {
		lastHash = calcHash(bb);
		// check hash table
		cachedBoxAreas_c *p;
		p = hashTable[lastHash];
		while(p) {
			if(p->hasAABB(bb)) {
				// add the selected entry to the head of the list
				u32 next = (atPtr+1) % MAX_CACHED_REQUESTS;
				if(ptrs[next] && ptrs[next] != p) {
					u32 moveBackTo = p->at;
					cachedBoxAreas_c *moveBack = ptrs[next];
					ptrs[next] = p;
					ptrs[next]->at = next;
					ptrs[moveBackTo] = moveBack;
					ptrs[moveBackTo]->at = moveBackTo;
					// now, the found entry is at the head of the list
				}
				return p;
			}
			p = p->next;
		}
		return 0;
	}
};