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
// mat_texmods.cpp
#include "mat_texMods.h"
#include <shared/parser.h>
#include <shared/ast.h>
#include <api/coreAPI.h>
#include <math/matrix.h>

texMod_c::texMod_c() {
	type = TCMOD_BAD;
}
texMod_c::texMod_c(const texMod_c &other) {
	type = other.type;
	if(type == TCMOD_TURBULENT) {
		wave = other.wave;
	} else if(type == TCMOD_SCROLL) {
		scroll[0] = other.scroll[0];
		scroll[1] = other.scroll[1];
	} else if(type == TCMOD_ROTATE) {
		rotationSpeed = other.rotationSpeed;
	} else if(type == TCMOD_SCALE) {
		scale[0] = other.scale[0];
		scale[1] = other.scale[1];
	} else if(type == TCMOD_TRANSFORM) {
		translate[0] = other.translate[0];
		translate[1] = other.translate[1];
		matrix[0][0] = other.matrix[0][0];
		matrix[0][1] = other.matrix[0][1];
		matrix[1][0] = other.matrix[1][0];
		matrix[1][1] = other.matrix[1][0];
	} else if(type == TCMOD_STRETCH) {
		wave = other.wave;
	} else if(type == TCMOD_D3_ROTATE) {
		astRotation = other.astRotation->duplicateAST();
	} else if(type == TCMOD_D3_SCALE) {
		astScale[0] = other.astScale[0]->duplicateAST();
		astScale[1] = other.astScale[1]->duplicateAST();
	} else if(type == TCMOD_D3_SHEAR) {
		astShear[0] = other.astShear[0]->duplicateAST();
		astShear[1] = other.astShear[1]->duplicateAST();
	} else if(type == TCMOD_D3_SCROLL) {
		astScroll[0] = other.astScroll[0]->duplicateAST();
		astScroll[1] = other.astScroll[1]->duplicateAST();
	} else if(type == TCMOD_D3_CENTERSCALE) {
		astScale[0] = other.astScale[0]->duplicateAST();
		astScale[1] = other.astScale[1]->duplicateAST();
	}
}
texMod_c &texMod_c::operator =(const texMod_c &other) {
	clear();
	texMod_c::texMod_c(other);
	return *this;
}
texMod_c::~texMod_c() {
	clear();
}	
void texMod_c::clear() {
	if(type == TCMOD_D3_ROTATE) {
		astRotation->destroyAST();
		astRotation = 0;
	} else if(type == TCMOD_D3_SCROLL) {
		astScroll[0]->destroyAST();
		astScroll[0] = 0;
		astScroll[1]->destroyAST();
		astScroll[1] = 0;
	} else if(type == TCMOD_D3_SCALE || type == TCMOD_D3_CENTERSCALE) {
		astScale[0]->destroyAST();
		astScale[0] = 0;
		astScale[1]->destroyAST();
		astScale[1] = 0;
	} else if(type == TCMOD_D3_SHEAR) {
		astShear[0]->destroyAST();
		astShear[0] = 0;
		astShear[1]->destroyAST();
		astShear[1] = 0;
	}
	type = TCMOD_BAD;
}
bool texMod_c::parse(class parser_c &p) {
	if(p.atWord("scroll")) {
		type = TCMOD_SCROLL;
		scroll[0] = p.getFloat();
		scroll[1] = p.getFloat();
	} else if(p.atWord("stretch")) {
		type = TCMOD_STRETCH;
		if(wave.parse(p)) {
			return true; // error
		}
	} else if(p.atWord("scale")) {
		type = TCMOD_SCALE;
		scale[0] = p.getFloat();
		scale[1] = p.getFloat();
	} else if(p.atWord("transform")) {
		type = TCMOD_TRANSFORM;
		matrix[0][0] = p.getFloat();
		matrix[0][1] = p.getFloat();
		matrix[1][0] = p.getFloat();
		matrix[1][1] = p.getFloat();
		translate[0] = p.getFloat();
		translate[1] = p.getFloat();
	} else if(p.atWord("turb")) {
		type = TCMOD_TURBULENT;
		if(wave.parseParameters(p)) {
			return true; // error
		}
	} else if(p.atWord("rotate")) {
		type = TCMOD_ROTATE;
		rotationSpeed = p.getFloat();
	} else {
		g_core->RedWarning("texMod_c::parse: unknown tcmod type %s in file %s at line %i\n",p.getToken(),p.getDebugFileName(),p.getCurrentLineNumber());
		return true; // error
	}
	return false; // ok
}

void texMod_c::applyRotationToMatrix(class matrix_c &mat, float rot) {
	mat.translate(0.5f, 0.5f, 0);
	mat.rotateZ(rot);
	mat.translate(-0.5f, -0.5f, 0);
}
void texMod_c::appendTransform(class matrix_c &mat, float timeNowSeconds, const class astInputAPI_i *in) {
	if(type == TCMOD_SCROLL) {
		float s = timeNowSeconds * this->scroll[0];
		float t = timeNowSeconds * this->scroll[1];
		// normalize coordinates
		s = s - floor(s);
		t = t - floor(t);
		// append transform
		mat.translate(s,t,0);
	} else if(type == TCMOD_STRETCH) {
		float div = this->wave.evaluate(timeNowSeconds);
		float scale;
		if(div == 0) {
			scale = 1.f;
		} else {
			scale = 1.f / div;
		}
		mat.translate(0.5f, 0.5f, 0);
		mat.scale(scale,scale,0);
		mat.translate(-0.5f, -0.5f, 0);
	} else if(type == TCMOD_SCALE) {
		mat.scale(this->scale[0],this->scale[1],0);
	} else if(type == TCMOD_TRANSFORM) {
		matrix_c trans;
		trans.identity();
		trans[0] = this->matrix[0][0];
		trans[1] = this->matrix[0][1];
		trans[4] = this->matrix[1][0];
		trans[5] = this->matrix[1][1];
		trans[12] = this->translate[0];
		trans[13] = this->translate[1];
		mat = mat * trans;
	} else if(type == TCMOD_TURBULENT) {
		float x = 0.25;
		float y = this->wave.phase + timeNowSeconds * this->wave.frequency;
		float scaleX = 1.f + (this->wave.base + sin(y) * this->wave.amplitude) * x;
		float scaleY = 1.f + (this->wave.base + sin(y + 0.25f) * this->wave.amplitude) * x;
		mat.scale(scaleX,scaleY,0.f);
	} else if(type == TCMOD_ROTATE) {
		float rot = this->rotationSpeed * timeNowSeconds;
		rot = -rot;
		applyRotationToMatrix(mat,rot);
	} else if(type == TCMOD_D3_ROTATE) {
		float rot = astRotation->execute(in);
		rot *= 360.f;
		applyRotationToMatrix(mat,rot);
	} else if(type == TCMOD_D3_SCALE) {
		float scaleVal0 = astScale[0]->execute(in);
		float scaleVal1 = astScale[1]->execute(in);
		mat.scale(scaleVal0,scaleVal1,0);
	} else if(type == TCMOD_D3_SHEAR) {
		float shearVal0 = astShear[0]->execute(in);
		float shearVal1 = astShear[1]->execute(in);
		mat.translate(0.5f, 0.5f, 0);
		mat.shear(shearVal0,shearVal1);
		mat.translate(-0.5f, -0.5f, 0);
	} else if(type == TCMOD_D3_SCROLL) {
		float s = astScroll[0]->execute(in);
		float t = astScroll[1]->execute(in);
		// normalize coordinates
		s = s - floor(s);
		t = t - floor(t);
		// append transform
		mat.translate(s,t,0);
	} else if(type == TCMOD_D3_CENTERSCALE) {
		//float scaleVal0 = astScale[0]->execute(in);
		//float scaleVal1 = astScale[1]->execute(in);
		//mat.translate(0.5f, 0.5f, 0);
		//mat.translate(-0.5f, -0.5f, 0);
		//mat.scale(scaleVal0,scaleVal1,0);
	} else {
		g_core->RedWarning("texMod_c::appendTransform: type %i not handled\n",this->type);
	}
}

void texModArray_c::calcTexMatrix(matrix_c &out, float timeNowSeconds, const class astInputAPI_i *in) {
	out.identity();
	for(u32 i = 0; i < size(); i++) {
		(*this)[i].appendTransform(out, timeNowSeconds, in);
	}
}
void texModArray_c::addD3TexModRotate(class astAPI_i *value) {
	this->pushBack().setD3TexModRotate(value);
}
void texModArray_c::addD3TexModScale(class astAPI_i *val0, class astAPI_i *val1) {
	this->pushBack().setD3TexModScale(val0,val1);
}
void texModArray_c::addD3TexModShear(class astAPI_i *val0, class astAPI_i *val1) {
	this->pushBack().setD3TexModShear(val0,val1);
}
void texModArray_c::addD3TexModScroll(class astAPI_i *val0, class astAPI_i *val1) {
	this->pushBack().setD3TexModScroll(val0,val1);
}
void texModArray_c::addD3TexModCenterScale(class astAPI_i *val0, class astAPI_i *val1) {
	this->pushBack().setD3TexModCenterScale(val0,val1);
}


