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
// cg_emitter_base.h - base class for emitter objects
#ifndef __CG_EMITTER_BASE_H__
#define __CG_EMITTER_BASE_H__

#include <api/customRenderObjectAPI.h>

// emitter object base
class emitterBase_c : public customRenderObjectAPI_i {
public:
	virtual ~emitterBase_c() { }

	virtual void setOrigin(const class vec3_c &newOrigin) { };
	virtual void updateEmitter(int newTime) { };
	virtual void setRadius(float newSpriteRadius) { };
	virtual void setInterval(int newSpawnInterval) { };
	virtual void setMaterial(class mtrAPI_i *newMat) { };
	virtual void setParticleDecl(class particleDeclAPI_i *newDecl) { };
};

#endif // __CG_EMITTER_BASE_H__
