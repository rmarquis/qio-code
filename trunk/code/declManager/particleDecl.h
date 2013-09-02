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
// particleDecl.h - Doom3 particle decls handling
#ifndef __PARTICLEDECL_H__
#define __PARTICLEDECL_H__

#include "declRefState.h"
#include <shared/str.h>
#include <shared/array.h>
#include <math/vec3.h>

class particleDeclAPI_i {

};

//
// particle "distribution" parameter class
//
enum particleDistributionType_e {
	PDIST_BAD,
	PDIST_RECT, // rect <sizeX> <sizeY> <sizeZ>
	PDIST_SPHERE, // sphere <sizeX> <sizeY> <sizeZ> <frac>
	PDIST_CYLINDER, // rect <sizeX> <sizeY> <sizeZ>
};
class particleDistribution_c {
	particleDistributionType_e type;
	vec3_c size;
	float frac; // only if type == PDT_SPHERE
public:
	bool parseParticleDistribution(class parser_c &p, const char *fname);
};
//
// particle "direction" parameter class
//
enum particleDirectionType_e {
	PDIRT_BAD,
	PDIRT_CONE,
	PDIRT_OUTWARD,
};
class particleDirection_c {
	particleDirectionType_e type;
	float parm;
public:
	bool parseParticleDirection(class parser_c &p, const char *fname);
};
//
// generic particle parameter class
//
class particleParm_c {
	str tableName;
	float from;
	float to;
public:
	bool parseParticleParm(class parser_c &p, const char *fname);
};
//
// particle "orientation" parameter class
//
enum particleOrientationType_e {
	POR_BAD,
	POR_VIEW,
	POR_X,
	POR_Y,
	POR_Z,
	POR_AIMED,
};
class particleOrientation_c {
	particleOrientationType_e type;
public:
	bool parseParticleOrientation(class parser_c &p, const char *fname);
};
class particleStage_c {
	int count;
	str material;
	float time;
	float cycles;
	float bunching;
	float fadeIn, fadeOut;
	float fadeIndex;
	bool bWorldGravity;
	float gravity;
	particleDistribution_c distribution;
	particleOrientation_c orientation;
	particleDirection_c direction;
	particleParm_c rotSpeed;
	particleParm_c size;
	particleParm_c aspect;
	particleParm_c speed;
	bool bRandomDistribution;
	float boundsExpansion;
	float color[4];
	float fadeColor[4];
	vec3_c offset;
public:

	bool parseParticleStage(class parser_c &p, const char *fname);
};

class particleDecl_c : public particleDeclAPI_i, public declRefState_c {
	particleDecl_c *hashNext;
	str particleDeclName;
	float depthHack;
	arraySTD_c<particleStage_c*> stages;
public:
	const char *getName() const;
	void setDeclName(const char *newDeclName);
	bool isValid() const;
	
	bool parse(const char *text, const char *textBase, const char *fname);


	void setHashNext(particleDecl_c *newHashNext) {
		hashNext = newHashNext;
	}
	particleDecl_c *getHashNext() const {
		return hashNext;
	}
};

#endif // __PARTICLEDECL_H__
