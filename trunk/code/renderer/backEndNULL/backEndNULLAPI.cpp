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
// backEndNULLAPI.cpp
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/inputSystemAPI.h>
#include <api/rbAPI.h>

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
inputSystemAPI_i * g_inputSystem = 0;

class rbNULL_c : public rbAPI_i {
public:
	virtual backEndType_e getType() const {
		return BET_NULL;
	}
	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap, class textureAPI_i *deluxemap) {
	}
	virtual void unbindMaterial() {
	}
	virtual void setColor4(const float *rgba) {
	}
	virtual void setBindVertexColors(bool bBindVertexColors) {
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) {
	}
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
	}	
	virtual void drawElementsWithSingleTexture(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, class textureAPI_i *tex) {

	}
	virtual void beginFrame() {
	}
	virtual void endFrame() {
	}
	virtual void clearDepthBuffer() {

	}
	virtual void setup2DView() {
	}
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &camAxis) {
	}
	virtual void setupProjection3D(const struct projDef_s *pd = 0) {
	}
	virtual void drawCapsuleZ(const float *xyz, float h, float w) {
	}
	virtual void drawBoxHalfSizes(const float *halfSizes) {
	}
	virtual void drawLineFromTo(const float *from, const float *to, const float *colorRGB) {
	}
	virtual void drawBBLines(const class aabb &bb) {
	}
	// used while drawing world surfaces and particles
	virtual void setupWorldSpace() {
	}
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
	}
	// same as above but with angles instead of axis
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) {
	}

	virtual u32 getWinWidth() const  {
		return 1;
	}
	virtual u32 getWinHeight() const  {
		return 1;
	}

	virtual void uploadTextureRGBA(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
	}
	virtual void uploadLightmap(class textureAPI_i *out, const byte *data, u32 w, u32 h, bool rgba) {
	}
	virtual void freeTextureData(class textureAPI_i *tex) {
	}

	// vertex buffers (VBOs)
	virtual bool createVBO(class rVertexBuffer_c *vbo) {
		return false;
	}
	virtual bool destroyVBO(class rVertexBuffer_c *vbo) {
		return false;
	}

	// index buffers (IBOs)
	virtual bool createIBO(class rIndexBuffer_c *ibo) {
		return false;
	}
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) {
		return false;
	}

	virtual void init() {
	}
	virtual void shutdown(bool destroyWindow) {
	}
};


static rbNULL_c g_staticNULLBackend;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)&g_staticNULLBackend,RENDERER_BACKEND_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_inputSystem,INPUT_SYSTEM_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_REF_BACKEND_NULL;
}

