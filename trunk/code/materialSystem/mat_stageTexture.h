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
// mat_stageTexture.h
#ifndef __MAT_STAGETEXTURE_H__
#define __MAT_STAGETEXTURE_H__

#include <shared/array.h>
#include <shared/str.h>

// for animated textures (sprites)
class textureAnimation_c {
	arraySTD_c<str> texNames;
	arraySTD_c<class textureAPI_i *> textures;
	float frequency;
public:
	
	void uploadTextures();
	void unloadTextures();
	~textureAnimation_c();
	bool parseAnimMap(class parser_c &p);
	textureAPI_i *getTexture(u32 idx);
	textureAPI_i *getTextureForTime(float time);
};

class stageTexture_c {
	// for single-texture stages
	str mapName;
	textureAPI_i *singleTexture;
	bool bClamp; // use GL_CLAMP instead of GL_LINEAR. This is for "clampmap" keyword.
	// for stages with animated textures
	textureAnimation_c *animated;
public:
	stageTexture_c();
	void uploadTexture();
	void unloadTexture();
	~stageTexture_c();
	bool isAnimated() const;
	bool hasTexture() const;
	bool parseMap(parser_c &p);
	bool parseAnimMap(parser_c &p);
	bool isLightmap() const;
	void setDefaultTexture();
	// returns the singleTexture (if its present)
	// otherwise first texture of animated image
	textureAPI_i *getAnyTexture() const;
	// this function should be used by renderer backend so animated images works
	textureAPI_i *getTexture(float time = 0.f) const;
	void fromTexturePointer(textureAPI_i *newTexturePtr);
	bool isEmpty() const;
	void setBClamp(bool newBClamp);
};

#endif // __MAT_STAGETEXTURE_H__
