/*
============================================================================
Copyright (C) 2013 V.

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
};

VS_OUTPUT VS_GeneringShader(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    OUT.position = mul(float4(IN.position, 1.0f), worldViewProjectionMatrix);

    return OUT;
}

float4 PS_GeneringShader(VS_OUTPUT IN) : COLOR
{
	return float4(0,0,0,0);
}

technique GeneringShader
{
	pass
	{
		VertexShader = compile vs_2_0 VS_GeneringShader();
		PixelShader = compile ps_2_0 PS_GeneringShader();
	}
}
