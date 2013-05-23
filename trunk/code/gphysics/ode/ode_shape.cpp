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
// btp_shape.cpp
#include "ode_shape.h"
#include "ode_headers.h"
#include <api/cmAPI.h>
#include <api/coreAPI.h>
#include "ode_convert.h"
#include "ode_cMod2OdeGeom.h"

odeColShape_c::odeColShape_c() {
	geom = 0;
	centerOfMassTransform.identity();
	type = OCS_BAD;
}
odeColShape_c::~odeColShape_c() {
	// NOTE: geoms are automatically removed along with ode world
}		

bool odeColShape_c::init(dSpaceID space, const class cMod_i *newCModel, bool newBIStatic) {
	this->cModel = newCModel;

	vec3_c centerOfMass;
	if(newBIStatic) {
		// static models dont need to have center of mass fixed
		bHasCenterOfMassTransform = false;
		centerOfMass.zero();
	} else {
		aabb bb;
		newCModel->getBounds(bb);
		centerOfMass = bb.getCenter();
		this->centerOfMassTransform.identity();
		if(centerOfMass.isAlmostZero()) {
			bHasCenterOfMassTransform = false;
		} else {
			bHasCenterOfMassTransform = true;
			this->centerOfMassTransform.setOrigin((centerOfMass*QIO_TO_BULLET).floatPtr());
		}
	}
	if(cModel->isCompound() && cModel->getCompound()->getNumSubShapes() == 1) {
		cModel = cModel->getCompound()->getSubShapeN(0);
	}

	// see if we can represent this geom with AABB 
	if(cModel->isHull() && cModel->isHullBoxShaped()) {
		type = OCS_BOX;
		aabb bb;
		cModel->getBounds(bb);
		bb.translate(-centerOfMass);
		boxSizes = bb.maxs*QIO_TO_BULLET*2.f;
		geom = dCreateBox(space,boxSizes.x,boxSizes.y,boxSizes.z);
		return false;
	} else {
		//geom = dCreateSphere (space,0.5f);
		//type = OCS_SPHERE;

		type = OCS_TRIMESH;
		cModel->getRawTriSoupData(&sf);
		if(sf.getNumTris()) {
			sf.translateXYZ(-centerOfMass);
			sf.scaleXYZ(QIO_TO_BULLET);
			sf.swapIndexes();
			dTriMeshDataID triMeshData = dGeomTriMeshDataCreate();
			dGeomTriMeshDataBuildSingle(triMeshData, sf.getVerts(), 3 * sizeof(float),
				sf.getNumVerts(), sf.getIndices(), sf.getNumIndices(), 3 * sizeof(dTriIndex));
			geom = dCreateTriMesh(space, triMeshData, 0, 0, 0);
		}
	}
	//return ODE_CreateSphereGeom(space,0.5);

	//geom = ODE_CModelToODEGeom(space,newCModel,newBIStatic,&centerOfMass);
	return false; // no error
}

