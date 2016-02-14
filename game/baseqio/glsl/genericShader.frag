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
// glsl/genericShader.frag

// shader input
uniform sampler2D colorMap;
#ifdef HAS_LIGHTMAP
uniform sampler2D lightMap;
#endif // HAS_LIGHTMAP
#ifdef HAS_HEIGHT_MAP
uniform sampler2D heightMap;
varying vec3 v_tbnEyeDir;
#endif
#if defined(HAS_HEIGHT_MAP) && defined(USE_RELIEF_MAPPING)
#include "reliefMappingRaycast.inc"
#endif
#ifdef HAS_DELUXEMAP
uniform sampler2D deluxeMap;
#endif
#ifdef HAS_BUMP_MAP
uniform sampler2D bumpMap;
#endif
// extra per-surface material color
#ifdef HAS_MATERIAL_COLOR
uniform vec4 u_materialColor;
#endif

#ifdef HAS_VERTEXCOLORS
varying vec4 v_color4;
#endif // HAS_VERTEXCOLORS

#if defined(HAS_BUMP_MAP) && defined(HAS_DELUXEMAP)
attribute vec3 atrTangents;
attribute vec3 atrBinormals;
varying mat3 tbnMat;
#endif

#ifdef HAS_SUNLIGHT
uniform vec3 u_sunDirection;
uniform vec3 u_sunColor;
varying vec3 v_vertNormal;
#endif

#ifdef HAS_DIRECTIONAL_SHADOW_MAPPING
uniform sampler2DShadow directionalShadowMap;
varying vec4 shadowCoord;
#endif 
#ifdef HAS_SHADOWMAP_LOD1
uniform sampler2DShadow directionalShadowMap_lod1;
varying vec4 shadowCoord_lod1;
uniform vec3 u_shadowMapLod0Mins;
uniform vec3 u_shadowMapLod0Maxs;
#endif
#ifdef HAS_SHADOWMAP_LOD2
uniform sampler2DShadow directionalShadowMap_lod2;
varying vec4 shadowCoord_lod2;
uniform vec3 u_shadowMapLod1Mins;
uniform vec3 u_shadowMapLod1Maxs;
#endif

#ifdef HAS_SHADOWMAP_LOD1
varying vec3 v_vertXYZ;
#endif

#ifdef HAS_DIRECTIONAL_SHADOW_MAPPING
#ifdef ENABLE_SHADOW_MAPPING_BLUR
float doShadowBlurSample(sampler2DShadow map, vec4 coord)
{
	float shadow = 0.0;
	
	vec2 poissonDisk[4] = vec2[](
		vec2( -0.94201624, -0.39906216 ),
		vec2( 0.94558609, -0.76890725 ),
		vec2( -0.094184101, -0.92938870 ),
		vec2( 0.34495938, 0.29387760 )
	);

	for (int i=0;i<4;i++)
	{
		vec4 c = coord + vec4(poissonDisk[i].x / 700.0, poissonDisk[i].y / 700.0, 0, 0.0);
		shadow += shadow2DProj(map, c ).s;
	}


	shadow /= 4;
	
	return shadow;
}
#endif 
#endif


#ifdef HAS_SHADOWMAP_LOD1
bool IsCurrentFragmentInsideBounds(vec3 mins, vec3 maxs) {
	if(v_vertXYZ.x < maxs.x && v_vertXYZ.y < maxs.y && v_vertXYZ.z < maxs.z &&
		v_vertXYZ.x > mins.x && v_vertXYZ.y > mins.y && v_vertXYZ.z > mins.z) {
		return true;
	}
	return false;
}
#endif

void main() {
#ifdef HAS_HEIGHT_MAP
    vec3 eyeDirNormalized = normalize(v_tbnEyeDir);
#ifdef USE_RELIEF_MAPPING
	// relief mapping
	vec2 texCoord = ReliefMappingRayCast(gl_TexCoord[0].xy,eyeDirNormalized);
#else
	// simple height mapping
    vec4 offset = texture2D(heightMap, gl_TexCoord[0].xy);
	offset = offset * 0.05 - 0.02;
	vec2 texCoord = offset.xy * eyeDirNormalized.xy +  gl_TexCoord[0].xy;   
#endif
#else
	vec2 texCoord = gl_TexCoord[0].st;
#endif 

#if defined(HAS_BUMP_MAP) && defined(HAS_DELUXEMAP) && defined(HAS_LIGHTMAP)
	// decode light direction from deluxeMap
	vec3 deluxeLightDir = texture2D(deluxeMap, gl_TexCoord[1].xy);
	deluxeLightDir = (deluxeLightDir - 0.5) * 2.0;
	//deluxeLightDir *= -1;
	
	// decode bumpmap normal
	vec3 bumpMapNormal = texture2D (bumpMap, texCoord);
	bumpMapNormal = (bumpMapNormal - 0.5) * 2.0;
	vec3 vertNormal = -tbnMat * bumpMapNormal;  
	
    // calculate the diffuse value based on light angle	
    float angleFactor = dot(vertNormal, deluxeLightDir);
    if(angleFactor < 0) {
		// light is behind the surface
		gl_FragColor = vec4(0,0,0,0); // need to set default color for some GPUs
		return;
    }
	gl_FragColor = texture2D (colorMap, texCoord)*texture2D (lightMap, gl_TexCoord[1].st)*angleFactor;
	//gl_FragColor = texture2D(deluxeMap, gl_TexCoord[1].xy);
	//gl_FragColor = texture2D (colorMap, gl_TexCoord[0].xy);
	//gl_FragColor = vec4(1,0,1,1);
    return;
#else
    
 
#ifdef HAS_SUNLIGHT
    float angleFactor = dot(v_vertNormal, u_sunDirection);
    if(angleFactor < 0) {
		// light is behind the surface
		gl_FragColor = vec4(0,0,0,0); // need to set default color for some GPUs
		return;
    }
#endif   

#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
	int usedSplitIndex = -1;
#endif

#ifdef HAS_DIRECTIONAL_SHADOW_MAPPING
#ifdef ENABLE_SHADOW_MAPPING_BLUR
	float shadow;
#ifdef HAS_SHADOWMAP_LOD1
	// this check is silly, TODO: do a better one
	if(IsCurrentFragmentInsideBounds(u_shadowMapLod0Mins,u_shadowMapLod0Maxs)) {
		shadow = doShadowBlurSample(directionalShadowMap, shadowCoord);	
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 0;
	#endif	
#ifdef HAS_SHADOWMAP_LOD2
	} else if(IsCurrentFragmentInsideBounds(u_shadowMapLod1Mins,u_shadowMapLod1Maxs)) {
		shadow = doShadowBlurSample(directionalShadowMap_lod1, shadowCoord_lod1);	
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 1;
	#endif	
	} else {
		shadow = doShadowBlurSample(directionalShadowMap_lod2, shadowCoord_lod2);
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 2;
	#endif	
	}
#else
	} else {
		shadow = doShadowBlurSample(directionalShadowMap_lod1, shadowCoord_lod1);
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 1;
	#endif	
	}
#endif
#else
	shadow = doShadowBlurSample(directionalShadowMap, shadowCoord);
#endif
#else
	float shadow;
#ifdef HAS_SHADOWMAP_LOD1
	// this check is silly, TODO: do a better one
	if(IsCurrentFragmentInsideBounds(u_shadowMapLod0Mins,u_shadowMapLod0Maxs)) {
		shadow = shadow2DProj(directionalShadowMap, shadowCoord).s;
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 0;
	#endif	
#ifdef HAS_SHADOWMAP_LOD2
	} else if(IsCurrentFragmentInsideBounds(u_shadowMapLod1Mins,u_shadowMapLod1Maxs)) {
		shadow = shadow2DProj(directionalShadowMap_lod1, shadowCoord_lod1).s;
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 1;
	#endif	
	} else {
		shadow = shadow2DProj(directionalShadowMap_lod2, shadowCoord_lod2).s;
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 2;
	#endif	
	}
#else	
	} else {
		shadow = shadow2DProj(directionalShadowMap_lod1, shadowCoord_lod1).s;	
	#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
		usedSplitIndex = 1;
	#endif	
	}
#endif
#else
	shadow = shadow2DProj(directionalShadowMap, shadowCoord).s;
#endif
#endif
#endif 
  // calculate the final color
#ifdef HAS_LIGHTMAP
#ifdef HAS_VERTEXCOLORS
  gl_FragColor = texture2D (colorMap, texCoord)*texture2D (lightMap, gl_TexCoord[1].st)*v_color4;
#else
  gl_FragColor = texture2D (colorMap, texCoord)*texture2D (lightMap, gl_TexCoord[1].st);
#endif // HAS_VERTEXCOLORS
#else
#ifdef HAS_VERTEXCOLORS
  gl_FragColor = texture2D (colorMap, texCoord)*v_color4;
#else
  gl_FragColor = texture2D (colorMap, texCoord);
#endif // HAS_VERTEXCOLORS
#endif // HAS_LIGHTMAP
#ifdef HAS_MATERIAL_COLOR
	gl_FragColor *= u_materialColor;
#endif
#endif // defined(HAS_BUMP_MAP) && defined(HAS_DELUXEMAP) && defined(HAS_LIGHTMAP)
 
#ifdef HAS_SUNLIGHT
    gl_FragColor *= angleFactor;
#endif   
#ifdef HAS_DIRECTIONAL_SHADOW_MAPPING
    gl_FragColor *= shadow;
#endif 
#ifdef DEBUG_SHADOWMAPPING_SHOW_SPLITS
	if(usedSplitIndex == 0) {
		gl_FragColor.rgb *= vec3(1,0,0);
	} else if(usedSplitIndex == 1) {
		gl_FragColor.rgb *= vec3(0,1,0);
	} else if(usedSplitIndex == 2) {
		gl_FragColor.rgb *= vec3(0,0,1);
	} else {
		gl_FragColor.rgb *= vec3(1,0,1);
	}
#endif	  
}