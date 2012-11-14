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
// rf_decals.cpp - decals managment, storage, and drawing
#ifndef __RF_DECALS_H__
#define __RF_DECALS_H__

#include <shared/array.h>

// TODO: pvsDecalBatcher for BSP trees
// and octreeDecalBatches for larger outdoor scenes?
class simpleDecalBatcher_c {
	arraySTD_c<class r_surface_c*> batches;
	arraySTD_c<struct simpleDecal_s*> decals;

	void addDecalsWithMatToBatch(class mtrAPI_i *mat, class r_surface_c *batch);
	bool hasDecalWithMat(class mtrAPI_i *m);
	void rebuildBatchWithMat(class mtrAPI_i *m);
public:
	simpleDecalBatcher_c();
	~simpleDecalBatcher_c();

	void addDecal(const simplePoly_s &decalPoly);
	void addDrawCalls();
};

#endif // __RF_DECALS_H__
