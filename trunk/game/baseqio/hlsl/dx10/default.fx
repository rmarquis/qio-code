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
#ifdef HAS_TEXGEN_ENVIROMENT
	float3 normal : NORMAL;
#endif
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
#ifdef HAS_LIGHTMAP
    float2 lm : TEXCOORD1;
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

	return output;
}

float4 PS_SimpleTexturing(PixelInputType input) : SV_Target
{
	float4 textureColor;
	textureColor = shaderTexture.Sample(SampleType, input.tex);
#ifdef HAS_LIGHTMAP
	textureColor *= shaderLightmap.Sample(SampleType, input.lm);
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
