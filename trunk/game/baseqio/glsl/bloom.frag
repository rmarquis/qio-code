/*
============================================================================
Copyright (C) 2014 V.

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
// glsl/blur.frag

uniform sampler2D colorMap;
uniform sampler2D colorMap2;
 
varying vec2 v_texCoord;
uniform float u_averageScreenLuminance;

#if 1

void main(void)
{ 
    vec4 color = texture (colorMap, v_texCoord);
    vec4 bloomColor = texture (colorMap2, v_texCoord);
    
    vec4 LUMINANCE_VECTOR = vec4(0.2125, 0.7154, 0.0721, 0.0);
    float luminance = dot(LUMINANCE_VECTOR,color);
	luminance /= 4;
	
	if(luminance > u_averageScreenLuminance) {
float v = 0.5;
		float exposure = (1/u_averageScreenLuminance)*v;
		// Apply the exposure to this texel
		color = 1.0 - exp2 (-color * exposure);
		color.a = 1.0;
	   gl_FragColor = color;
	} else {
	   gl_FragColor = color;
	   //gl_FragColor = vec4(0,0,1,1);
	}


}
#elif 0
void main(void)
{ 
    // Fetch from HDR texture
    vec4 color = texture (colorMap, v_texCoord);
    vec4 bloomColor = texture (colorMap2, v_texCoord);

float v = 0.5;
	float exposure = (1/u_averageScreenLuminance)*v;
    // Apply the exposure to this texel
    bloomColor = 1.0 - exp2 (-color * exposure);
    bloomColor.a = 1.0;
    
    gl_FragColor = color + bloomColor;
}
#elif 0

void main(void)
{ 
    // Fetch from HDR texture
    vec4 color = texture (colorMap, v_texCoord);

float v = 0.5;
	float exposure = (1/u_averageScreenLuminance)*v;
    // Apply the exposure to this texel
    gl_FragColor = 1.0 - exp2 (-color * exposure);
    gl_FragColor.a = 1.0;
}
#elif 0
float contrast(float c)
{
    float a = 0.3;
    return clamp((c - a) / (1 - a), 0, 1);
}
 
void main(void)
{
    float radius = 0.005;
    vec2 pos = v_texCoord;
    float color = 0;
    float sum = 0;
 
    for(int x = -3; x < 4; x++)
    {
        for(int y = -3; y < 4; y++)
        {
            vec2 offset = vec2(x * radius, y * radius);
            float dist = exp(- sqrt(x * x + y * y) / 2);
            vec4 t = texture2D(colorMap, pos + offset);
            color += contrast((t.r + t.g + t.b) / 3) * dist;
            sum += dist;
        }
    }
 
    color /= sum;
 
    vec4 tc = texture2D(colorMap, pos) + vec4(color, color, color, 1) * 0.5;
 
    gl_FragColor = tc;
}

#else
 float contrast(float c)
{
    float a = 0.3;
    return clamp((c - a) / (1 - a), 0, 1);
}

void main()
{
   vec4 LUMINANCE_VECTOR = vec4(0.2125, 0.7154, 0.0721, 0.0);
	vec4 color = texture2D(colorMap2, v_texCoord);
    float luminance = dot(LUMINANCE_VECTOR,color);
	
	// adjust contrast
	//luminance = pow(luminance, 1.32);

	float T = clamp(luminance, 0.0, 1.0);

//T =  contrast((color.r + color.g + color.b) / 3);
T =  contrast(luminance);

		color.rgb *= T;

	
	vec4 baseColor = texture2D(colorMap, v_texCoord);
	gl_FragColor = baseColor + color;
	//gl_FragColor = color;

}

#endif