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
// skelUtils.cpp - helper functions for skeletal animation
// This file should be included in each module using bones system.
#include "skelUtils.h"

void boneOrArray_c::localBonesToAbsBones(const class boneDefArray_c *boneDefs) {
	// TODO: validate the bones order and their names?

	const boneDef_s *def = boneDefs->getArray();
	boneOr_s *or = this->getArray();
	for(u32 i = 0; i < size(); i++, or++, def++) {
		if(def->parentIndex == -1) {
			// do nothing
		} else {
			matrix_c parent = (*this)[def->parentIndex].mat;
			or->mat = parent * or->mat;
		}
	}
}