matrix worldMatrix;
matrix viewMatrix;
matrix projectionMatrix;
Texture2D shaderTexture;
#ifdef HAS_LIGHTMAP
Texture2D shaderLightmap;
#endif
#ifdef HAS_TEXGEN_ENVIROMENT
float3 viewOrigin;
#endif
#ifdef HAS_ALPHAFUNC
int alphaFuncType;
float alphaFuncValue;
#endif
#ifdef HAS_MATERIAL_COLOR
float4 materialColor;
#endif
#ifdef HAS_SUNLIGHT
float3 sunDirection;
#endif

SamplerState SampleType
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
#ifdef HAS_LIGHTMAP
    float2 lm : TEXCOORD1;
#endif
#if defined( HAS_TEXGEN_ENVIROMENT) || defined(HAS_SUNLIGHT)
	float3 normal : NORMAL;
#endif
#ifdef HAS_VERTEXCOLORS
    float4 color : COLOR;
#endif
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
#ifdef HAS_LIGHTMAP
    float2 lm : TEXCOORD1;
#endif
#ifdef HAS_SUNLIGHT
	float3 normal : TEXCOORD2;
#endif
#ifdef HAS_VERTEXCOLORS
    float4 color : COLOR;
#endif
};

PixelInputType VS_SimpleTexturing(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
#ifdef HAS_TEXGEN_ENVIROMENT
	float3 dir = viewOrigin - input.position;
	dir = normalize(dir);
	float dotValue = dot(input.normal,dir);
	float twoDot = 2.f * dotValue;

	float3 reflected;
	reflected.x = input.normal.x * twoDot - dir.x;
	reflected.y = input.normal.y * twoDot - dir.y;
	reflected.z = input.normal.z * twoDot - dir.z;

	output.tex.x = 0.5f + reflected.y * 0.5f;
	output.tex.y = 0.5f - reflected.z * 0.5f;
#else
    output.tex = input.tex;
#endif

#ifdef HAS_LIGHTMAP
    output.lm = input.lm;
#endif

#ifdef HAS_SUNLIGHT
	output.normal = input.normal;
#endif
#ifdef HAS_VERTEXCOLORS
    output.color = input.color;
#endif
	return output;
}

float4 PS_SimpleTexturing(PixelInputType input) : SV_Target
{
	float4 textureColor;
#ifdef ONLY_USE_LIGHTMAP
	textureColor = shaderLightmap.Sample(SampleType, input.lm);
#else // if not ONLY_USE_LIGHTMAP

	textureColor = shaderTexture.Sample(SampleType, input.tex);

#ifdef HAS_ALPHAFUNC
	if(alphaFuncType == AF_GT0) {
		if(textureColor[3] == 0) {
			discard;
		}
	} else if(alphaFuncType == AF_LT128) {
		if(textureColor[3] > 0.5f) {
			discard;
		}
	} else if(alphaFuncType == AF_GE128) {
		if(textureColor[3] <= 0.5f) {
			discard;
		}
	} else if(alphaFuncType == AF_D3_ALPHATEST) {
		if(textureColor[3] < alphaFuncValue) {
			discard;
		}
	}
#endif
	
#ifdef HAS_SUNLIGHT
	float dotValue = dot(input.normal,sunDirection);
	if(dotValue < 0)
	{
		return float4(0,0,0,0);
	}
	textureColor *= dotValue;
#endif

#ifdef HAS_LIGHTMAP
	textureColor *= shaderLightmap.Sample(SampleType, input.lm);
#endif
#endif // ONLY_USE_LIGHTMAP
// append per-vertex color (if needed)
#ifdef HAS_VERTEXCOLORS
	textureColor *= input.color;
#endif
// append per-surface color (if needed)
#ifdef HAS_MATERIAL_COLOR
	textureColor *= materialColor;
#endif
	//textureColor = float4(1,1,0,1);
    return textureColor;
}

technique10 DefaultTechnique
{
    pass pass0
    {
        SetVertexShader(CompileShader(vs_4_0, VS_SimpleTexturing()));
        SetPixelShader(CompileShader(ps_4_0, PS_SimpleTexturing()));
        SetGeometryShader(NULL);
    }
}
