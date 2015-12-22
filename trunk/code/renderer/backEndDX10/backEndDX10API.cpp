/*
============================================================================
Copyright (C) 2012-2013 V.

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
#include <api/rLightAPI.h>

#include <shared/r2dVert.h>
#include <shared/fcolor4.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <math/aabb.h>
#include <renderer/rVertexBuffer.h>
#include <renderer/rIndexBuffer.h>
#include <materialSystem/mat_public.h> // alphaFunc_e etc
#include <shared/str.h>
#include <shared/autoCvar.h>
#include <shared/cullType.h>

#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#else
#	include <SDL.h>
#endif

#include <d3d10.h>
#include <d3dx10.h>

#include <comdef.h>

// dx10 libraries
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")

static aCvar_c rb_showLightMaps("rb_showLightMaps","0");
static aCvar_c rb_showNormalColors("rb_showNormalColors","0");
static aCvar_c rb_skipStagesOfType_colorMapLightMapped("rb_skipStagesOfType_colorMapLightMapped","0");
static aCvar_c rb_skipStagesOfType_colorMap("rb_skipStagesOfType_colorMap","0");
static aCvar_c rb_skipStagesOfType_lightmap("rb_skipStagesOfType_lightmap","0");
static aCvar_c rb_showTris("rb_showTris","0");

// HLSL shaders can be compiled with various 
// options and defines
struct dx10ShaderPermutationFlags_s {
	// draw colormap with lightmap
	bool hasLightmap; // #define HAS_LIGHTMAP 
	bool hasTexGenEnvironment; // #define HAS_TEXGEN_ENVIROMENT
	bool onlyLightmap; // #define ONLY_USE_LIGHTMAP
	bool hasVertexColors; // #define HAS_VERTEXCOLORS
	bool hasAlphaFunc; // #define HAS_ALPHAFUNC
	bool hasMaterialColor; // #define HAS_MATERIAL_COLOR
	bool hasSunLight; // #define HAS_SUNLIGHT

	dx10ShaderPermutationFlags_s() {
		memset(this,0,sizeof(*this));
	}
};



class dx10Shader_c {
	str name;
	dx10ShaderPermutationFlags_s flags;
public:
	ID3D10Effect *m_effect;
	ID3D10EffectTechnique *m_technique;
	ID3D10InputLayout *m_layout;

	ID3D10EffectMatrixVariable *m_worldMatrixPtr;
	ID3D10EffectMatrixVariable *m_viewMatrixPtr;
	ID3D10EffectMatrixVariable *m_projectionMatrixPtr;
	ID3D10EffectShaderResourceVariable *m_texturePtr;
	ID3D10EffectShaderResourceVariable *m_lightmapPtr;
	ID3D10EffectVectorVariable *m_viewOrigin;
	ID3D10EffectVectorVariable *m_materialColor;
	ID3D10EffectVectorVariable *m_sunDirection;

	ID3D10EffectScalarVariable *m_alphaFuncType;
	ID3D10EffectScalarVariable *m_alphaFuncValue;

	ID3D10EffectVectorVariable *m_lightOrigin;
	ID3D10EffectScalarVariable *m_lightRadius;

	dx10Shader_c() {
		m_effect = 0;
		m_technique = 0;
		m_layout = 0;
		m_worldMatrixPtr = 0;
		m_viewMatrixPtr = 0;
		m_projectionMatrixPtr = 0;
		m_texturePtr = 0;
		m_lightmapPtr = 0;
		m_viewOrigin = 0;
		m_alphaFuncType = 0;
		m_alphaFuncValue = 0;
		m_materialColor = 0;
		m_lightOrigin = 0;
		m_lightRadius = 0;
	}

	const char *getName() const {
		return name;
	}
	const dx10ShaderPermutationFlags_s &getPermutations() const {
		return flags;
	}
	bool isValid() const {
		if(m_effect)
			return true;
		return false;
	}


	// init rVert_c format layout
	void init3DVertexFormat(D3D10_INPUT_ELEMENT_DESC polygonLayout[], u32 &numElements) {
		polygonLayout[0].SemanticName = "POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		polygonLayout[1].SemanticName = "NORMAL";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
		polygonLayout[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		polygonLayout[2].SemanticName = "COLOR";
		polygonLayout[2].SemanticIndex = 0;
		polygonLayout[2].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		polygonLayout[2].InputSlot = 0;
		polygonLayout[2].AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
		polygonLayout[2].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		polygonLayout[2].InstanceDataStepRate = 0;

		polygonLayout[3].SemanticName = "TEXCOORD";
		polygonLayout[3].SemanticIndex = 0;
		polygonLayout[3].Format = DXGI_FORMAT_R32G32_FLOAT;
		polygonLayout[3].InputSlot = 0;
		polygonLayout[3].AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
		polygonLayout[3].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		polygonLayout[3].InstanceDataStepRate = 0;

		polygonLayout[4].SemanticName = "TEXCOORD";
		polygonLayout[4].SemanticIndex = 1;
		polygonLayout[4].Format = DXGI_FORMAT_R32G32_FLOAT;
		polygonLayout[4].InputSlot = 0;
		polygonLayout[4].AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
		polygonLayout[4].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		polygonLayout[4].InstanceDataStepRate = 0;

		numElements = 5;
	}
	// init r2dVert_s vertex format (used for gui graphics, console background and fonts)
	void init2DVertexFormat(D3D10_INPUT_ELEMENT_DESC polygonLayout[], u32 &numElements) {
		// this is still a vec3 (but we're using only X and Y)
		polygonLayout[0].SemanticName = "POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		// vec2, texcoord
		polygonLayout[1].SemanticName = "TEXCOORD";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
		polygonLayout[1].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		numElements = 2;
	}

	bool loadHLSLShader(ID3D10Device* device, const char *fname, bool is3DShader, const dx10ShaderPermutationFlags_s *pFlags) {
		this->flags = *pFlags;
		this->name = fname;
		char *buf;
		u32 fileLen;
		fileLen = g_vfs->FS_ReadFile(fname,(void**)&buf);
		if(buf == 0) {
			g_core->RedWarning("dx10Shader_c:: cannot open %s for reading\n",fname);
			return true; // error
		}
		// generate final shader text
		str finalEffectDef = "\n";
		// first append #defines
		if(pFlags->hasLightmap) {
			finalEffectDef.append("#define HAS_LIGHTMAP\n");
		}
		if(pFlags->hasTexGenEnvironment) {
			finalEffectDef.append("#define HAS_TEXGEN_ENVIROMENT\n");
		}
		if(pFlags->onlyLightmap) {
			finalEffectDef.append("#define ONLY_USE_LIGHTMAP\n");
		}
		if(pFlags->hasVertexColors) {
			finalEffectDef.append("#define HAS_VERTEXCOLORS\n");
		}
		if(pFlags->hasAlphaFunc) {
			finalEffectDef.append("#define HAS_ALPHAFUNC\n");
			finalEffectDef.append(va("#define AF_NONE %i\n",AF_NONE));
			finalEffectDef.append(va("#define AF_GT0 %i\n",AF_GT0));
			finalEffectDef.append(va("#define AF_LT128 %i\n",AF_LT128));
			finalEffectDef.append(va("#define AF_GE128 %i\n",AF_GE128));
			finalEffectDef.append(va("#define AF_D3_ALPHATEST %i\n",AF_D3_ALPHATEST));
		}
		if(pFlags->hasMaterialColor) {
			finalEffectDef.append("#define HAS_MATERIAL_COLOR\n");
		}
		if(pFlags->hasSunLight) {
			finalEffectDef.append("#define HAS_SUNLIGHT\n");
		}

		// then add main shader code
		finalEffectDef.append(buf);

		finalEffectDef.append("\n");

		// load the shader from the memory
		ID3D10Blob* errorMessage = 0;
		HRESULT result = D3DX10CreateEffectFromMemory(finalEffectDef,finalEffectDef.length(),fname, NULL, NULL, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, 
											device, NULL, NULL, &m_effect, &errorMessage, NULL);
		if(FAILED(result)) {
			g_core->RedWarning("dx10Shader_c:: D3DX10CreateEffectFromMemory failed for %s\n",fname);
			if(errorMessage) {
				g_core->RedWarning("%s",errorMessage->GetBufferPointer());
				errorMessage->Release();
			}
			return true; // error
		}

		// Get a pointer to the technique inside the shader.
		m_technique = m_effect->GetTechniqueByName("DefaultTechnique");
		if(!m_technique) {
			g_core->RedWarning("dx10Shader_c:: coudlnt find \"DefaultTechnique\" in %s\n",fname);
			return true; // error
		}

		// Now setup the layout of the data that goes into the shader.
		// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
		D3D10_INPUT_ELEMENT_DESC polygonLayout[8];
		u32 numElements;
		if(is3DShader) {
			init3DVertexFormat(polygonLayout,numElements);
		} else {
			init2DVertexFormat(polygonLayout,numElements);
		}

		// Get the description of the first pass described in the shader technique.
		D3D10_PASS_DESC passDesc;
		m_technique->GetPassByIndex(0)->GetDesc(&passDesc);

		// Create the input layout.
		result = device->CreateInputLayout(polygonLayout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, 
										   &m_layout);
		if(FAILED(result)) {
			g_core->RedWarning("dx10Shader_c:: CreateInputLayout failed for %s\n",fname);
			return true; // error;
		}

		// Get pointers to the three matrices inside the shader so we can update them from this class.
		m_worldMatrixPtr = m_effect->GetVariableByName("worldMatrix")->AsMatrix();
		m_viewMatrixPtr = m_effect->GetVariableByName("viewMatrix")->AsMatrix();
		m_projectionMatrixPtr = m_effect->GetVariableByName("projectionMatrix")->AsMatrix();

		// Get pointer to the texture resource inside the shader.
		m_texturePtr = m_effect->GetVariableByName("shaderTexture")->AsShaderResource();
		m_lightmapPtr = m_effect->GetVariableByName("shaderLightmap")->AsShaderResource();

		m_viewOrigin = m_effect->GetVariableByName("viewOrigin")->AsVector();
		m_materialColor = m_effect->GetVariableByName("materialColor")->AsVector();

		m_alphaFuncValue = m_effect->GetVariableByName("alphaFuncValue")->AsScalar();
		m_alphaFuncType = m_effect->GetVariableByName("alphaFuncType")->AsScalar();

		m_sunDirection = m_effect->GetVariableByName("sunDirection")->AsVector();

		m_lightOrigin = m_effect->GetVariableByName("lightOrigin")->AsVector();
		m_lightRadius= m_effect->GetVariableByName("lightRadius")->AsScalar();

		return false; // no error
	}
	void destroyShader() {
		if(m_effect) {
			m_effect->Release();
			m_effect = 0;
		}
	}
};

class dx10TempBuffer_c {
	ID3D10Buffer *buf;
	ID3D10Device *pMyDev;
public:
	dx10TempBuffer_c() {
		buf = 0;
		pMyDev = 0;
	}
	~dx10TempBuffer_c() {
		destroy();
	}
	bool create(ID3D10Device *pD3DDevice, const void *data, u32 sizeInBytes, bool isVertexBuffer) {
		pMyDev = pD3DDevice;

		this->destroy();

		D3D10_BUFFER_DESC bufferDesc;
		memset(&bufferDesc,0,sizeof(bufferDesc));
		bufferDesc.Usage = D3D10_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeInBytes;
		if(isVertexBuffer) {
			bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		} else {
			bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		}
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		D3D10_SUBRESOURCE_DATA bufferData;
		memset(&bufferData,0,sizeof(bufferData));
		bufferData.pSysMem = data;
		try {
			HRESULT result = pD3DDevice->CreateBuffer(&bufferDesc, &bufferData, &buf);
			if(FAILED(result)) {
				return true;
			}	
		} catch(_com_error& ex) {
			g_core->RedWarning("dx10TempBuffer_c::create: _com_error exception caught!\n");
		}
		return false;
	}
	bool destroy() {
		if(buf) {
			try {
				buf->Release();
				buf = 0;
			} catch(_com_error& ex) {
				g_core->RedWarning("dx10TempBuffer_c::destroy: _com_error exception caught!\n");
			}
		}	
		return false;
	}
	ID3D10Buffer *getDX10Buffer() {
		return buf;
	}
};


class dx10ShadersSystem_c {
	ID3D10Device *pD3DDevice;
	arraySTD_c<dx10Shader_c*> shaders;

	dx10Shader_c *findShader(const char *baseName, const dx10ShaderPermutationFlags_s &p) {
		for(u32 i = 0; i < shaders.size(); i++) {
			dx10Shader_c *s = shaders[i];
			if(!_stricmp(baseName,s->getName())
				&& !memcmp(&s->getPermutations(),&p,sizeof(dx10ShaderPermutationFlags_s))) {
				return s;
			}
		}
		return 0;
	}
public:
	dx10ShadersSystem_c() {
		pD3DDevice = 0;
	}
	~dx10ShadersSystem_c() {
		shutdownShadersSystem();
	}
	void initShadersSystem(ID3D10Device *pNewDevice) {
		this->pD3DDevice = pNewDevice;
	}
	void shutdownShadersSystem() {
		for(u32 i = 0; i < shaders.size(); i++) {
			delete shaders[i];
		}
		shaders.clear();
	}
	dx10Shader_c *registerShader(const char *shaderName, const dx10ShaderPermutationFlags_s *flags, bool is3D = true) {
		str fullPath = "hlsl/dx10/";
		fullPath.append(shaderName);
		fullPath.append(".fx");
		dx10ShaderPermutationFlags_s defaultFlags;
		if(flags == 0) {
			flags = &defaultFlags;
		}
		dx10Shader_c *existing = findShader(fullPath,*flags);
		if(existing) {
			if(existing->isValid())
				return existing;
			return 0;
		}
		dx10Shader_c *newShader = new dx10Shader_c;
		newShader->loadHLSLShader(pD3DDevice,fullPath,is3D,flags);
		shaders.push_back(newShader);
		if(newShader->isValid() == false)
			return 0;
		return newShader;
	}
};
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
	// windows handle
	HWND hWnd;
	// DirectX10 pointers
	ID3D10Device *pD3DDevice;
	IDXGISwapChain *pSwapChain;
	ID3D10RenderTargetView *pRenderTargetView;
	ID3D10Texture2D* m_depthStencilBuffer;
	ID3D10DepthStencilState* m_depthStencilState;
	ID3D10DepthStencilView* m_depthStencilView;
	ID3D10RasterizerState* m_rasterState_backFaceCulling;
	ID3D10RasterizerState* m_rasterState_frontFaceCulling;
	ID3D10RasterizerState* m_rasterState_noFaceCulling;
	ID3D10RasterizerState* m_rasterState_showTris;
	ID3D10DepthStencilState* m_depthDisabledStencilState;
	ID3D10DepthStencilState* m_depthStencilState_noWrite;
	// viewport def
	D3D10_VIEWPORT viewPort;
	// our classes
	dx10ShadersSystem_c *shadersSystem;

	// matrices, they will be send to HLSL shaders
	matrix_c viewMatrix;
	matrix_c worldMatrix;
	matrix_c worldMatrixInverse;
	axis_c viewAxis;
	vec3_c camOriginWorldSpace;
	vec3_c camOriginEntitySpace;
	matrix_c projectionMatrix;

	bool bHasVertexColors;
	bool bDrawingSky;
	fcolor4_c curColor;
	int forcedMaterialFrameNum;
	float timeNowSeconds;
	bool bHasSunLight;
	class vec3_c sunColor;
	class vec3_c sunDirection;
	aabb sunLightBounds;
	bool usingWorldSpace;
	bool bDrawOnlyOnDepthBuffer;
	u32 portalDepth;

	// material and lightmap
	mtrAPI_i *lastMat;
	textureAPI_i *lastLightmap;
	const rLightAPI_i *curLight;
public:
	rbDX10_c() {
		hWnd = 0;
		lastMat = 0;
		lastLightmap = 0;
		shadersSystem = 0;
		bHasVertexColors = false;
		bDrawingSky = false;
		forcedMaterialFrameNum = -1;
		timeNowSeconds = 0.f;
		bHasSunLight = false;
		usingWorldSpace = false;
		bDrawOnlyOnDepthBuffer = false;
		curLight = 0;
	}
	virtual backEndType_e getType() const {
		return BET_DX10;
	}
	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap, class textureAPI_i *deluxemap) {
		lastMat = mat;
		lastLightmap = lightmap;
	}
	virtual void unbindMaterial() {
		lastMat = 0;
		disableLightmap();
		disableBlendFunc();
	}
	virtual void setColor4(const float *rgba) {
		curColor.fromColor4f(rgba);
	}
	virtual void setBindVertexColors(bool bBindVertexColors) {
		this->bHasVertexColors = bBindVertexColors;
	}
	virtual void setPortalDepth(u32 nPortalDepth) {
		portalDepth = nPortalDepth;
	}
	virtual void setBDrawOnlyOnDepthBuffer(bool bNewDrawOnlyOnDepthBuffer) {
		bDrawOnlyOnDepthBuffer = bNewDrawOnlyOnDepthBuffer;
	}
	virtual void setCurrentDrawCallSort(enum drawCallSort_e sort) {

	}
	virtual void setRShadows(int newRShadows) {

	}
	virtual void setCurrentDrawCallCubeMapSide(int iCubeSide) {

	}
	virtual void setCurLightShadowMapSize(int newW, int newH) {

	}
	virtual void setCurLight(const class rLightAPI_i *light) {
		if(this->curLight == light)
			return;
		this->curLight = light;
	}
	virtual void setSunParms(bool newBHasSunLight, const class vec3_c &newSunColor, const class vec3_c &newSunDirection, const class aabb &newSunBounds) {
		bHasSunLight = newBHasSunLight;
		sunColor = newSunColor;
		sunDirection = newSunDirection;
		sunLightBounds = newSunBounds;
	}
	virtual void setForcedMaterialMapFrame(int animMapFrame) {
		this->forcedMaterialFrameNum = animMapFrame;
	}
	virtual void setRenderTimeSeconds(float newCurTime) {
		this->timeNowSeconds = newCurTime;
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) {
		if(lastMat == 0)
			return;
		if(lastMat->getNumStages() == 0)
			return;
#if 0
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
		}\
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
#else
		if(shadersSystem == 0) {
			g_core->RedWarning("rbDX10_c::draw2D: shadersSystem is not ready\n");
			return;
		}
		dx10Shader_c *shader2D = shadersSystem->registerShader("default_2d",0,false);
		if(shader2D == 0) {
			g_core->RedWarning("rbDX10_c::draw2D: cannot find default 2D shader\n");
			return;
		}

		tempGPUVerts.create(this->pD3DDevice,verts,numVerts*sizeof(r2dVert_s),true);
		ID3D10Buffer *pNewVertexBuffer = tempGPUVerts.getDX10Buffer();

		tempGPUIndices.create(this->pD3DDevice,indices,numIndices*sizeof(u16),false);
		ID3D10Buffer *pNewIndexBuffer = tempGPUIndices.getDX10Buffer();

		unsigned int stride;
		unsigned int offset;

		// store vertex props
		stride = sizeof(r2dVert_s); 
		offset = 0;
	    
		// set blending state
		//setBlendFunc(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);
		disableBlendFunc();

		// set vertex buffer data
		pD3DDevice->IASetVertexBuffers(0, 1, &pNewVertexBuffer, &stride, &offset);

		// set index type (u16/u32)
		pD3DDevice->IASetIndexBuffer(pNewIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		// set primitives type (we're always using triangles)
		pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	

		// set the world matrix (model pos)
		shader2D->m_worldMatrixPtr->SetMatrix((float*)&worldMatrix);

		// set the view matrix (camera pos)
		shader2D->m_viewMatrixPtr->SetMatrix((float*)&viewMatrix);

		// set the projection matrix (camera lens)
		shader2D->m_projectionMatrixPtr->SetMatrix((float*)&projectionMatrix);

		// set current color
		if(shader2D->m_materialColor) {
			shader2D->m_materialColor->SetFloatVector((float*)curColor.toPointer());
		}

		// bind the texture.
		const mtrStageAPI_i *s = lastMat->getStage(0);
		ID3D10ShaderResourceView *colorMapResource = (ID3D10ShaderResourceView *)s->getTexture(0)->getExtraUserPointer();

		shader2D->m_texturePtr->SetResource(colorMapResource);

		// set the input layout.
		pD3DDevice->IASetInputLayout(shader2D->m_layout);

		// get the description structure of the technique from inside the shader so it can be used for rendering.
		D3D10_TECHNIQUE_DESC techniqueDesc;
		shader2D->m_technique->GetDesc(&techniqueDesc);

		// go through each pass in the technique and render the triangles.
		for(u32 i = 0; i < techniqueDesc.Passes; i++) {
			shader2D->m_technique->GetPassByIndex(i)->Apply(0);
			pD3DDevice->DrawIndexed(numIndices, 0, 0);
		}

		tempGPUIndices.destroy();
		tempGPUVerts.destroy();
#endif
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
	D3D10_BLEND getDX10BlendEnumValue(short val) {
		if(val == BM_ONE) {
			return D3D10_BLEND_ONE;
		} else if(val == BM_ONE_MINUS_SRC_ALPHA) {
			return D3D10_BLEND_INV_SRC_ALPHA;
		} else if(val == BM_SRC_ALPHA) {
			return D3D10_BLEND_SRC_ALPHA;
		} else if(val == BM_ONE_MINUS_DST_ALPHA) {
			return D3D10_BLEND_INV_DEST_ALPHA;
		} else if(val == BM_DST_ALPHA) {
			return D3D10_BLEND_DEST_ALPHA;
		} else if(val == BM_DST_COLOR) {
			return D3D10_BLEND_DEST_COLOR;
		} else if(val == BM_ONE_MINUS_DST_COLOR) {
			return D3D10_BLEND_INV_DEST_COLOR;
		} else if(val == BM_ZERO) {
			return D3D10_BLEND_ZERO;
		} else {
			g_core->RedWarning("rbDX10_c::getDX10BlendEnumValue: Unsupported blend type\n");
			return D3D10_BLEND_ONE;
		}
	}
	void initBlendDesc(D3D10_BLEND_DESC &blendDesc, short src, short dst) {
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.BlendEnable[0] = true;

		blendDesc.SrcBlend = getDX10BlendEnumValue(src);
		blendDesc.SrcBlendAlpha = D3D10_BLEND_ONE;//getDX10BlendEnumValueAlpha(src);
	
		blendDesc.DestBlend = getDX10BlendEnumValue(dst);
		blendDesc.DestBlendAlpha = D3D10_BLEND_ZERO; //getDX10BlendEnumValueAlpha(dst);

		blendDesc.BlendOp        = D3D10_BLEND_OP_ADD;
		blendDesc.BlendOpAlpha   = D3D10_BLEND_OP_ADD;
		blendDesc.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;
	}
	ID3D10BlendState *blendStates[BM_NUM_BLEND_TYPES][BM_NUM_BLEND_TYPES];
	void setBlendFunc( short src, short dst ) {
		if( blendSrc != src || blendDst != dst ) {
			blendSrc = src;
			blendDst = dst;
			if(src == BM_NOT_SET && dst == BM_NOT_SET) {
				pD3DDevice->OMSetBlendState(0, 0, 0xffffffff);
			} else {
				if(blendStates[src][dst] == 0) {
					D3D10_BLEND_DESC blendDesc = {0};
					initBlendDesc(blendDesc,src,dst);
					HRESULT res = pD3DDevice->CreateBlendState(&blendDesc, &blendStates[src][dst]);
					if(res != S_OK) {
						g_core->RedWarning("CreateBlendState failed\n");
					}
				}
				pD3DDevice->OMSetBlendState(blendStates[src][dst], 0, 0xffffffff);
			}
		}
	}
	virtual void beginDrawingSky() {
		bDrawingSky = true;
	}
	virtual void endDrawingSky() {
		bDrawingSky = false;
	}
	void disableBlendFunc() {
		setBlendFunc(BM_NOT_SET,BM_NOT_SET);
	}
	// glDepthRange equivalent
	void setDepthRange(float min, float max) {
		D3D10_VIEWPORT viewport;
		viewport.Width = g_sharedSDLAPI->getWinWidth();;
		viewport.Height = g_sharedSDLAPI->getWinHeigth();
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = min;
		viewport.MaxDepth = max;

		// Create the viewport.
		pD3DDevice->RSSetViewports(1, &viewport);
	}
	textureAPI_i *getStageTextureInternal(const mtrStageAPI_i *stage) {
		if(forcedMaterialFrameNum == -1) {
			// use material time to get animated texture frame
			return stage->getTexture(this->timeNowSeconds);
		} else {
			// use texture frame set by cgame
			return stage->getTextureForFrameNum(forcedMaterialFrameNum);
		}
	}
	void setShaderVariables(dx10Shader_c *shader) {
		// set the world matrix (model pos)
		if(shader->m_worldMatrixPtr) {
			shader->m_worldMatrixPtr->SetMatrix((float*)&worldMatrix);
		}
		// set the view matrix (camera pos)
		if(shader->m_viewMatrixPtr) {
			shader->m_viewMatrixPtr->SetMatrix((float*)&viewMatrix);
		}
		// set the projection matrix (camera lens)
		if(shader->m_projectionMatrixPtr) {
			shader->m_projectionMatrixPtr->SetMatrix((float*)&projectionMatrix);
		}
		// set camera origin (in model space)
		// this is needed eg. for texgen enviromental
		if(shader->m_viewOrigin) {
			shader->m_viewOrigin->SetFloatVector(camOriginEntitySpace);
		}
		// set current color
		if(shader->m_materialColor) {
			shader->m_materialColor->SetFloatVector((float*)curColor.toPointer());
		}
		if(shader->m_sunDirection) {
			if(usingWorldSpace) {
				shader->m_sunDirection->SetFloatVector(sunDirection);
			} else {
				vec3_c dirLocal;
				worldMatrixInverse.transformNormal(sunDirection,dirLocal);
				shader->m_sunDirection->SetFloatVector(dirLocal);
			}
		}
		if(shader->m_lightOrigin) {
			if(curLight) {
				if(usingWorldSpace) {
					shader->m_lightOrigin->SetFloatVector(curLight->getOrigin());
				} else {
					vec3_c posLocal;
					worldMatrixInverse.transformPoint(curLight->getOrigin(),posLocal);
					shader->m_lightOrigin->SetFloatVector(posLocal);
				}
			}
		}
		if(shader->m_lightRadius) {
			if(curLight) {
				shader->m_lightRadius->SetFloat(curLight->getRadius());
			}
		}
	}
	dx10TempBuffer_c tempGPUVerts;
	dx10TempBuffer_c tempGPUIndices;
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		if(verts.size() == 0)
			return;
		if(indices.getNumIndices() == 0)
			return;
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::drawElements: pD3DDevice is NULL\n");
			return;
		}
		// setup geometry buffers (they are shared by all stages)
		ID3D10Buffer *pNewVertexBuffer;
		if(verts.getInternalHandleVoid() == 0) {
			tempGPUVerts.create(this->pD3DDevice,verts.getArray(),verts.getSizeInBytes(),true);
			pNewVertexBuffer = tempGPUVerts.getDX10Buffer();
		} else {
			pNewVertexBuffer = (ID3D10Buffer *)verts.getInternalHandleVoid();
		}
		ID3D10Buffer *pNewIndexBuffer;
		if(indices.getInternalHandleVoid() == 0) {
			tempGPUIndices.create(this->pD3DDevice,indices.getArray(),indices.getSizeInBytes(),false);
			pNewIndexBuffer = tempGPUIndices.getDX10Buffer();
		} else {
			pNewIndexBuffer = (ID3D10Buffer *)indices.getInternalHandleVoid();
		}

		unsigned int stride;
		unsigned int offset;

		// store vertex props
		stride = sizeof(rVert_c); 
		offset = 0;
	    
		// set vertex buffer data
		pD3DDevice->IASetVertexBuffers(0, 1, &pNewVertexBuffer, &stride, &offset);

		// set index type (u16/u32)
		if(indices.is32Bit()) {
			pD3DDevice->IASetIndexBuffer(pNewIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		} else {
			pD3DDevice->IASetIndexBuffer(pNewIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		}

		// set primitives type (we're always using triangles)
		pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		if(bDrawOnlyOnDepthBuffer) {	
			dx10Shader_c *shader = shadersSystem->registerShader("depthPass",0,true);
			if(shader == 0) {
				g_core->RedWarning("rbDX10_c::drawElements: cannot load default shader\n");
				return;
			}
			this->setShaderVariables(shader);

			// we're writing to depth buffer (opaque surfaces)
			pD3DDevice->OMSetDepthStencilState(m_depthStencilState, 1);

			// set the input layout.
			pD3DDevice->IASetInputLayout(shader->m_layout);

			// get the description structure of the technique from inside the shader so it can be used for rendering.
			D3D10_TECHNIQUE_DESC techniqueDesc;
			shader->m_technique->GetDesc(&techniqueDesc);

			// go through each pass in the technique and render the triangles.
			for(u32 i = 0; i < techniqueDesc.Passes; i++) {
				shader->m_technique->GetPassByIndex(i)->Apply(0);
				pD3DDevice->DrawIndexed(indices.getNumIndices(), 0, 0);
			}
			return;
		}

		// adjust the depth range for sky
		if(bDrawingSky) {
			this->setDepthRange(1,1);
		} else {
			this->setDepthRange(0,1);
		}

		cullType_e cullType = lastMat->getCullType();
		if(cullType == CT_BACK_SIDED) {
			if(portalDepth % 2 == 1) {
				pD3DDevice->RSSetState(m_rasterState_backFaceCulling);;
			} else {
				pD3DDevice->RSSetState(m_rasterState_frontFaceCulling);;
			}
		} else if(cullType == CT_TWO_SIDED) {
			pD3DDevice->RSSetState(m_rasterState_noFaceCulling);;
		} else {
			if(portalDepth % 2 == 1) {
				pD3DDevice->RSSetState(m_rasterState_frontFaceCulling);;
			} else {
				pD3DDevice->RSSetState(m_rasterState_backFaceCulling);;
			}
		}

		// for each stage...
		for(u32 stageNum = 0; stageNum < lastMat->getNumStages(); stageNum++) {
			const mtrStageAPI_i *s = lastMat->getStage(stageNum);
			enum stageType_e stageType = s->getStageType();

			textureAPI_i *colorMap = getStageTextureInternal(s);
			ID3D10ShaderResourceView *colorMapResource = (ID3D10ShaderResourceView *)colorMap->getExtraUserPointer();

			ID3D10ShaderResourceView *lightmapResource;
			if(lastLightmap) {
				lightmapResource =  (ID3D10ShaderResourceView *)lastLightmap->getExtraUserPointer();
			} else {
				lightmapResource = 0;
			}

			// find and setup the appropriate DirectX10 shader for current material stage
			dx10ShaderPermutationFlags_s flags;
			if(stageType == ST_COLORMAP_LIGHTMAPPED) {
				if(rb_skipStagesOfType_colorMapLightMapped.getInt())
					continue; // allow developers to skip rendering certain type of stage
				if(lastLightmap) {
					flags.hasLightmap = true;
				}
			} else if(stageType == ST_LIGHTMAP) {
				if(rb_skipStagesOfType_lightmap.getInt())
					continue; // allow developers to skip rendering certain type of stage
				if(lastLightmap) {
					flags.hasLightmap = true;
					flags.onlyLightmap = true;
				}
			} else if(stageType == ST_COLORMAP) {
				if(rb_skipStagesOfType_colorMap.getInt())
					continue; // allow developers to skip rendering certain type of stage
			}
			if(s->hasRGBGen() && s->getRGBGenType() == RGBGEN_VERTEX) {
				flags.hasVertexColors = true;
			} else if(bHasVertexColors && lastLightmap == 0) {
				// enable vertex lighting on bsp inline md3 models (for Quake3 maps)
				flags.hasVertexColors = true;
			}
			if(curColor.isFullBright() == false) {
				flags.hasMaterialColor = true;
			}
			flags.hasSunLight = bHasSunLight;

			if(s->hasTexGen() && s->getTexGen() == TCG_ENVIRONMENT) {
				flags.hasTexGenEnvironment = true;
			}
			// get the alpha func
			alphaFunc_e alphaFunc = s->getAlphaFunc();
			if(alphaFunc != AF_NONE) {
				flags.hasAlphaFunc = true;
			}

			if(curLight) {
				setBlendFunc(BM_ONE,BM_ONE);
			} else {
				const blendDef_s &blendDef = s->getBlendDef();
				setBlendFunc(blendDef.src,blendDef.dst);
			}

			dx10Shader_c *shader;
			if(rb_showNormalColors.getInt()) {
				shader = shadersSystem->registerShader("showNormalColors",0,true);
			} else if(curLight) {
				shader = shadersSystem->registerShader("lighting",&flags,true);
			} else {
				shader = shadersSystem->registerShader("default",&flags,true);
			}
			if(shader == 0) {
				if(shader == 0) {
					g_core->RedWarning("rbDX10_c::drawElements: cannot load selected shader, trying to select default...\n");
				}
				shader = shadersSystem->registerShader("default",0,true);
				if(shader == 0) {
					g_core->RedWarning("rbDX10_c::drawElements: cannot load default shader\n");
					return;
				}
			}
			this->setShaderVariables(shader);
			if(shader->m_lightmapPtr) {
				shader->m_lightmapPtr->SetResource(lightmapResource);
			}
			if(shader->m_texturePtr) {
				shader->m_texturePtr->SetResource(colorMapResource);
			}
			// set the alphafunc params
			if(shader->m_alphaFuncType) {
				shader->m_alphaFuncType->SetInt(alphaFunc);
			}
			if(shader->m_alphaFuncValue && alphaFunc == AF_D3_ALPHATEST) {
				float alphaFuncValue = s->evaluateAlphaTestValue(0);
				shader->m_alphaFuncValue->SetFloat(alphaFuncValue);
			}
			if(s->getDepthWrite()) {
				// we're writing to depth buffer (opaque surfaces)
				pD3DDevice->OMSetDepthStencilState(m_depthStencilState, 1);
			} else {
				// don't write to depth buffer (used for blended surfaces)
				pD3DDevice->OMSetDepthStencilState(m_depthStencilState_noWrite, 1);
			}

			// set the input layout.
			pD3DDevice->IASetInputLayout(shader->m_layout);

			// get the description structure of the technique from inside the shader so it can be used for rendering.
			D3D10_TECHNIQUE_DESC techniqueDesc;
			shader->m_technique->GetDesc(&techniqueDesc);

			// go through each pass in the technique and render the triangles.
			for(u32 i = 0; i < techniqueDesc.Passes; i++) {
				shader->m_technique->GetPassByIndex(i)->Apply(0);
				pD3DDevice->DrawIndexed(indices.getNumIndices(), 0, 0);
			}
		}
		if(rb_showTris.getInt()) {
			dx10Shader_c *	shader = shadersSystem->registerShader("whiteShader",0,true);
			if(shader) {
				disableZBuffer();
				this->setShaderVariables(shader);
				pD3DDevice->IASetInputLayout(shader->m_layout);
				D3D10_TECHNIQUE_DESC techniqueDesc;
				shader->m_technique->GetDesc(&techniqueDesc);
				pD3DDevice->RSSetState(m_rasterState_showTris);
				// go through each pass in the technique and render the triangles.
				for(u32 i = 0; i < techniqueDesc.Passes; i++) {
					shader->m_technique->GetPassByIndex(i)->Apply(0);
					pD3DDevice->DrawIndexed(indices.getNumIndices(), 0, 0);
				}
				pD3DDevice->RSSetState(m_rasterState_backFaceCulling);;
				enableZBuffer();
			}
		}
		tempGPUVerts.destroy();
		tempGPUIndices.destroy();
	}	
	virtual void drawElementsWithSingleTexture(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, class textureAPI_i *tex) {

	}
	virtual void beginFrame() {
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::beginFrame: pD3DDevice is NULL\n");
			return;
		}
		// clear scene
		pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0,0,0,0) );
    
		// clear the depth buffer.
		pD3DDevice->ClearDepthStencilView(m_depthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	}
	virtual void endFrame() {
		if(pSwapChain == 0) {
			g_core->RedWarning("rbDX10_c::endFrame: pSwapChain is NULL\n");
			return;
		}
		//flip buffers
		pSwapChain->Present(0,0);
	}
	virtual void clearDepthBuffer() {
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::clearDepthBuffer: pD3DDevice is NULL\n");
			return;
		}
		// clear the depth buffer.
		pD3DDevice->ClearDepthStencilView(m_depthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	}
	void enableZBuffer() {
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::enableZBuffer: pD3DDevice is NULL\n");
			return;
		}
		pD3DDevice->OMSetDepthStencilState(m_depthStencilState, 1);
	}
	void disableZBuffer() {
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::enableZBuffer: pD3DDevice is NULL\n");
			return;
		}
		pD3DDevice->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	}
	virtual void setup2DView() {
		D3DXMatrixOrthoOffCenterLH((D3DXMATRIX *)&projectionMatrix,0,this->getWinWidth(),this->getWinHeight(),0,0,1);
		worldMatrix.identity();
		viewMatrix.identity();

		disableZBuffer();
		if(pD3DDevice) {
			pD3DDevice->RSSetState(m_rasterState_noFaceCulling);;
		} else {
			g_core->RedWarning("rbDX10_c::setup2DView: pD3DDevice is NULL\n");
		}
	}
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &newCamAxis) {
		camOriginWorldSpace = newCamPos;
		viewAxis = newCamAxis;

		// transform by the camera placement and view axis
		viewMatrix.invFromAxisAndVector(newCamAxis,newCamPos);
		// convert to gl coord system
		viewMatrix.toGL();

		setupWorldSpace();

		enableZBuffer();
	}
	virtual void setupProjection3D(const struct projDef_s *pd) {
		projectionMatrix.setupProjection(pd->fovX,pd->fovY,pd->zNear,pd->zFar);
	}
	virtual void drawCapsuleZ(const float *xyz, float h, float w) {
	}
	virtual void drawBoxHalfSizes(const float *halfSizes) {
	}
	virtual void drawLineFromTo(const float *from, const float *to, const float *colorRGB, float lineWidth) {
	}
	virtual void drawBBLines(const class aabb &bb) {
	}
	// used while drawing world surfaces and particles
	virtual void setupWorldSpace() {
		worldMatrix.identity();
		worldMatrixInverse = worldMatrix.getInversed();
		camOriginEntitySpace = this->camOriginWorldSpace;
		usingWorldSpace = true;
	}
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
		worldMatrix.fromAxisAndOrigin(axis,origin);
		worldMatrixInverse = worldMatrix.getInversed();
		worldMatrixInverse.transformPoint(camOriginWorldSpace,camOriginEntitySpace);
		usingWorldSpace = false;
	}
	// same as above but with angles instead of axis
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) {
		worldMatrix.fromAnglesAndOrigin(angles,origin);
		worldMatrixInverse = worldMatrix.getInversed();
		worldMatrixInverse.transformPoint(camOriginWorldSpace,camOriginEntitySpace);
		usingWorldSpace = false;
	}

	virtual u32 getWinWidth() const  {
		return g_sharedSDLAPI->getWinWidth();
	}
	virtual u32 getWinHeight() const  {
		return g_sharedSDLAPI->getWinHeigth();
	}

	virtual void uploadTextureRGBA(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::uploadTextureRGBA: device is NULL\n");
			return;
		}
		ID3D10Texture2D *tex = 0;
		// create ID3D10Texture2D
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
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			g_core->RedWarning("rbDX10_c::uploadTexture: pD3DDevice->CreateTexture2D failed\n");
			return;
		}
		// copy image data to dx10 texture
		D3D10_MAPPED_TEXTURE2D mappedTex;
		hr = tex->Map( D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &mappedTex );
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			g_core->RedWarning("rbDX10_c::uploadTexture: tex->Map( .. ) failed\n");
			return;
		}
		byte *outPTexels = (UCHAR*)mappedTex.pData;
		memcpy(outPTexels,data,w*h*4);
		tex->Unmap( D3D10CalcSubresource(0, 0, 1) ); // NOTE: unmap "returns" void

		out->setWidth(w);
		out->setHeight(h);

		// create ID3D10ShaderResourceView
    	D3D10_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
		ZeroMemory( &resourceViewDesc, sizeof(resourceViewDesc)); 
		resourceViewDesc.Format = desc.Format;
		resourceViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		resourceViewDesc.Texture2D.MipLevels = desc.MipLevels;
    	resourceViewDesc.Texture2D.MostDetailedMip = 0;

		ID3D10ShaderResourceView *resourceView = 0;
		pD3DDevice->CreateShaderResourceView(tex, &resourceViewDesc, &resourceView);

		//pD3DDevice->GenerateMips(resourceView);

		// store ID3D10Texture2D pointer
		out->setInternalHandleV(tex);
		// store ID3D10ShaderResourceView pointer
		out->setExtraUserPointer(resourceView);
	}
	virtual void uploadLightmap(class textureAPI_i *out, const byte *data, u32 w, u32 h, bool rgba) {
		if(rgba) {
			uploadTextureRGBA(out,data,w,h);
		} else {
			arraySTD_c<byte> tmp;
			u32 numPixels = w * h;
			tmp.resize(numPixels * 4);
			const byte *in = data;
			byte *outB = tmp.getArray();
			for(u32 i = 0; i < numPixels; i++) {
				outB[0] = in[0];
				outB[1] = in[1];
				outB[2] = in[2];
				outB[3] = 255;
				outB+=4;
				in+=3;
			}
			uploadTextureRGBA(out,tmp.getArray(),w,h);
		}
	}
	virtual void freeTextureData(class textureAPI_i *ptr) {
		ID3D10Texture2D *tex = (ID3D10Texture2D *)ptr->getInternalHandleV();
		if(tex) {
			tex->Release();
			ptr->setInternalHandleV(0);
		}
		ID3D10ShaderResourceView *resourceView = (ID3D10ShaderResourceView *)ptr->getExtraUserPointer();
		if(resourceView) {
			resourceView->Release();
			ptr->setExtraUserPointer(0);
		}
	}

	// vertex buffers (VBOs)
	virtual bool createVBO(class rVertexBuffer_c *ptr) {
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::createVBO: device is NULL\n");
			return true;
		}
		D3D10_BUFFER_DESC vertexBufferDesc;
		memset(&vertexBufferDesc,0,sizeof(vertexBufferDesc));
		D3D10_SUBRESOURCE_DATA vertexData;
		memset(&vertexData,0,sizeof(vertexData));
		// Set up the description of the vertex buffer.
		vertexBufferDesc.Usage = D3D10_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = ptr->getSizeInBytes();
		vertexBufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = ptr->getArray();

		// Now finally create the vertex buffer.
		ID3D10Buffer *pNewVertexBuffer = 0;
		HRESULT result = pD3DDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &pNewVertexBuffer);
		if(FAILED(result))
		{
			return true;
		}	

		ptr->setInternalHandleVoid(pNewVertexBuffer);
		return false;
	}
	virtual bool destroyVBO(class rVertexBuffer_c *ptr) {
		if(ptr == 0)
			return false;
		ID3D10Buffer *pVertexBuffer = (ID3D10Buffer *)ptr->getInternalHandleVoid();
		if(pVertexBuffer) {
			pVertexBuffer->Release();
			ptr->setInternalHandleVoid(0);
		}
		return false;
	}

	// index buffers (IBOs)
	virtual bool createIBO(class rIndexBuffer_c *ptr) {	
		if(pD3DDevice == 0) {
			g_core->RedWarning("rbDX10_c::createVBO: device is NULL\n");
			return true;
		}
		destroyIBO(ptr);
		if(ptr->getNumIndices() == 0) {
			return false;
		}
		D3D10_BUFFER_DESC indexBufferDesc;
		memset(&indexBufferDesc,0,sizeof(indexBufferDesc));
		D3D10_SUBRESOURCE_DATA indexData;
		memset(&indexData,0,sizeof(indexData));
		// Set up the description of the index buffer.
		indexBufferDesc.Usage = D3D10_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = ptr->getSizeInBytes();
		indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = ptr->getArray();

		// Create the index buffer.
		ID3D10Buffer *pNewIndexBuffer = 0;
		HRESULT result = pD3DDevice->CreateBuffer(&indexBufferDesc, &indexData, &pNewIndexBuffer);
		if(FAILED(result))
		{
			return true;
		}
		ptr->setInternalHandleVoid(pNewIndexBuffer);	
		return false;
	}
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) {
		if(ibo == 0)
			return false;
		ID3D10Buffer *pIndexBuffer = (ID3D10Buffer *)ibo->getInternalHandleVoid();
		if(pIndexBuffer) {
			pIndexBuffer->Release();
			ibo->setInternalHandleVoid(0);
		}
		return false;
	}

	virtual void init() {
		// cvars
		AUTOCVAR_RegisterAutoCvars();
		// init SDL window
		g_sharedSDLAPI->init();

		// hack to get HWND (that wasnt needed for GL!)
		// I hope it won't cause any bugs
		hWnd = GetActiveWindow();

		ShowWindow(hWnd, 5);

		// Initialize the swap chain description.
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		// Set to a single back buffer.
		swapChainDesc.BufferCount = 1;

		// Set the width and height of the back buffer.
		swapChainDesc.BufferDesc.Width = g_sharedSDLAPI->getWinWidth();;
		swapChainDesc.BufferDesc.Height = g_sharedSDLAPI->getWinHeigth();;

		// Set regular 32-bit surface for the back buffer.
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Set the refresh rate of the back buffer.
		//if(vsync)
		//{
		//	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		//	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		//}
		//else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		// Set the usage of the back buffer.
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		// Set the handle for the window to render to.
		swapChainDesc.OutputWindow = hWnd;

		// Turn multisampling off.
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		// Set to full screen or windowed mode.
		//if(fullscreen)
		//{
		//	swapChainDesc.Windowed = false;
		//}
		//else
		{
			swapChainDesc.Windowed = true;
		}

		// Set the scan line ordering and scaling to unspecified.
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// Discard the back buffer contents after presenting.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		// Don't set the advanced flags.
		swapChainDesc.Flags = 0;

		// Create the swap chain and the Direct3D device.
		HRESULT result = D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0, D3D10_SDK_VERSION, 
											   &swapChainDesc, &pSwapChain, &pD3DDevice);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: D3D10CreateDeviceAndSwapChain failed\n");
			return;//  true;
		}

		// Get the pointer to the back buffer.
		ID3D10Texture2D* backBufferPtr;
		result = pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&backBufferPtr);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: pSwapChain->GetBuffer failed\n");
			return;// true;
		}

		// Create the render target view with the back buffer pointer.
		result = pD3DDevice->CreateRenderTargetView(backBufferPtr, NULL, &pRenderTargetView);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateRenderTargetView failed\n");
			return;//  true;
		}

		// Release pointer to the back buffer as we no longer need it.
		backBufferPtr->Release();
		backBufferPtr = 0;

		// Initialize the description of the depth buffer.
		D3D10_TEXTURE2D_DESC depthBufferDesc;
		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		// Set up the description of the depth buffer.
		depthBufferDesc.Width = g_sharedSDLAPI->getWinWidth();;
		depthBufferDesc.Height = g_sharedSDLAPI->getWinHeigth();;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D10_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		// Create the texture for the depth buffer using the filled out description.
		result = pD3DDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateTexture2D failed\n");
			return; //true; // error
		}

		// Initialize the description of the stencil state.
		D3D10_DEPTH_STENCIL_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		// Set up the description of the stencil state.
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
		///depthStencilDesc.DepthFunc = D3D10_COMPARISON_LESS;
		depthStencilDesc.DepthFunc = D3D10_COMPARISON_LESS_EQUAL;

		// Create the depth stencil state.
		result = pD3DDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateDepthStencilState failed\n");
			return; //true; // error
		}

		// create second depth stencil state that will not be writing to depth buffer
		depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
		result = pD3DDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState_noWrite);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateDepthStencilState failed\n");
			return; //true; // error
		}

		// Set the depth stencil state on the D3D device.
		pD3DDevice->OMSetDepthStencilState(m_depthStencilState, 1);

		// Initailze the depth stencil view.
		D3D10_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		// Create the depth stencil view.
		result = pD3DDevice->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateDepthStencilView failed\n");
			return; //true; // error
		}

		// Bind the render target view and depth stencil buffer to the output render pipeline.
		pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, m_depthStencilView);

		D3D10_RASTERIZER_DESC rasterDesc;
		// Setup the raster description which will determine how and what polygons will be drawn.
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D10_CULL_BACK;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D10_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		
		// Create the rasterizer state from the description we just filled out.
		result = pD3DDevice->CreateRasterizerState(&rasterDesc, &m_rasterState_backFaceCulling);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateRasterizerState failed\n");
			return; //true; // error
		}
		rasterDesc.CullMode = D3D10_CULL_FRONT;
		// Create the rasterizer state from the description we just filled out.
		result = pD3DDevice->CreateRasterizerState(&rasterDesc, &m_rasterState_frontFaceCulling);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateRasterizerState failed\n");
			return; //true; // error
		}
		// Setup the raster description which will determine how and what polygons will be drawn.
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D10_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D10_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		
		// Create the rasterizer state from the description we just filled out.
		result = pD3DDevice->CreateRasterizerState(&rasterDesc, &m_rasterState_noFaceCulling);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateRasterizerState failed\n");
			return; //true; // error
		}


		// Setup the raster description which will determine how and what polygons will be drawn.
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D10_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = false;
		rasterDesc.FillMode = D3D10_FILL_WIREFRAME;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		
		// Create the rasterizer state from the description we just filled out.
		result = pD3DDevice->CreateRasterizerState(&rasterDesc, &m_rasterState_showTris);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateRasterizerState failed\n");
			return; //true; // error
		}

		if(initShadowVolumeDepthStencilStates()) {
			g_core->RedWarning("rbDX10_c::init: CreateRasterizerState failed\n");
			return; //true; // error
		}

		// Now set the rasterizer state.
		pD3DDevice->RSSetState(m_rasterState_backFaceCulling);

		// Setup the viewport for rendering.
		D3D10_VIEWPORT viewport;
		viewport.Width = g_sharedSDLAPI->getWinWidth();;
		viewport.Height = g_sharedSDLAPI->getWinHeigth();;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		// Create the viewport.
		pD3DDevice->RSSetViewports(1, &viewport);

		// Clear the second depth stencil state before setting the parameters.
		D3D10_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
		ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

		// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
		// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
		depthDisabledStencilDesc.DepthEnable = false;
		depthDisabledStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
		depthDisabledStencilDesc.DepthFunc = D3D10_COMPARISON_LESS;
		depthDisabledStencilDesc.StencilEnable = true;
		depthDisabledStencilDesc.StencilReadMask = 0xFF;
		depthDisabledStencilDesc.StencilWriteMask = 0xFF;
		depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
		depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
		depthDisabledStencilDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
		depthDisabledStencilDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

		// Create the state using the device.
		result = pD3DDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
		if(FAILED(result))
		{
			g_core->RedWarning("rbDX10_c::init: CreateDepthStencilState failed\n");
			return; //true; // error
		}

		shadersSystem = new dx10ShadersSystem_c;
		shadersSystem->initShadersSystem(this->pD3DDevice);

		// This depends on SDL_INIT_VIDEO, hence having it here
		g_inputSystem->IN_Init();
	}
	bool initShadowVolumeDepthStencilStates() {
		// Initialize the description of the stencil state.
		//D3D10_DEPTH_STENCIL_DESC depthStencilDesc;
		//ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		//// Set up the description of the stencil state.
		//depthStencilDesc.DepthEnable = true;
		//depthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
		//depthStencilDesc.DepthFunc = D3D10_COMPARISON_LESS;

		//depthStencilDesc.StencilEnable = true;
		//depthStencilDesc.StencilReadMask = 0xFF;
		//depthStencilDesc.StencilWriteMask = 0xFF;

		//// Stencil operations if pixel is front-facing.
		//depthStencilDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
		//depthStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
		//depthStencilDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
		//depthStencilDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;


		//// Create the depth stencil state.
		//result = pD3DDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
		return false;
	}
	virtual void shutdown(bool destroyWindow) {
		AUTOCVAR_UnregisterAutoCvars();

		lastMat = 0;
		lastLightmap = 0;

		hWnd = 0;
		if(shadersSystem) {
			delete shadersSystem;
			shadersSystem = 0;
		}
		if(m_depthStencilState) {
			m_depthStencilState->Release();
			m_depthStencilState = 0;
		}
		if(m_rasterState_backFaceCulling) {
			m_rasterState_backFaceCulling->Release();
			m_rasterState_backFaceCulling = 0;
		}
		if(m_rasterState_frontFaceCulling) {
			m_rasterState_frontFaceCulling->Release();
			m_rasterState_frontFaceCulling = 0;
		}
		if(m_rasterState_noFaceCulling) {
			m_rasterState_noFaceCulling->Release();
			m_rasterState_noFaceCulling = 0;
		}
		if(m_rasterState_showTris) {
			m_rasterState_showTris->Release();
			m_rasterState_showTris = 0;
		}
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

