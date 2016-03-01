/*
============================================================================
Copyright (C) 2016 V.

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
#include "textureAllocator.h"

textureAllocator_c::textureAllocator_c() {
	setupTextureSize(64);
}
void textureAllocator_c::setupTextureSize(u32 newSize) {
	maxTextureSize = newSize;
	allocated.resize(maxTextureSize);
	allocated.nullMemory();
}
bool textureAllocator_c::allocTextureBlock(const u32 inSizes[2], u32 outPos[2]) {
	u32 margin = 1;
	u32 sizes[2] = { inSizes[0]+margin, inSizes[1]+margin };
	u32 best = maxTextureSize;
	for(u32 i = 0; i <= maxTextureSize-sizes[0]; i++ ) {
		u32 best2 = 0;
		u32 j;
		for(j = 0; j < sizes[0]; j++) {
			if(allocated[i+j] >= best) {
				break;
			}
			if(allocated[i+j] > best2) {
				best2 = allocated[i+j];
			}
		}
		if(j == sizes[0])	{	// this is a valid spot
			outPos[0] = i;
			outPos[1] = best = best2;
		}
	}

	if(best + sizes[1] > maxTextureSize) {
		return true;
	}

	for(u32 i = 0; i < sizes[0]; i++) {
		allocated[outPos[0] + i] = best + sizes[1];
	}

	return false;
}
