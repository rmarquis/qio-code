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

#include <shared/typedefs.h>
#include "iFaceBase.h"

#define RENDERER_BACKEND_API_IDENTSTR "RendererBackendAPI0001"

enum backEndType_e {
	BET_GL,
	BET_NULL,
	BET_DX9,
	BET_DX10
};

struct imageData_s {
	u32 w, h;
	byte *pic;
};

class rbAPI_i : public iFaceBase_i {
public:
	virtual backEndType_e getType() const = 0;
	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap = 0, class textureAPI_i *deluxemap = 0) = 0;
	virtual void unbindMaterial() = 0;
	virtual void setColor4(const float *rgba) = 0;
	virtual void setBindVertexColors(bool bBindVertexColors) = 0;
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) = 0;
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) = 0;
	virtual void drawElements_wireframe(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) { }
	virtual void drawElementsWithSingleTexture(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, class textureAPI_i *tex) = 0;
	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	virtual void clearDepthBuffer() = 0;
	virtual void setup2DView() = 0;
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &camAxis) = 0;
	virtual void setupProjection3D(const struct projDef_s *pd = 0) = 0;
	virtual void drawCapsuleZ(const float *xyz, float h, float w) = 0;
	virtual void drawBoxHalfSizes(const float *halfSizes) = 0;
	virtual void drawLineFromTo(const float *from, const float *to, const float *colorRGB, float lineWidth = 1.f) = 0;
	virtual void drawBBLines(const class aabb &bb) = 0;
	virtual void drawWinding(const class vec3_c *p, u32 numPoints, u32 stride = 12) { };
	virtual void drawWindingTextured(const class vec3_c *p, const class vec2_c *tc, u32 numPoints, u32 stride = 12) { };

	// used while drawing world surfaces and particles
	virtual void setupWorldSpace() = 0;
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) = 0;
	// same as above but with angles instead of axis
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) = 0;

	// window size
	virtual u32 getWinWidth() const = 0;
	virtual u32 getWinHeight() const = 0;

	// screen buffer access
	virtual byte *getScreenShotRGB(u32 *w, u32 *h) const { return 0; }
	virtual void freeScreenShotData(byte *b) { };

	virtual void setViewPort(u32 newWidth, u32 newHeight) { }

	virtual void setBSkipStaticEnvCubeMapStages(bool newBSkipStaticEnvCubeMapStages) { }

	// 2D textures
	virtual void uploadTextureRGBA(class textureAPI_i *out, const byte *data, u32 w, u32 h) = 0;
	virtual void uploadLightmap(class textureAPI_i *out, const byte *data, u32 w, u32 h, bool rgba = false) = 0;
	virtual void freeTextureData(class textureAPI_i *tex) = 0;
	// cubemap textures
	virtual bool uploadCubeMap(class cubeMapAPI_i *out, const imageData_s *in) { return true; }
	virtual void freeCubeMap(class cubeMapAPI_i *cm) { }

	// vertex buffers (VBOs)
	virtual bool createVBO(class rVertexBuffer_c *vbo) = 0;
	virtual bool destroyVBO(class rVertexBuffer_c *vbo) = 0;

	// index buffers (IBOs)
	virtual bool createIBO(class rIndexBuffer_c *ibo) = 0;
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) = 0;

	// experimental lighting system
	virtual void setCurLight(const class rLightAPI_i *light) {

	}
	virtual void setBDrawOnlyOnDepthBuffer(bool bNewDrawOnlyOnDepthBuffer) {

	}
	virtual void setBDrawingSunShadowMapPass(bool bNewDrawingSunShadowMapPass, int newSunShadowMapLOD) {

	}
	virtual void drawIndexedShadowVolume(const class rPointBuffer_c *points, const class rIndexBuffer_c *indices) {

	}

	virtual void setRenderTimeSeconds(float newCurTime) {

	}
	virtual void setIsMirror(bool newBIsMirror) {

	}
	virtual void setPortalDepth(u32 portalDepth) {

	}
	virtual void setPortalClipPlane(const class plane_c &pl, bool bEnabled) {

	}
	virtual void setCurrentDrawCallSort(enum drawCallSort_e sort) {

	}
	virtual void setRShadows(int newRShadows) {

	}
	// for shadow mapping
	virtual void setSunShadowBounds(const class aabb bounds[]) {

	}
	virtual void setCurrentDrawCallCubeMapSide(int iCubeSide) {

	}
	virtual void setCurLightShadowMapSize(int newW, int newH) {

	}
	virtual void setSunParms(bool bHasSunLight, const class vec3_c &sunColor, const class vec3_c &sunDirection, const class aabb &newSunBounds) {

	}
	// -1 means that renderer material time will be used to get "animMap" frame
	virtual void setForcedMaterialMapFrame(int animMapFrame) {

	}
	// returns true if "texgen environment" q3 shader effect can be done on GPU
	virtual bool gpuTexGensSupported() const {
		return false;
	}
	virtual bool areTangentsNeededForMaterial(const class mtrAPI_i *mat) const {
		return true;
	}
	virtual class occlusionQueryAPI_i *allocOcclusionQuery() {
		return 0;
	}
	virtual void beginDrawingSky() {

	}
	virtual void endDrawingSky() {

	}

	// V: bCreateWindow is for my built-in Radiant fork, which is using Qio's materialsystem
	virtual void init(bool bCreateWindow = true) = 0;
	virtual void shutdown(bool destroyWindow) = 0;
	// in editor mode qer_editorImages stages should be drawn
	// otherwise they are skipped
	virtual void setBDrawEditorImageStages(bool newBDraw) { }
	// for selected surface highlights..
	virtual void setAdditiveBlending() { }
};

extern rbAPI_i *rb;

#include <math/math.h>
// projection matrix (camera eye) definition
struct projDef_s {
	float fovX;
	// fovY will be automatically calculated from window aspect
	float fovY;
	float zFar;
	float zNear;

	void calcFovYForViewPort(float viewPortW, float viewPortH) {
		float x = viewPortW / tan( this->fovX / 360 * M_PI );
		this->fovY = atan2( viewPortH, x ) * 360 / M_PI;
	}
	void calcFovY() {
		float viewPortW = rb->getWinWidth();
		float viewPortH = rb->getWinHeight();
		calcFovYForViewPort(viewPortW,viewPortH);
	}
	void setDefaults() {
		fovX = 80;
		calcFovY();
		zFar = 8192.f;
		zNear = 1.f;
	}
	float getZFar() const {
		return zFar;
	}
};

#endif // __RB_API_H__
