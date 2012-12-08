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
// per pixel lighting shader for DirectX9 renderer backend

float4x4 worldViewProjectionMatrix;
float3 lightOrigin;
float lightRadius;

texture colorMapTexture;

sampler2D colorMap = sampler_state
{
	Texture = <colorMapTexture>;
    MagFilter = Linear;
    MinFilter = Anisotropic;
    MipFilter = Linear;
    MaxAnisotropy = 16;
};

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
	float3 positionRAW : TEXCOORD1;
	float3 normal : TEXCOORD2;
};

VS_OUTPUT VS_PerPixelLighting(VS_INPUT IN)
{
    VS_OUTPUT OUT;
    
    OUT.position = mul(float4(IN.position, 1.0f), worldViewProjectionMatrix);    
    OUT.positionRAW = IN.position;
	OUT.texCoord = IN.texCoord;
	OUT.normal = IN.normal;
       
    return OUT;
}

float4 PS_PerPixelLighting(VS_OUTPUT IN) : COLOR
{
#if 0
	return tex2D(colorMap, IN.texCoord);
	//return tex2D(colorMap, IN.texCoord*2);
#else
	float3 lightToVert = lightOrigin - IN.positionRAW;
	float distance = length(lightToVert);
	if(distance > lightRadius) {
		// pixel is too far from the light
		return float4(0,0,0,0);
	}
    float3 lightDirection = normalize(lightToVert);
    // calculate the diffuse value based on light angle	
    float angleFactor = dot(IN.normal, lightDirection);
    if(angleFactor < 0) {
		// light is behind the surface
		return float4(0,0,0,0);
    }
	//  apply distnace scale
  	float distanceFactor = 1 - distance / lightRadius;
	// calculate the final color
	return tex2D(colorMap, IN.texCoord) * angleFactor * distanceFactor;
#endif
}

technique PerPixelLighting
{
	pass
	{
		VertexShader = compile vs_2_0 VS_PerPixelLighting();
		PixelShader = compile ps_2_0 PS_PerPixelLighting();
	}
}
