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

// r,g,b,a masking for Doom3 materials
// set by "maskRed", "maskGreen", 
// "maskBlue", "maskColor", "maskAlpha"
// stage commands.

enum {
	MASKFLAG_RED = 1,
	MASKFLAG_GREEN = 2,
	MASKFLAG_BLUE = 4,
	MASKFLAG_ALPHA = 8
};

struct maskState_s {
	short flags;

	void toggleFlag(int flag, bool state) {
		if(state) {
			flags |= flag;
		} else {
			flags &= ~flag;
		}
	}
	bool getFlag(int flag) const {
		return (flags & flag);
	}
public:
	maskState_s() {
		flags = 0;
	}
	void setMaskRed(bool bMask) {
		toggleFlag(MASKFLAG_RED,bMask);
	}
	void setMaskGreen(bool bMask) {
		toggleFlag(MASKFLAG_GREEN,bMask);
	}
	void setMaskBlue(bool bMask) {
		toggleFlag(MASKFLAG_BLUE,bMask);
	}
	void setMaskAlpha(bool bMask) {
		toggleFlag(MASKFLAG_ALPHA,bMask);
	}
	void setMaskColor(bool bMask) {
		toggleFlag(MASKFLAG_RED,bMask);
		toggleFlag(MASKFLAG_GREEN,bMask);
		toggleFlag(MASKFLAG_BLUE,bMask);
	}
	bool getMaskRed() const {
		return getFlag(MASKFLAG_RED);
	}
	bool getMaskGreen() const {
		return getFlag(MASKFLAG_GREEN);
	}
	bool getMaskBlue() const {
		return getFlag(MASKFLAG_BLUE);
	}
	bool getMaskAlpha() const {
		return getFlag(MASKFLAG_ALPHA);
	}
};

class mtrStageAPI_i  { 
public:
	virtual class textureAPI_i *getTexture(float curTimeSec = 0.f) const = 0;
	virtual class textureAPI_i *getTextureForFrameNum(u32 frameNum) const = 0;
	virtual enum alphaFunc_e getAlphaFunc() const = 0;
	virtual const struct blendDef_s &getBlendDef() const = 0;
	virtual class cubeMapAPI_i *getCubeMap() const = 0;
	virtual class mtrStageAPI_i *getBumpMap() const = 0;
	virtual class mtrStageAPI_i *getHeightMap() const = 0;
	virtual class mtrStageAPI_i *getSpecularMap() const = 0;
	virtual class mtrStageAPI_i *getBumpHeightMap() const = 0;
	virtual class mtrStageAPI_i *getBlendChannelRed() const = 0;
	virtual class mtrStageAPI_i *getBlendChannelBlue() const = 0;
	virtual class mtrStageAPI_i *getBlendChannelGreen() const = 0;
	virtual bool hasTexMods() const = 0;
	virtual void applyTexMods(class matrix_c &out, float curTimeSec, const class astInputAPI_i *in) const = 0;
	virtual bool hasTexGen() const = 0;
	virtual enum texCoordGen_e getTexGen() const = 0;
	virtual enum stageType_e getStageType() const = 0;
	virtual bool hasRGBGen() const = 0;
	virtual enum rgbGen_e getRGBGenType() const = 0;
	virtual enum alphaGen_e getAlphaGenType() const = 0;
	virtual bool getRGBGenConstantColor3f(float *out3Floats) const = 0;
	virtual float getRGBGenWaveValue(float curTimeSec) const = 0;
	virtual bool getDepthWrite() const = 0;
	virtual void evaluateRGBGen(const class astInputAPI_i *in, float *out3Floats) const = 0;
	virtual float evaluateAlphaGen(const class astInputAPI_i *in) const = 0;
	// for Qio specular mapping
	virtual float evaluateShininess(const class astInputAPI_i *in) const = 0;
	// return true if stage is conditional (has Doom3 'if' condition)
	virtual bool hasIFCondition() const = 0;
	// return true if drawing condition is met for given input variables
	virtual bool conditionMet(const class astInputAPI_i *in) const = 0;
	// glColorMask parameters for Doom3 materials
	virtual bool getColorMaskRed() const = 0;
	virtual bool getColorMaskGreen() const = 0;
	virtual bool getColorMaskBlue() const = 0;
	virtual bool getColorMaskAlpha() const = 0;
	// doom3 alphaTest AST evaluation
	virtual float evaluateAlphaTestValue(const class astInputAPI_i *in) const = 0;
	virtual bool isUsingCustomProgram() const = 0;
	// material editing?
	virtual void setRGBGenVerex() = 0;
};

#endif // __MTRSTAGE_API_H__
