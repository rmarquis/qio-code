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

float4x4 worldViewProjectionMatrix;
float3 viewOrigin;

texture colorMapTexture;
#ifdef HAS_LIGHTMAP
texture lightMapTexture;
#endif
// clip plane - for mirrors only
float3 clipPlaneNormal;
float clipPlaneDist;
bool bHasClipPlane;

#ifndef ONLY_LIGHTMAP
sampler2D colorMap = sampler_state
{
	Texture = <colorMapTexture>;
    MagFilter = Linear;
    MinFilter = Anisotropic;
    MipFilter = Linear;
    MaxAnisotropy = 16;
};
#endif
#ifdef HAS_LIGHTMAP
sampler2D lightMap = sampler_state
{
	Texture = <lightMapTexture>;
    MagFilter = Linear;
    MinFilter = Anisotropic;
    MipFilter = Linear;
    MaxAnisotropy = 16;
};
#endif


struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	int color : COLOR;
	float2 texCoord : TEXCOORD0;
	float2 lmCoord : TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 position : POSITION;
	float2 texCoord : TEXCOORD0;
#ifdef HAS_LIGHTMAP
	float2 lmCoord : TEXCOORD1;
#endif
#ifdef HAS_VERTEXCOLORS
	float4 vertexColor : COLOR;
#endif
};

VS_OUTPUT VS_GeneringShader(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    OUT.position = mul(float4(IN.position, 1.0f), worldViewProjectionMatrix);
	// NOTE: texGen environment is now not compatible with lightmap-only stages
#ifdef HAS_TEXGEN_ENVIROMENT
	float3 dir = viewOrigin - IN.position;
	dir = normalize(dir);
	float dotValue = dot(IN.normal,dir);
	float twoDot = 2.f * dotValue;

	float3 reflected;
	reflected.x = IN.normal.x * twoDot - dir.x;
	reflected.y = IN.normal.y * twoDot - dir.y;
	reflected.z = IN.normal.z * twoDot - dir.z;

	OUT.texCoord.x = 0.5f + reflected.y * 0.5f;
	OUT.texCoord.y = 0.5f - reflected.z * 0.5f;
#else
	OUT.texCoord = IN.texCoord;
#endif
#ifdef HAS_LIGHTMAP
	OUT.lmCoord = IN.lmCoord;
#endif       
#ifdef HAS_VERTEXCOLORS
	OUT.vertexColor = IN.normal;
#endif
    return OUT;
}

float4 PS_GeneringShader(VS_OUTPUT IN) : COLOR
{
    // mirrors-only clip plane
    if(bHasClipPlane) {
		float distToClipPlane = dot(IN.positionRAW,clipPlaneNormal) + clipPlaneDist;
		// clip will discard current pixel if distToClipPlane < 0
		clip(distToClipPlane);
    }
    
	float4 ret;
#ifdef ONLY_LIGHTMAP
	ret = tex2D(lightMap, IN.lmCoord);
#else
#ifdef HAS_LIGHTMAP
	ret = tex2D(colorMap, IN.texCoord)*tex2D(lightMap, IN.lmCoord);
#else
	ret = tex2D(colorMap, IN.texCoord);
#endif
#endif // not defined ONLY_LIGHTMAP

// append vertex colors (if needed)
#ifdef HAS_VERTEXCOLORS
	ret = IN.vertexColor;
#endif
	return ret;
}

technique GeneringShader
{
	pass
	{
		VertexShader = compile vs_2_0 VS_GeneringShader();
		PixelShader = compile ps_2_0 PS_GeneringShader();
	}
}
