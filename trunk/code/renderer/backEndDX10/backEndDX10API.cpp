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
// backEndDX10API.cpp
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
#include <materialSystem/mat_public.h> // alphaFunc_e etc

#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#else
#	include <SDL.h>
#endif

#include <d3d10.h>
#include <d3dx10.h>

//
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
inputSystemAPI_i * g_inputSystem = 0;
//sysEventCasterAPI_c *g_sysEventCaster = 0;
sdlSharedAPI_i *g_sharedSDLAPI = 0;

class rbDX10_c : public rbAPI_i {
	ID3D10Device *pD3DDevice;
	IDXGISwapChain *pSwapChain;
	ID3D10RenderTargetView *pRenderTargetView;
	D3D10_VIEWPORT viewPort;
	D3DXMATRIX viewMatrix;
	D3DXMATRIX projectionMatrix;

	HWND hWnd;
	mtrAPI_i *lastMat;
	textureAPI_i *lastLightmap;
public:
	rbDX10_c() {
		hWnd = 0;
		lastMat = 0;
		lastLightmap = 0;
	}
	virtual backEndType_e getType() const {
		return BET_DX10;
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
		if(lastMat == 0)
			return;
		if(lastMat->getNumStages() == 0)
			return;
		//
		//	just display the current texture on the entire screen to test DX10 driver
		//
		// Get a pointer to the back buffer texture
		ID3D10Texture2D *pBackBuffer;
		HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D),
		 (LPVOID*)&pBackBuffer);
		if(hr != S_OK)
		{
			return;
		}
		//D3D10_BOX sourceRegion;
		//sourceRegion.left = 0;
		//sourceRegion.right = 640;
		//sourceRegion.top = 0;
		//sourceRegion.bottom = 480;
		//sourceRegion.front = 0;
		//sourceRegion.back = 1;

		const mtrStageAPI_i *s = lastMat->getStage(0);
		ID3D10Texture2D *srcTexture = (ID3D10Texture2D *)s->getTexture(0)->getInternalHandleV();
		// Copy part of a texture resource to the back buffer texture
		// The last parameter is a D3D10_BOX structure which defines the rectangle to copy to the back
		// buffer. Passing in 0 will copy the whole buffer.
		//pD3DDevice->CopySubresourceRegion(pBackBuffer, 0, 0, 0, 0, srcTexture, 0,
		//&sourceRegion);
		pD3DDevice->CopySubresourceRegion(pBackBuffer, 0, 0, 0, 0, srcTexture, 0,
			0);
	}
	void disableLightmap() {
	}
	//
	// alphaFunc changes
	//
	alphaFunc_e prevAlphaFunc;
	void setAlphaFunc(alphaFunc_e newAlphaFunc) {
		//if(prevAlphaFunc == newAlphaFunc) {
		//	return; // no change
		//}
		//if(newAlphaFunc == AF_NONE) {
		//	pDev->SetRenderState( D3DRS_ALPHATESTENABLE, false );			
		//} else if(newAlphaFunc == AF_GT0) {
		//	pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000000 );
		//	pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
		//	pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );	
		//} else if(newAlphaFunc == AF_GE128) {
		//	pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000080 );
		//	pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
		//	pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );	
		//} else if(newAlphaFunc == AF_LT128) {
		//	pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000080 );
		//	pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
		//	pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL );			
		//}
		prevAlphaFunc = newAlphaFunc;
	}
	void turnOffAlphaFunc() {
		setAlphaFunc(AF_NONE);
	}
	//int blendModeEnumToDX10Blend(int in) {
	//	static int blendTable [] = {
	//		0, // BM_NOT_SET
	//		D3DBLEND_ZERO, // 0
	//		D3DBLEND_ONE, // 1
	//		D3DBLEND_INVSRCCOLOR, // 2
	//		D3DBLEND_INVDESTCOLOR, // 3
	//		D3DBLEND_INVSRCALPHA, // 4
	//		D3DBLEND_INVDESTALPHA, // 5 
	//		D3DBLEND_DESTCOLOR, // 6
	//		D3DBLEND_DESTALPHA, // 7 
	//		D3DBLEND_SRCCOLOR, //8 
	//		D3DBLEND_SRCALPHA, // 9
	//		D3DBLEND_ZERO//GL_SRC_ALPHA_SATURATE,
	//	};
	//	return blendTable[in];
	//}
	short blendSrc;
	short blendDst;
	void setBlendFunc( short src, short dst ) {
		//if( blendSrc != src || blendDst != dst ) {
		//	blendSrc = src;
		//	blendDst = dst;
		//	if(src == BM_NOT_SET && dst == BM_NOT_SET) {
		//		pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		//		//pDev->SetRenderState(D3DRS_ZENABLE, true);
		//	} else {
		//		pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		//		//pDev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
		//		//pDev->SetRenderState(D3DRS_ZENABLE, false);
		//		pDev->SetRenderState(D3DRS_SRCBLEND, blendModeEnumToDX10Blend(src));
		//		pDev->SetRenderState(D3DRS_DESTBLEND, blendModeEnumToDX10Blend(dst));
		//	}
		//}
	}
	void disableBlendFunc() {
		setBlendFunc(BM_NOT_SET,BM_NOT_SET);
	}
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		
	}	
	virtual void drawElementsWithSingleTexture(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, class textureAPI_i *tex) {

	}
	virtual void beginFrame() {
		//clear scene
		pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0,0,0,0) );
	}
	virtual void endFrame() {
	
		//flip buffers
		pSwapChain->Present(0,0);
	}
	virtual void clearDepthBuffer() {
	
	}
	virtual void setup2DView() {
		
	}
	//matrix_c viewMatrix;
	//matrix_c worldMatrix;
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &camAxis) {
		// transform by the camera placement and view axis
	//	viewMatrix.invFromAxisAndVector(camAxis,newCamPos);
		// convert to gl coord system
	//	viewMatrix.toGL();

//		pDev->SetTransform(D3DTS_VIEW, (const D3DMATRIX *)&viewMatrix);

		setupWorldSpace();
	}
	virtual void setupProjection3D(const struct projDef_s *pd) {
	///	matrix_c proj;
	//	proj.setupProjection(pd->fovX,pd->fovY,pd->zNear,pd->zFar);
//		pDev->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX *)&proj);
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
	//	worldMatrix.identity();

	//	pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&worldMatrix);
	}
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
	//	worldMatrix.fromAxisAndOrigin(axis,origin);

	//	pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&worldMatrix);
	}
	// same as above but with angles instead of axis
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) {
	}

	virtual u32 getWinWidth() const  {
		return g_sharedSDLAPI->getWinWidth();
	}
	virtual u32 getWinHeight() const  {
		return g_sharedSDLAPI->getWinHeigth();
	}

	virtual void uploadTextureRGBA(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
		ID3D10Texture2D *tex = 0;
#if 1
		D3D10_TEXTURE2D_DESC desc; 
		ZeroMemory( &desc, sizeof(desc)); 
		desc.Width = w; 
		desc.Height = h; 
		desc.MipLevels = 1; // 0 causes INVALIDARG error
		desc.ArraySize = 1; // create a single texture
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
		desc.SampleDesc.Count = 1; 
		desc.Usage = D3D10_USAGE_DYNAMIC; 
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE; 
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE; 
		HRESULT hr = pD3DDevice->CreateTexture2D( &desc, NULL, &tex ); 
#else
		D3D10_TEXTURE2D_DESC desc;
		memset(&desc,0,sizeof(desc));
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 0; // 0 = autogenerate mipmaps
		desc.ArraySize = 1; // create a single texture
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D10_USAGE_DEFAULT;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS;  
		desc.CPUAccessFlags = 0;
		HRESULT hr = this->pD3DDevice->CreateTexture2D(&desc,0,&tex);
#endif
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX10_c::uploadTexture: pD3DDevice->CreateTexture2D failed\n");
			return;
		}
		// copy image data to dx10 texture
		D3D10_MAPPED_TEXTURE2D mappedTex;
		hr = tex->Map( D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex );
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX10_c::uploadTexture: tex->Map( .. ) failed\n");
			return;
		}
		byte *outPTexels = (UCHAR*)mappedTex.pData;
		memcpy(outPTexels,data,w*h*4);
		tex->Unmap( D3D10CalcSubresource(0, 0, 1) ); // NOTE: unmap "returns" void
		out->setInternalHandleV(tex);
	}
	virtual void uploadLightmap(class textureAPI_i *out, const byte *data, u32 w, u32 h, bool rgba) {
		
	}
	virtual void freeTextureData(class textureAPI_i *ptr) {
		ID3D10Texture2D *tex = (ID3D10Texture2D *)ptr->getInternalHandleV();
		if(tex == 0)
			return;
		tex->Release();
		ptr->setInternalHandleV(0);
	}

	// vertex buffers (VBOs)
	virtual bool createVBO(class rVertexBuffer_c *ptr) {
		
		return false;
	}
	virtual bool destroyVBO(class rVertexBuffer_c *ptr) {

		return false;
	}

	// index buffers (IBOs)
	virtual bool createIBO(class rIndexBuffer_c *ptr) {	
	
		return false;
	}
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) {
		
		return false;
	}

	virtual void init() {
		// init SDL window
		g_sharedSDLAPI->init();

		// hack to get HWND (that wasnt needed for GL!)
		// I hope it won't cause any bugs
		hWnd = GetActiveWindow();

		ShowWindow(hWnd, 5);

		//Set up DX swap chain
		//--------------------------------------------------------------
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		
		//set buffer dimensions and format
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = g_sharedSDLAPI->getWinWidth();
		swapChainDesc.BufferDesc.Height = g_sharedSDLAPI->getWinHeigth();
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;;
		
		//set refresh rate
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		
		//sampling settings
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SampleDesc.Count = 1;

		//output window handle
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.Windowed = true;    

		//Create the D3D device
		//--------------------------------------------------------------
		if ( FAILED( D3D10CreateDeviceAndSwapChain(		NULL, 
														D3D10_DRIVER_TYPE_HARDWARE, 
														NULL, 
														0, 
														D3D10_SDK_VERSION, 
														&swapChainDesc, 
														&pSwapChain, 
														&pD3DDevice ) ) ) {
			g_core->RedWarning("D3D device creation failed");
			return; // ERROR
		}

		//Create render target view
		//--------------------------------------------------------------
		
		//try to get the back buffer
		ID3D10Texture2D* pBackBuffer;	
		if ( FAILED( pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*) &pBackBuffer) ) ) {
			g_core->RedWarning("Could not get back buffer");			
			return; // ERROR
		}

		//try to create render target view
		if ( FAILED( pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView) ) ) {
			g_core->RedWarning("Could not create render target view");			
			return; // ERROR
		}

		//release the back buffer
		pBackBuffer->Release();

		//set the render target
		pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);
		
		//Create viewport
		//--------------------------------------------------------------
		
		//create viewport structure	
		viewPort.Width = g_sharedSDLAPI->getWinWidth();
		viewPort.Height = g_sharedSDLAPI->getWinHeigth();
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;

		//set the viewport
		pD3DDevice->RSSetViewports(1, &viewPort);

		// Set up the view matrix
		//--------------------------------------------------------------
		D3DXVECTOR3 camera[3] = {	D3DXVECTOR3(0.0f, 0.0f, -5.0f),
									D3DXVECTOR3(0.0f, 0.0f, 1.0f),
									D3DXVECTOR3(0.0f, 1.0f, 0.0f) };
		D3DXMatrixLookAtLH(&viewMatrix, &camera[0], &camera[1], &camera[2]);
			
		//Set up projection matrix
		//--------------------------------------------------------------
		D3DXMatrixPerspectiveFovLH(&projectionMatrix, (float)D3DX_PI * 0.5f, (float)g_sharedSDLAPI->getWinWidth()/(float)g_sharedSDLAPI->getWinHeigth(), 0.1f, 100.0f);

		// This depends on SDL_INIT_VIDEO, hence having it here
		g_inputSystem->IN_Init();
	}
	virtual void shutdown(bool destroyWindow) {
		lastMat = 0;
		lastLightmap = 0;

		hWnd = 0;
		if(pRenderTargetView) {
			pRenderTargetView->Release();
			pRenderTargetView = 0;
		}
		if(pSwapChain) {
			pSwapChain->Release();
			pSwapChain = 0;
		}
		if(pD3DDevice) {
			pD3DDevice->Release();	
			pD3DDevice = 0;
		}
		if(destroyWindow) {
			g_sharedSDLAPI->shutdown();
		}
	}
};


static rbDX10_c g_staticDX10Backend;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)&g_staticDX10Backend,RENDERER_BACKEND_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_inputSystem,INPUT_SYSTEM_API_IDENTSTR);
	//g_iFaceMan->registerIFaceUser(&g_sysEventCaster,SYSEVENTCASTER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_sharedSDLAPI,SHARED_SDL_API_IDENTSTRING);
}

qioModule_e IFM_GetCurModule() {
	return QMD_REF_BACKEND_DX10;
}

