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
// cg_tracer.cpp - bullet tracer effects
#include "cg_local.h"
#include <api/customRenderObjectAPI.h>
#include <shared/array.h>
#include <math/vec3.h>
#include <math/axis.h>
#include <api/staticModelCreatorAPI.h>
#include <api/rAPI.h>

class cgBulletTracer_c : public customRenderObjectAPI_i {
	int life; // in msec
	int addTime;
	float width;
	vec3_c from;
	vec3_c to;
	vec3_c forward;
	class mtrAPI_i *mat;
public:
	cgBulletTracer_c(const vec3_c &newFrom, const vec3_c &newTo, float newWidth, class mtrAPI_i *newMat, int newLife) {
		from = newFrom;
		to = newTo;
		width = newWidth;
		mat = newMat;
		life = newLife;
		forward = (to - from).getNormalized();

		addTime = cg.time;

		rf->addCustomRenderObject(this);
	}
	~cgBulletTracer_c() {
		rf->removeCustomRenderObject(this);
	}
	virtual void instanceModel(class staticModelCreatorAPI_i *out, const class axis_c &viewerAxis) {
		out->clear();
		out->addSprite(from,width,mat,viewerAxis);

		// compute side vector
		vec3_c v1 = from - cg.refdefViewOrigin;
		vec3_c v2 = to - cg.refdefViewOrigin;
		vec3_c right = v1.crossProduct(v2);
		right.normalize();

		float radius = width * 0.5;
		vec3_c points[4];
		points[1] = from + right * radius;
		points[2] = from - right * radius;
		points[3] = to - right * radius;
		points[0] = to + right * radius;

		for(u32 i = 0; i < 4; i++) {
			out->setVertexPos(i,points[i]);
		}
		int elapsed = cg.time - addTime;
		float lifeFracInv = float(elapsed)/float(life);
		float lifeFrac = 1.f - lifeFracInv;
		byte alpha = 255 * lifeFrac;
		out->setAllVertexColors(alpha,alpha,alpha,alpha);
	}
	bool hasExpired() const {
		int passed = cg.time - addTime;
		if(passed > life)
			return true;
		return false;
	}
};

static arraySTD_c<cgBulletTracer_c*> cg_tracers;

void CG_UpdateBulletTracers() {
	for(int i = 0; i < cg_tracers.size(); i++) {
		if(cg_tracers[i]->hasExpired()) {
			delete cg_tracers[i];
			cg_tracers.erase(i);
			i--;
		}
	}
}

void CG_AddBulletTracer(const vec3_c &from, const vec3_c &to, float width, class mtrAPI_i *mat, int life) {
	cgBulletTracer_c *tr = new cgBulletTracer_c(from,to,width,mat,life);
	cg_tracers.push_back(tr);
}


