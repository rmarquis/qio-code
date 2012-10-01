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
// rbAPI.h - renderer backend (openGL / DirectX) interface
// (low-level drawing routines ONLY)

#ifndef __RB_API_H__
#define __RB_API_H__

#include "iFaceBase.h"

#define RENDERER_BACKEND_API_IDENTSTR "RendererBackendAPI0001"
#define RB_SDLOPENGL_API_IDENTSTR "RB_SDLOpenGL_API0001"

class rbAPI_i : public iFaceBase_i {
public:
	virtual void setMaterial(class mtrAPI_i *mat) = 0;
	virtual void setColor4(const float *rgba) = 0;
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) = 0;
	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	virtual void setup2DView() = 0;

	virtual u32 getWinWidth() const = 0;
	virtual u32 getWinHeight() const = 0;

	virtual void init() = 0;
	virtual void shutdown() = 0;
};

extern rbAPI_i *rb;

#endif // __RB_API_H__
