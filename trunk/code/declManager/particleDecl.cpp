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
// particleDecl.cpp - Doom3 particle decls handling
#include "particleDecl.h"
#include <api/coreAPI.h>
#include <shared/parser.h>
#include <math/math.h>
#include <shared/simpleVert.h>

//
// particleDistribution_c
//
bool particleDistribution_c::parseParticleDistribution(class parser_c &p, const char *fname) {
	frac = 0.f;
	if(p.atWord("sphere")) {
		if(p.getFloatMat(size,3)) {
			int line = p.getCurrentLineNumber();
			g_core->RedWarning("particleDistribution_c::parse: failed to parse 'sphere' sizes at line %i of %s\n",line,fname);
			return true;
		}
		if(p.isAtEOL() == false) {
			frac = p.getFloat();
		}
		type = PDIST_SPHERE;
	} else if(p.atWord("cylinder")) {
		if(p.getFloatMat(size,3)) {
			int line = p.getCurrentLineNumber();
			g_core->RedWarning("particleDistribution_c::parse: failed to parse 'cylinder' sizes at line %i of %s\n",line,fname);
			return true;
		}
		if(p.isAtEOL() == false) {
			frac = p.getFloat();
		}
		type = PDIST_CYLINDER;
	} else if(p.atWord("rect")) {
		if(p.getFloatMat(size,3)) {
			int line = p.getCurrentLineNumber();
			g_core->RedWarning("particleDistribution_c::parse: failed to parse 'rect' sizes at line %i of %s\n",line,fname);
			return true;
		}
		type = PDIST_RECT;
	} else {
		int line = p.getCurrentLineNumber();
		const char *unk = p.getToken();
		g_core->RedWarning("particleDistribution_c::parse: unknown particle distribution type %s at line %i of %s\n",unk,line,fname);
		return true;
	}
	return false;
}
void particleDistribution_c::calcParticleInitialOrigin(bool bRandomDistribution, particleInstanceData_s &in, vec3_c &out) const {
	if(type == PDIST_RECT) {
		if(bRandomDistribution) {
			out.x = in.rand.getCRandomFloat() * size.x;
			out.y = in.rand.getCRandomFloat() * size.y;
			out.z = in.rand.getCRandomFloat() * size.z;
		} else {
			out.x = size.x;
			out.y = size.y;
			out.z = size.z;
		}
	} else if(type == PDIST_CYLINDER) {
		float angle;
		if(bRandomDistribution) {
			angle = in.rand.getCRandomFloat() * M_TWOPI;
			out.z = in.rand.getCRandomFloat();
		} else {
			angle = M_TWOPI;
			out.z = 1.f;
		}
		out.x = sin(angle);
		out.y = cos(angle);
		if(this->frac > 0.f) {
			float radiusSqr = out[0] * out[0] + out[1] * out[1];
			if (radiusSqr < Square(this->frac)) {
				// if we are inside the inner reject zone, rescale to put it out into the good zone
				float f = sqrt(radiusSqr) / this->frac;
				float invf = 1.0f / f;
				float newRadius = this->frac + f * ( 1.0f - this->frac );
				float rescale = invf * newRadius;

				out[0] *= rescale;
				out[1] *= rescale;
			}
		}
		out.x *= size.x;
		out.y *= size.y;
		out.z *= size.z;
	} else if(type == PDIST_SPHERE) {
		float radiusSqr;
		// iterating with rejection is the only way to get an even distribution over a sphere
		if (bRandomDistribution) {
			do {
				out[0] = in.rand.getCRandomFloat();
				out[1] = in.rand.getCRandomFloat();
				out[2] = in.rand.getCRandomFloat();
				radiusSqr = out[0] * out[0] + out[1] * out[1] + out[2] * out[2];
			} while( radiusSqr > 1.0f );
		} else {
			out.set(1.0f,1.0f,1.0f);
			radiusSqr = 3.0f;
		}

		if (this->frac > 0.0f) {
			// we could iterate until we got something that also satisfied ringFraction,
			// but for narrow rings that could be a lot of work, so reproject inside points instead
			if (radiusSqr < Square(this->frac) ) {
				// if we are inside the inner reject zone, rescale to put it out into the good zone
				float f = sqrt( radiusSqr ) / this->frac;
				float invf = 1.0f / f;
				float newRadius = this->frac + f * ( 1.0f - this->frac);
				float rescale = invf * newRadius;

				out[0] *= rescale;
				out[1] *= rescale;
				out[2] *= rescale;
			}
		}
		out.x *= size.x;
		out.y *= size.y;
		out.z *= size.z;
	}
}
//
// particleDirection_c
//
bool particleDirection_c::parseParticleDirection(class parser_c &p, const char *fname) {
	if(p.atWord("cone")) {
		parm = p.getFloat();
		type = PDIRT_CONE;
	} else if(p.atWord("outward")) {
		parm = p.getFloat();
		type = PDIRT_OUTWARD;
	} else {
		int line = p.getCurrentLineNumber();
		const char *unk = p.getToken();
		g_core->RedWarning("particleDirection_c::parse: unknown particle distribution type %s at line %i of %s\n",unk,line,fname);
		return true;
	}
	return false;
}	
void particleDirection_c::calcParticleDirection(particleInstanceData_s &in, const vec3_c &origin, vec3_c &out) const {
	if(type == PDIRT_CONE) {
		float a0 = DEG2RAD(in.rand.getCRandomFloat() * parm);
		float a1 = in.rand.getCRandomFloat() * M_PI;
		float s0, s1, c0, c1;
		s0 = sin(a0); s1 = sin(a1);
		c0 = cos(a0); c1 = cos(a1);
		out.set(s0*c1,s0*s1,c0);
	} else if(type == PDIRT_OUTWARD) {
		out = origin;
		out.normalize();
		out.z += parm;
	}
}
	
//
// particleParm_c
//
bool particleParm_c::parseParticleParm(class parser_c &p, const char *fname) {
	const char *s = p.getToken();
	if(isdigit(s[0]) || s[0] == '-') {
		from = atof(s);
		if(p.atWord("to")) {
			to = p.getFloat();
		} else {
			to = from;
		}
	} else {
		// table name
		tableName = s;
	}
	return false;
}

float particleParm_c::evaluate(float frac) const {
	if (tableName.length()) {
		// TODO
		return 0.f;
	}
	return from + frac * (to - from);
}
float particleParm_c::integrate(float frac) const {
	if(tableName.length()) {
		g_core->RedWarning("particleParm_c::integrate: unable to integrate tables\n");
		return 0.f;
	}
	return ( from + frac * ( to - from ) * 0.5f ) * frac;
}
//
// particleOrientation_c
//
bool particleOrientation_c::parseParticleOrientation(class parser_c &p, const char *fname) {
	if(p.atWord("view")) {

		type = POR_VIEW;
	} else if(p.atWord("x")) {

		type = POR_X;
	} else if(p.atWord("y")) {

		type = POR_Y;
	} else if(p.atWord("z")) {

		type = POR_Z;
	} else if(p.atWord("aimed")) {

		type = POR_AIMED;
	} else {
		int line = p.getCurrentLineNumber();
		const char *unk = p.getToken();
		g_core->RedWarning("particleOrientation_c::parse: unknown particle orientation type %s at line %i of %s\n",unk,line,fname);
		return true;
	}
	return false;
}
//
// particleStage_c
//
particleStage_c::particleStage_c() {
	setDefaults();
}
void particleStage_c::setDefaults() {
	bunching = 1.f;
	deadTime = 0.f;
	fadeIn = 0.1f;
	fadeOut = 0.25f;
	color[0] = color[1] = color[2] = color[3] = 1.f;
	fadeColor[0] = fadeColor[1] = fadeColor[2] = fadeColor[3] = 0.f;
	aspect.set(1.f);
	numAnimationFrames = 0;
	animationRate = 0.f;
	bEntityColor = false;
}
u32 particleStage_c::instanceParticle(particleInstanceData_s &in, struct simpleVert_s *verts) const {
	// start with calculating color.
	union {
		byte curColor[4];
		int colorAsInt;
	};
	calcParticleColor(in,curColor);
	// check if the particle has already faded out
	if(colorAsInt == 0) {
		return 0;
	}

	// calc particle origin at the given time
	vec3_c origin;
	calcParticleOrigin(in, origin);
	// calculate texcoords
	calcParticleTexCoords(in, verts);
	// calculate positions of particle vertices
	u32	numVerts = calcParticleVerts(in, origin, verts);


	return numVerts;
}
void particleStage_c::calcParticleColor(particleInstanceData_s &in, byte *outRGBA) const {
	float fadeFraction = 1.0f;

	// start fading in/out at a given fraction
	if (in.lifeFrac < fadeIn) {
		fadeFraction *= (in.lifeFrac / fadeIn);
	} 
	if (1.0f - in.lifeFrac < fadeOut) {
		fadeFraction *= ((1.0f - in.lifeFrac) / fadeOut);
	}

	float fadeFracInv = 1.0f - fadeFraction;
	// calculate each color
	for (u32 i = 0; i < 4; i++) {
		float fColor = color[i] * fadeFraction + fadeColor[i] * fadeFracInv;
		int iColor = fColor * 255.f;
		// normalize color to bytes range
		if (iColor < 0) {
			iColor = 0;
		} else if (iColor > 255) {
			iColor = 255;
		}
		outRGBA[i] = iColor;
	}
}
void particleStage_c::calcParticleOrigin(particleInstanceData_s &in, vec3_c &out) const {
	// first calculate initial particle origin
	distribution.calcParticleInitialOrigin(bRandomDistribution,in,out);

	// add generic particle spawn offset
	out += offset;

	// calc particle direction
	vec3_c dir;
	direction.calcParticleDirection(in,out,dir);

	// add speed offset in time
	float iSpeed = speed.integrate(in.lifeFrac);
	out += dir * iSpeed * this->time;	


	// add gravity

}
void particleStage_c::calcParticleTexCoords(particleInstanceData_s &in, struct simpleVert_s *verts) const {
	float s, width;
	if (numAnimationFrames > 1) {
		width = 1.0f / numAnimationFrames;
		float	floatFrame;
		if (animationRate) {
			// explicit, cycling animation
			floatFrame = in.lifeFrac * this->time * animationRate;
		} else {
			// single animation cycle over the life of the particle
			floatFrame = in.lifeFrac * numAnimationFrames;
		}
		int	intFrame = (int)floatFrame;
	//	in.animationFrameFrac = floatFrame - intFrame;
		s = width * intFrame;
	} else  {
		s = 0.0f;
		width = 1.0f;
	}

	float t = 0.0f;
	float height = 1.0f;

	verts[0].setUV(s,t);

	verts[1].setUV(s+width,t);

	verts[2].setUV(s,t+height);

	verts[3].setUV(s+width,t+height);
}
u32 particleStage_c::calcParticleVerts(particleInstanceData_s &in, const vec3_c &origin, struct simpleVert_s *verts) const {
	if(orientation.isOrientationTypeAimed()) {
		return calcParticleVerts_aimed(in,origin,verts);
	}
	float particleSize = size.evaluate(in.lifeFrac);
	float particleAspect = aspect.evaluate(in.lifeFrac);

	float width = particleSize;
	float height = particleSize * particleAspect;

	// apply rotation
	float angle;
	angle = 360 * in.rand.getRandomFloat();

	float angleMove = rotSpeed.integrate(in.lifeFrac) * this->time;
	// half of the particles will rotate in the opposite direction
	if (in.particleIndex & 1) {
		angle += angleMove;
	} else {
		angle -= angleMove;
	}

	angle = DEG2RAD(angle);
	float c = cos(angle);
	float s = sin(angle);

	vec3_c left, up;
	if (orientation.getType() == POR_Z) {
		left[0] = s;
		left[1] = c;
		left[2] = 0;
		up[0] = c;
		up[1] = -s;
		up[2] = 0;
	} else if (orientation.getType() == POR_X) {
		left[0] = 0;
		left[1] = c;
		left[2] = s;
		up[0] = 0;
		up[1] = -s;
		up[2] = c;
	} else if (orientation.getType() == POR_Y) {
		left[0] = c;
		left[1] = 0;
		left[2] = s;
		up[0] = -s;
		up[1] = 0;
		up[2] = c;
	} else if (orientation.getType() == POR_VIEW) {
		// TODO: handle entity space
		vec3_c viewLeft = in.viewAxis.getLeft();
		vec3_c viewUp = in.viewAxis.getUp();

		left = viewLeft * c + viewUp * s;
		up = viewUp * c - viewLeft * s;
	} else {
		g_core->RedWarning("Unknown orientation type %i\n",orientation.getType());
		return 0;
	}

	left *= width;
	up *= height;

	verts[0].xyz = origin - left + up;
	verts[1].xyz = origin + left + up;
	verts[2].xyz = origin - left - up;
	verts[3].xyz = origin + left - up;

	return 4;
}
u32 particleStage_c::calcParticleVerts_aimed(particleInstanceData_s &in, const vec3_c &origin, struct simpleVert_s *verts) const {

	return 0;
}
bool particleStage_c::parseParticleStage(class parser_c &p, const char *fname) {
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atWord("count")) {
			count = p.getInteger();
		} else if(p.atWord("material")) {
			material = p.getToken();
		} else if(p.atWord("time")) {
			time = p.getFloat();
		} else if(p.atWord("deadTime")) {
			deadTime = p.getFloat();
		} else if(p.atWord("cycles")) {
			cycles = p.getFloat();
		} else if(p.atWord("bunching")) {
			bunching = p.getFloat();
		} else if(p.atWord("distribution")) {
			if(distribution.parseParticleDistribution(p,fname)) {
				g_core->RedWarning("Failed to parse particle 'distribution'\n");
			}
		} else if(p.atWord("direction")) {
			if(direction.parseParticleDirection(p,fname)) {
				g_core->RedWarning("Failed to parse particle 'direction'\n");
			}
		} else if(p.atWord("orientation")) {
			if(orientation.parseParticleOrientation(p,fname)) {
				g_core->RedWarning("Failed to parse particle 'orientation'\n");
			}
		} else if(p.atWord("speed")) {
			if(speed.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'size' parm\n");
			}
		} else if(p.atWord("size")) {
			if(size.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'size' parm\n");
			}
		} else if(p.atWord("aspect")) {
			if(aspect.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'size' parm\n");
			}
		} else if(p.atWord("rotation")) {
			if(rotSpeed.parseParticleParm(p,fname)) {
				g_core->RedWarning("Failed to parse 'rotation' parm\n");
			}
		} else if(p.atWord("randomDistribution")) {
			bRandomDistribution = p.getInteger();
		} else if(p.atWord("boundsExpansion")) {
			boundsExpansion = p.getFloat();
		} else if(p.atWord("fadeIn")) {
			fadeIn = p.getFloat();
		} else if(p.atWord("fadeOut")) {
			fadeOut = p.getFloat();
		} else if(p.atWord("fadeIndex")) {
			fadeIndex = p.getFloat();
		} else if(p.atWord("color")) {
			if(p.getFloatMat(color,4)) {
				g_core->RedWarning("Failed to parse 'color' matrix (4xfloat)\n");
			}
		} else if(p.atWord("fadeColor")) {
			if(p.getFloatMat(fadeColor,4)) {
				g_core->RedWarning("Failed to parse 'fadeColor' matrix (4xfloat)\n");
			}
		} else if(p.atWord("offset")) {
			if(p.getFloatMat(offset,3)) {
				g_core->RedWarning("Failed to parse 'offset' matrix (3xfloat)\n");
			}
		} else if(p.atWord("gravity")) {
			if(p.atWord("world")) {
				bWorldGravity = true;
			}	
			gravity = p.getFloat();
		} else if(p.atWord("animationRate")) {
			animationRate = p.getFloat();
		} else if(p.atWord("animationFrames")) {
			numAnimationFrames = p.getFloat();
		} else if(p.atWord("entityColor")) {
			bEntityColor = p.getInteger();
		} else {
			int line = p.getCurrentLineNumber();
			const char *unk = p.getToken();
			g_core->RedWarning("particleStage_c::parse: unknown token %s at line %i of %s\n",unk,line,fname);
		}
	}
	this->cycleMsec = ( this->time + this->deadTime ) * 1000;
	return false;
}

	
//
// particleDecl_c
//
particleDecl_c::particleDecl_c() {
	depthHack = 0.f;
}
particleDecl_c::~particleDecl_c() {
	clear();
}
void particleDecl_c::clear() {
	for(u32 i = 0; i < stages.size(); i++) {
		delete stages[i];
	}
	stages.clear();
}
const char *particleDecl_c::getName() const {
	return particleDeclName;
}
u32 particleDecl_c::getNumStages() const {
	return stages.size();
}
const class particleStageAPI_i *particleDecl_c::getStage(u32 i) const {
	return stages[i];
}
void particleDecl_c::setDeclName(const char *newDeclName) {
	particleDeclName = newDeclName;
}
bool particleDecl_c::isValid() const {
	if(stages.size()) 
		return true;
	return false;
}
bool particleDecl_c::parse(const char *text, const char *textBase, const char *fname) {
	parser_c p;
	p.setup(textBase, text);
	p.setDebugFileName(fname);
	while(p.atWord_dontNeedWS("}") == false) {
		if(p.atWord_dontNeedWS("{")) {
			particleStage_c *newStage = new particleStage_c;
			if(newStage->parseParticleStage(p,fname)) {
				delete newStage;
			} else {
				stages.push_back(newStage);
			}
		} else if(p.atWord("depthHack")) {
			depthHack = p.getFloat();
		} else {
			int line = p.getCurrentLineNumber();
			const char *unk = p.getToken();
			g_core->RedWarning("particleDecl_c::parse: unknown token %s at line %i of %s\n",unk,line,fname);
		}
	}
	return false;
}

