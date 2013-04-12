
matrix worldMatrix;
matrix viewMatrix;
matrix projectionMatrix;
Texture2D shaderTexture;


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
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

BlendState AlphaBlendingState
{
	BlendEnable[0] = TRUE;
	DestBlend = INV_SRC_ALPHA;
	SrcBlend = SRC_ALPHA;
};

PixelInputType VS_SimpleTexturing(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
    
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;

	return output;
}

float4 PS_SimpleTexturing(PixelInputType input) : SV_Target
{
	float4 textureColor;
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	
	if(textureColor.a < 0.5f)
	{
		discard;
	}
    return textureColor;
}

technique10 DefaultTechnique
{
    pass pass0
    {
   		//SetBlendState(AlphaBlendingState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_4_0, VS_SimpleTexturing()));
        SetPixelShader(CompileShader(ps_4_0, PS_SimpleTexturing()));
        SetGeometryShader(NULL);
    }
}
