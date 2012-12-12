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
// backEndDX9API.cpp
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/inputSystemAPI.h>
#include <api/sysEventCasterAPI.h>
#include <api/rbAPI.h>
#include <api/textureAPI.h>
#include <api/mtrStageAPI.h>
#include <api/mtrAPI.h>
#include <api/sdlSharedAPI.h>

#include <shared/r2dVert.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <renderer/rVertexBuffer.h>
#include <renderer/rIndexBuffer.h>
#include <renderer/rPointBuffer.h>
#include <materialSystem/mat_public.h> // alphaFunc_e etc
#include <api/rLightAPI.h>

#include <shared/cullType.h>

#include "dx9_local.h"
#include "dx9_shader.h"

#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#else
#	include <SDL.h>
#endif


#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

const DWORD R2DVERT_FVF = D3DFVF_XYZ | D3DFVF_TEX2;
const DWORD RVERT_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2;
// shadow volume vertices dont need anything else
const DWORD RSHADOWVOLUMEVERT_FVF = D3DFVF_XYZ;

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
inputSystemAPI_i * g_inputSystem = 0;
//sysEventCasterAPI_c *g_sysEventCaster = 0;
sdlSharedAPI_i *g_sharedSDLAPI = 0;

// I need them in dx9_shader.cpp
IDirect3D9 *pD3D;
IDirect3DDevice9 *pDev;

// these are the vertex declarations for HLSL shaders ONLY
D3DVERTEXELEMENT9 dx_rVertexDecl[] =
{
    {0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	{0, 24, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,   0},
    {0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
 //   {0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
};

class rbDX9_c : public rbAPI_i {
	HWND hWnd;
	int dxWidth, dxHeight;
	mtrAPI_i *lastMat;
	textureAPI_i *lastLightmap;
	IDirect3DVertexDeclaration9 *rVertDecl;
	bool usingWorldSpace;
	bool bDrawingShadowVolumes;
public:
	rbDX9_c() {
		hWnd = 0;
		lastMat = 0;
		lastLightmap = 0;
		rVertDecl = 0;
	}
	virtual backEndType_e getType() const {
		return BET_DX9;
	}
	void initDX9State() {
		pDev->SetRenderState(D3DRS_ZENABLE, true);
		pDev->SetRenderState(D3DRS_AMBIENT,RGB(255,255,255));
		pDev->SetRenderState(D3DRS_LIGHTING,false);
		pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		pDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL); // less or EQUAL - for multistage materials

		pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);                
		pDev->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC); 
		
		//pDev->SetRenderState( D3DRS_STENCILREF,       0 );
		//pDev->SetRenderState( D3DRS_STENCILMASK,       0 );
		//pDev->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
		//pDev->SetRenderState( D3DRS_CCW_STENCILFUNC,  D3DCMP_ALWAYS );
		//pDev->SetRenderState( D3DRS_CCW_STENCILZFAIL,  D3DSTENCILOP_KEEP );
		//pDev->SetRenderState( D3DRS_CCW_STENCILFAIL,  D3DSTENCILOP_KEEP );
		//pDev->SetRenderState( D3DRS_CCW_STENCILPASS,  D3DSTENCILOP_DECR );
	}

	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap) {
		lastMat = mat;
		lastLightmap = lightmap;
	}
	virtual void unbindMaterial() {
		lastMat = 0;
		disableLightmap();
		disableBlendFunc();
	}
	virtual void setColor4(const float *rgba) {
	}
	virtual void setBindVertexColors(bool bBindVertexColors) {
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) {
		disableLightmap();

		pDev->SetFVF(R2DVERT_FVF);
		setCull(CT_FRONT_SIDED);

		setBlendFunc(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);

		if(lastMat) {
			const mtrStageAPI_i *s = lastMat->getStage(0);
			IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)s->getTexture()->getInternalHandleV();
			pDev->SetTexture(0,texDX9);
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,numVerts,numIndices/3,indices,D3DFMT_INDEX16,
				verts,sizeof(r2dVert_s));
		} else {
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,numVerts,numIndices/3,indices,D3DFMT_INDEX16,
				verts,sizeof(r2dVert_s));
		}
	}
	void setLightmap(IDirect3DTexture9 *lightmapDX9) {
		pDev->SetTexture(1,lightmapDX9);

		pDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
		pDev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);

		pDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE4X );
		//pDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED );
		pDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		pDev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
		pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);

		pDev->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		pDev->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	}
	void disableLightmap() {
		pDev->SetTexture(1,0);
	}
	//
	// alphaFunc changes
	//
	alphaFunc_e prevAlphaFunc;
	void setAlphaFunc(alphaFunc_e newAlphaFunc) {
		if(prevAlphaFunc == newAlphaFunc) {
			return; // no change
		}
		if(newAlphaFunc == AF_NONE) {
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, false );			
		} else if(newAlphaFunc == AF_GT0) {
			pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000000 );
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
			pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );	
		} else if(newAlphaFunc == AF_GE128) {
			pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000080 );
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
			pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );	
		} else if(newAlphaFunc == AF_LT128) {
			pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000080 );
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
			pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL );			
		}
		prevAlphaFunc = newAlphaFunc;
	}
	void turnOffAlphaFunc() {
		setAlphaFunc(AF_NONE);
	}
	int blendModeEnumToDX9Blend(int in) {
		static int blendTable [] = {
			0, // BM_NOT_SET
			D3DBLEND_ZERO, // 0
			D3DBLEND_ONE, // 1
			D3DBLEND_INVSRCCOLOR, // 2
			D3DBLEND_INVDESTCOLOR, // 3
			D3DBLEND_INVSRCALPHA, // 4
			D3DBLEND_INVDESTALPHA, // 5 
			D3DBLEND_DESTCOLOR, // 6
			D3DBLEND_DESTALPHA, // 7 
			D3DBLEND_SRCCOLOR, //8 
			D3DBLEND_SRCALPHA, // 9
			D3DBLEND_ZERO//GL_SRC_ALPHA_SATURATE,
		};
		return blendTable[in];
	}
	short blendSrc;
	short blendDst;
	void setBlendFunc( short src, short dst ) {
		if( blendSrc != src || blendDst != dst ) {
			blendSrc = src;
			blendDst = dst;
			if(src == BM_NOT_SET && dst == BM_NOT_SET) {
				pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
				//pDev->SetRenderState(D3DRS_ZENABLE, true);
			} else {
				pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
				//pDev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
				//pDev->SetRenderState(D3DRS_ZENABLE, false);
				pDev->SetRenderState(D3DRS_SRCBLEND, blendModeEnumToDX9Blend(src));
				pDev->SetRenderState(D3DRS_DESTBLEND, blendModeEnumToDX9Blend(dst));
			}
		}
	}
	void disableBlendFunc() {
		setBlendFunc(BM_NOT_SET,BM_NOT_SET);
	}	
	// D3DRS_STENCILENABLE state
	void setDX9StencilTest(bool bEnable) {
		pDev->SetRenderState(D3DRS_STENCILENABLE,bEnable);
	}
	// glDepthMask equivalent
	// disable/enable writing to depth buffer
	void setDX9DepthMask(bool bEnable) {
		pDev->SetRenderState(D3DRS_ZWRITEENABLE,bEnable);
	}
	// GL_CULL
	cullType_e prevCullType;
	void setCull(cullType_e cullType) {
		if(prevCullType == cullType) {
			return;
		}
		prevCullType = cullType;
		if(cullType == CT_TWO_SIDED) {
			pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		} else {
			if(cullType == CT_BACK_SIDED) {
				pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
			} else {
				pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
			}
		}
	}
	const rLightAPI_i *curLight;
	bool bDrawOnlyOnDepthBuffer;
	virtual void setCurLight(const class rLightAPI_i *light) {
		this->curLight = light;
	}
	virtual void setBDrawOnlyOnDepthBuffer(bool bNewDrawOnlyOnDepthBuffer) {
		bDrawOnlyOnDepthBuffer = bNewDrawOnlyOnDepthBuffer;
	}
	inline void drawIndexedTrimeshInternal(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		if(verts.getInternalHandleVoid() && indices.getInternalHandleVoid()) {
			pDev->SetIndices((IDirect3DIndexBuffer9 *)indices.getInternalHandleVoid());
			pDev->SetStreamSource(0,(IDirect3DVertexBuffer9*)verts.getInternalHandleVoid(),0,sizeof(rVert_c));
			pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,verts.size(),0,indices.getNumIndices()/3);
		} else {
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,verts.size(),indices.getNumIndices()/3,
				indices.getArray(),
				indices.getDX9IndexType(),verts.getArray(),sizeof(rVert_c));
		}
	}
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		pDev->SetFVF(RVERT_FVF);

		stopDrawingShadowVolumes();
		setCull(CT_FRONT_SIDED);

		if(bDrawOnlyOnDepthBuffer) {
			turnOffAlphaFunc();

			// set the color mask
			pDev->SetRenderState(D3DRS_COLORWRITEENABLE,0);
			drawIndexedTrimeshInternal(verts,indices);
			return;
		}
		// set the color mask
		pDev->SetRenderState(D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);

		if(lastMat) {
			if(curLight) {
				hlslShader_c *sh = DX9_RegisterShader("perPixelLighting");
				initRVertDecl();
				if(sh && rVertDecl) {
					pDev->SetVertexDeclaration(rVertDecl);
					if(verts.getInternalHandleVoid() && indices.getInternalHandleVoid()) {
						pDev->SetStreamSource(0,(IDirect3DVertexBuffer9*)verts.getInternalHandleVoid(),0,sizeof(rVert_c));
						pDev->SetIndices((IDirect3DIndexBuffer9 *)indices.getInternalHandleVoid());
					} 
					//D3DXHANDLE hTechnique = sh->effect->GetTechniqueByName("SimpleTexturing");
					D3DXHANDLE hTechnique = sh->effect->GetTechnique(0);
					sh->effect->SetTechnique(hTechnique);
					D3DXMATRIX worldViewProjectionMatrix;
					worldViewProjectionMatrix = dxWorld * dxView * dxProj;
					sh->effect->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
					IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)lastMat->getStage(0)->getTexture()->getInternalHandleV();
					sh->effect->SetTexture("colorMapTexture", texDX9);

					const vec3_c &xyz = curLight->getOrigin();
					if(usingWorldSpace) {
						sh->effect->SetValue("lightOrigin", xyz, sizeof(vec3_c));
					} else {
						matrix_c entityMatrixInv = worldMatrix.getInversed();
						vec3_c xyzLocal;
						entityMatrixInv.transformPoint(xyz,xyzLocal);
						sh->effect->SetValue("lightOrigin", xyzLocal, sizeof(vec3_c));
					}
					sh->effect->SetFloat("lightRadius", curLight->getRadius());
					u32 numDX9EffectPasses;
					sh->effect->Begin(&numDX9EffectPasses,0);
					for(u32 i = 0; i < numDX9EffectPasses; i++) {
						sh->effect->BeginPass(i);
						if(verts.getInternalHandleVoid() && indices.getInternalHandleVoid()) {
							pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,verts.size(),0,indices.getNumIndices()/3);
						} else {
							pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,verts.size(),indices.getNumIndices()/3,
								indices.getArray(),
								indices.getDX9IndexType(),verts.getArray(),sizeof(rVert_c));
						}
						sh->effect->EndPass();
					}
					sh->effect->End();
					return;
				}	
			}
			for(u32 i = 0; i < lastMat->getNumStages(); i++) {
				const mtrStageAPI_i *s = lastMat->getStage(i);
				IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)s->getTexture()->getInternalHandleV();
				
				// set alphafunc (for grates, etc)
				setAlphaFunc(s->getAlphaFunc());
				// set blendfunc (for particles, etc)
				if(curLight) {
					// light interactions are appended with addictive blending
					setBlendFunc(BM_ONE,BM_ONE);
				} else {
					const blendDef_s &bd = s->getBlendDef();
					setBlendFunc(bd.src,bd.dst);
				}
				// bind colormap (from material stage)
				pDev->SetTexture(0,texDX9);
				// bind lightmap (only for BSP planar surfaces and bezier patches)
				if(lastLightmap) {
					setLightmap((IDirect3DTexture9 *)lastLightmap->getInternalHandleV());
				} else {
					disableLightmap();
				}

				drawIndexedTrimeshInternal(verts,indices);
			}
		}
	}	
	virtual void drawElementsWithSingleTexture(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, class textureAPI_i *tex) {

	}	
	void startDrawingShadowVolumes() {
		if(bDrawingShadowVolumes == true)
			return;
		unbindMaterial();
		pDev->Clear(0, 0, D3DCLEAR_STENCIL, 0, 1, 0); // clear the stencil buffer before drawing new light
		pDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESS); // We change the z-testing function to LESS, to avoid little bugs in shadow
		pDev->SetRenderState(D3DRS_COLORWRITEENABLE,0);
		pDev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS); // always draw to stencil buffer
		pDev->SetRenderState(D3DRS_STENCILREF, 0);
		pDev->SetRenderState(D3DRS_STENCILMASK, 0);
		setDX9DepthMask(false);
		setDX9StencilTest(true);

		bDrawingShadowVolumes = true;
	}
	void stopDrawingShadowVolumes() {
		if(bDrawingShadowVolumes == false)
			return;

		// We draw our lighting now that we created the shadows area in the stencil buffer
		pDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL); // we put it again to LESS or EQUAL (or else you will get some z-fighting)
		setCull(CT_FRONT_SIDED); // we draw the front face
		pDev->SetRenderState(D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);

		pDev->SetRenderState( D3DRS_STENCILREF, 0 );
		pDev->SetRenderState( D3DRS_STENCILMASK, 0xffffffff );
		pDev->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
		pDev->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
		pDev->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
		pDev->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
		pDev->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_EQUAL );

		bDrawingShadowVolumes = false;
	}
	virtual void drawIndexedShadowVolume(const class rPointBuffer_c *points, const class rIndexBuffer_c *indices) {
		startDrawingShadowVolumes();

		pDev->SetFVF(RSHADOWVOLUMEVERT_FVF);

		// TODO: incr/decr when a depth test fail
		// the current code causes problems when a viewer eye is inside shadow volume

		setCull(CT_FRONT_SIDED);

		pDev->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );

		pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,points->size(),indices->getNumIndices()/3,indices->getArray(),indices->getDX9IndexType(),
			points->getArray(),sizeof(hashVec3_c)); // draw the shadow volume

		setCull(CT_BACK_SIDED);

		pDev->SetRenderState(   D3DRS_STENCILPASS, D3DSTENCILOP_DECR );

		pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,points->size(),indices->getNumIndices()/3,indices->getArray(),indices->getDX9IndexType(),
			points->getArray(),sizeof(hashVec3_c)); // draw the shadow volume
	}
	virtual void beginFrame() {
		pDev->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0);
		pDev->BeginScene();
	}
	virtual void endFrame() {
		pDev->EndScene();
		//g_sharedSDLAPI->endFrame();
		pDev->Present(0, 0, 0, 0);
	}
	virtual void clearDepthBuffer() {
		pDev->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1, 0);
	}
	virtual void setup2DView() {
		setDX9StencilTest(false);
		setDX9DepthMask(true);

		D3DVIEWPORT9 vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = dxWidth;
		vp.Height = dxHeight;
		vp.MinZ = 0.f;
		vp.MaxZ = 1.f;
		pDev->SetViewport(&vp);
		D3DXMATRIX ortho;
		//D3DXMatrixOrthoRH(&ortho,win_width,win_height,0,1);
		D3DXMatrixOrthoOffCenterLH(&ortho,0,dxWidth,dxHeight,0,0,1);
		pDev->SetTransform(D3DTS_PROJECTION,&ortho);

		D3DXMATRIX id;
		D3DXMatrixIdentity(&id);
		pDev->SetTransform(D3DTS_WORLD,&id);
		pDev->SetTransform(D3DTS_VIEW,&id);
	}
	D3DXMATRIX dxView, dxWorld, dxProj;

	matrix_c viewMatrix;
	matrix_c worldMatrix;
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &camAxis) {
		// transform by the camera placement and view axis
		viewMatrix.invFromAxisAndVector(camAxis,newCamPos);
		// convert to gl coord system
		viewMatrix.toGL();

		dxView = *(const D3DMATRIX *)&viewMatrix;
		pDev->SetTransform(D3DTS_VIEW, (const D3DMATRIX *)&viewMatrix);

		setupWorldSpace();
	}
	virtual void setupProjection3D(const struct projDef_s *pd) {
		matrix_c proj;
		proj.setupProjection(pd->fovX,pd->fovY,pd->zNear,pd->zFar);

		dxProj = *(const D3DMATRIX *)&proj;
		pDev->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX *)&proj);
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
		worldMatrix.identity();

		dxWorld = *(const D3DMATRIX *)&worldMatrix;
		pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&worldMatrix);

		usingWorldSpace = true;
	}
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
		worldMatrix.fromAxisAndOrigin(axis,origin);

		dxWorld = *(const D3DMATRIX *)&worldMatrix;
		pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&worldMatrix);
		
		usingWorldSpace = false;
	}
	// same as above but with angles instead of axis
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) {
	}

	virtual u32 getWinWidth() const  {
		return dxWidth;
	}
	virtual u32 getWinHeight() const  {
		return dxHeight;
	}

	virtual void uploadTextureRGBA(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
		IDirect3DTexture9 *tex = 0;
		HRESULT hr = pDev->CreateTexture(w,h,0,D3DUSAGE_AUTOGENMIPMAP,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&tex,0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadTexture: pDev->CreateTexture failed\n");
			return;
		}
		D3DLOCKED_RECT rect;
		hr = tex->LockRect(0, &rect, 0, 0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadTexture: tex->LockRect failed\n");
			return;
		}
#if 0
		memcpy(rect.pBits,pic,w*h*4);
#else
		byte *outDXData = (byte*)rect.pBits;
		for(u32 i = 0; i < w; i++) {
			for(u32 j = 0; j < h; j++) {
				const byte *inPixel = data + (i * h + j)*4;
				byte *outPixel = outDXData + (i * h + j)*4;
				outPixel[0] = inPixel[2];
				outPixel[1] = inPixel[1];
				outPixel[2] = inPixel[0];
				outPixel[3] = inPixel[3];
			}
		}
#endif
		hr = tex->UnlockRect(0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadTexture: tex->UnlockRect(0) failed\n");
			return;
		}
		out->setWidth(w);
		out->setHeight(h);
		out->setInternalHandleV(tex);
	}
	virtual void uploadLightmapRGB(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
		IDirect3DTexture9 *tex = 0;
		// lightmaps dont have alpha channel
		HRESULT hr = pDev->CreateTexture(w,h,0,D3DUSAGE_AUTOGENMIPMAP,D3DFMT_X8R8G8B8,D3DPOOL_MANAGED,&tex,0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadLightmap: pDev->CreateTexture failed\n");
			return;
		}
		D3DLOCKED_RECT rect;
		hr = tex->LockRect(0, &rect, 0, 0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadLightmap: tex->LockRect failed\n");
			return;
		}
		byte *outPicData = (byte*)rect.pBits;
		//memcpy(rect.pBits,pic,w*h*4);
		for(u32 i = 0; i < w; i++) {
			for(u32 j = 0; j < h; j++) {
				const byte *inPixel = data + (i * h + j)*3;
				byte *outPixel = outPicData + (i * h + j)*4;
				outPixel[0] = inPixel[0];
				outPixel[1] = inPixel[1];
				outPixel[2] = inPixel[2];
				outPixel[3] = 255;
			}
		}
		hr = tex->UnlockRect(0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadLightmap: tex->UnlockRect(0) failed\n");
			return;
		}
		out->setWidth(w);
		out->setHeight(h);
		out->setInternalHandleV(tex);
	}
	virtual void freeTextureData(class textureAPI_i *tex) {
		IDirect3DTexture9 *texD9 = (IDirect3DTexture9 *)tex->getInternalHandleV();
		if(texD9 == 0)
			return;
		texD9->Release();
		tex->setInternalHandleV(0);
	}

	void initRVertDecl() {
		if(rVertDecl)
			return;
		HRESULT hr = pDev->CreateVertexDeclaration(dx_rVertexDecl, &rVertDecl);
	}

	// vertex buffers (VBOs)
	virtual bool createVBO(class rVertexBuffer_c *ptr) {
		IDirect3DVertexBuffer9 *vbo = 0;
		HRESULT createResult = pDev->CreateVertexBuffer(ptr->getSizeInBytes(), D3DUSAGE_WRITEONLY, RVERT_FVF, D3DPOOL_MANAGED, &vbo, NULL);
		if (FAILED(createResult)) {
			g_core->RedWarning("refApiDX9_c::createVBO: pDev->CreateVertexBuffer failed\n");
			return true;
		}
		void *bufData;
		HRESULT lockResult = vbo->Lock(0,0,&bufData,0);
		if (FAILED(lockResult)) {
			g_core->RedWarning("refApiDX9_c::createVBO: vbo->Lock failed\n");
			return true; 
		}
		memcpy(bufData,ptr->getArray(),ptr->getSizeInBytes());
		HRESULT unlockResult = vbo->Unlock();
		if (FAILED(unlockResult)) {
			g_core->RedWarning("refApiDX9_c::createVBO: vbo->Unlock() failed\n");
			return true;
		}
		ptr->setInternalHandleVoid((void*)vbo);
		return false;
	}
	virtual bool destroyVBO(class rVertexBuffer_c *ptr) {
		if(ptr->getInternalHandleVoid() == 0)
			return true;
		IDirect3DVertexBuffer9 *vbo = (IDirect3DVertexBuffer9 *)ptr->getInternalHandleVoid();
		vbo->Release();
		ptr->setInternalHandleVoid(0);
		return false;
	}

	// index buffers (IBOs)
	virtual bool createIBO(class rIndexBuffer_c *ptr) {	
		IDirect3DIndexBuffer9 *ibo = 0;
		HRESULT createResult = pDev->CreateIndexBuffer(ptr->getSizeInBytes(), D3DUSAGE_WRITEONLY, ptr->getDX9IndexType(), D3DPOOL_MANAGED, &ibo, NULL);
		if (FAILED(createResult)) {
			g_core->RedWarning("refApiDX9_c::createIBO: pDev->CreateIndexBuffer failed\n");
			return true;
		}
		void *bufData;
		HRESULT lockResult = ibo->Lock(0,0,&bufData,0);
		if (FAILED(lockResult)) {
			g_core->RedWarning("refApiDX9_c::createIBO: ibo->Lock failed\n");
			return true;
		}
		memcpy(bufData,ptr->getArray(),ptr->getSizeInBytes());
		HRESULT unlockResult = ibo->Unlock();
		if (FAILED(unlockResult)) {
			g_core->RedWarning("refApiDX9_c::createIBO: ibo->Unlock() failed\n");
			return true;
		}
		ptr->setInternalHandleVoid(ibo);
		return false;
	}
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) {
		if(ibo->getInternalHandleVoid() == 0)
			return false;
		IDirect3DIndexBuffer9 *ib = (IDirect3DIndexBuffer9 *)ibo->getInternalHandleVoid();
		ib->Release();
		ibo->setInternalHandleVoid(0);
		return false;
	}

	virtual void init() {
		// init SDL window
		g_sharedSDLAPI->init();

		// hack to get HWND (that wasnt needed for GL!)
		// I hope it won't cause any bugs
		hWnd = GetActiveWindow();

		ShowWindow(hWnd, 5);

		// create Dx9 device
		pD3D = Direct3DCreate9(D3D_SDK_VERSION);

		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Windowed = true;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		// this turns off V-sync (60 fps limit)
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		dxWidth = d3dpp.BackBufferWidth = g_sharedSDLAPI->getWinWidth(); // CreateDevice will crash if its set to 0 while Windowed is set to true
		dxHeight = d3dpp.BackBufferHeight = g_sharedSDLAPI->getWinHeigth();

		// add z buffer for depth tests
		d3dpp.EnableAutoDepthStencil = true;
		//d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

		pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, 
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDev);

		ShowWindow(hWnd,SW_SHOW);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);

		curLight = 0;
		bDrawOnlyOnDepthBuffer = false;
		rVertDecl = 0;
		bDrawingShadowVolumes = false;
		prevCullType = CT_NOT_SET;
		 

		initDX9State();

		// This depends on SDL_INIT_VIDEO, hence having it here
		g_inputSystem->IN_Init();
	}
	virtual void shutdown(bool destroyWindow) {
		lastMat = 0;
		lastLightmap = 0;

		pDev->Release();
		pD3D->Release();
		if(rVertDecl) {
			rVertDecl->Release();
			rVertDecl = 0;
		}	
		DX9_ShutdownHLSLShaders();
		hWnd = 0;
		pDev = 0;
		pD3D = 0;
		if(destroyWindow) {
			g_sharedSDLAPI->shutdown();
		}
	}
};


static rbDX9_c g_staticDX9Backend;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)&g_staticDX9Backend,RENDERER_BACKEND_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_inputSystem,INPUT_SYSTEM_API_IDENTSTR);
	//g_iFaceMan->registerIFaceUser(&g_sysEventCaster,SYSEVENTCASTER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_sharedSDLAPI,SHARED_SDL_API_IDENTSTRING);
}

qioModule_e IFM_GetCurModule() {
	return QMD_REF_BACKEND_DX9;
}

