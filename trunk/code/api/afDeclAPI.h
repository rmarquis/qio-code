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
// afDeclAPI.h
#ifndef __AFDECLAPI_H__
#define __AFDECLAPI_H__

#include <shared/str.h>
#include <shared/array.h>

//
//	public AF structs
//
enum afVec3Type_e {
	AFVEC3_RAWDATA,
	AFVEC3_BONECENTER,
	AFVEC3_JOINT,
};

struct afVec3_s {
	afVec3Type_e type;
	vec3_c rawData;
	str boneName;
	str secondBoneName;
};

enum afModelType_e {
	AFM_BAD,
	AFM_CYLINDER,
	AFM_BONE,
	AFM_CONE,
	AFM_BOX,
	AFM_DODECAHEDRON,
};

// cylinder model format: point0, points1, numSides
// bone model format: joint0, joint1, width
struct afModel_s {
	afModelType_e type;
	// cylinder points
	afVec3_s v[2];
	union {
		int numSides; // cylinder numsides
		float width; // bone width
	};
	str jointNames[2]; // bone joints
};

struct afBody_s {
	str name;
	str jointName; // "joint"
	afModel_s model;
	afVec3_s origin; // in Doom3 src its idAFVector
	vec3_c angles; // in Doom3 src it's idAngles
	float density;
	arraySTD_c<str> containedJoints;
	// cached data - is it really needed here?
	//arraySTD_c<int> containedJointNums;
	//arraySTD_c<matrix_c> jointOffsets;
};
enum afConstraintType_e {
	AFC_UNIVERSALJOINT,
	AFC_HINGE,
};
// constraints are used to link two bodies together
struct afConstraint_s {
	afConstraintType_e type;
	str name;
	str body0Name;
	str body1Name;
	//afBody_s *body0;
	//afBody_s *body1;
	afVec3_s anchor;
	afVec3_s axis; // for hinges?
};
struct afPublicData_s {
	str name;
	str modelName;
	arraySTD_c<afBody_s> bodies;
	arraySTD_c<afConstraint_s> constraints;

	bool isEmpty() const {
		if(name.length() == 0)
			return true;
		if(bodies.size() == 0 && constraints.size() == 0)
			return true;
		return false;
	}
	int getLocalIndexForBodyName(const char *bodyName) const {
		const afBody_s *b = bodies.getArray();
		for(u32 i = 0; i < bodies.size(); i++, b++) {
			if(!stricmp(b->name,bodyName)) {
				return i;
			}
		}
		return -1;
	}
};
class afDeclAPI_i {
public:
	virtual const char *getName() const = 0;
	virtual const afPublicData_s *getData() const = 0;
	virtual const char *getDefaultRenderModelName() const = 0;
	virtual u32 getNumBodies() const = 0;
};

#endif // __AFDECLAPI_H__
