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
// mtrStage_api.h - material stage class interface

#ifndef __MTRSTAGE_API_H__
#define __MTRSTAGE_API_H__

class mtrStageAPI_i  { 
public:
	virtual class textureAPI_i *getTexture(float curTimeSec = 0.f) const = 0;
	virtual class textureAPI_i *getTextureForFrameNum(u32 frameNum) const = 0;
	virtual enum alphaFunc_e getAlphaFunc() const = 0;
	virtual const struct blendDef_s &getBlendDef() const = 0;
	virtual class mtrStageAPI_i *getBumpMap() const = 0;
	virtual class mtrStageAPI_i *getHeightMap() const = 0;
	virtual bool hasTexMods() const = 0;
	virtual void applyTexMods(class matrix_c &out, float curTimeSec, const class astInputAPI_i *in) const = 0;
	virtual bool hasTexGen() const = 0;
	virtual enum texCoordGen_e getTexGen() const = 0;
	virtual enum stageType_e getStageType() const = 0;
	virtual bool hasRGBGen() const = 0;
	virtual enum rgbGen_e getRGBGenType() const = 0;
	virtual bool getRGBGenConstantColor3f(float *out3Floats) const = 0;
	virtual float getRGBGenWaveValue(float curTimeSec) const = 0;
	virtual bool getDepthWrite() const = 0;
	virtual void evaluateRGBGen(const class astInputAPI_i *in, float *out3Floats) const = 0;
	// return true if stage is conditional (has Doom3 'if' condition)
	virtual bool hasIFCondition() const = 0;
	// return true if drawing condition is met for given input variables
	virtual bool conditionMet(const class astInputAPI_i *in) const = 0;
};

#endif // __MTRSTAGE_API_H__
