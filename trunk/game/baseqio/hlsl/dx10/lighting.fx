matrix worldMatrix;
matrix viewMatrix;
matrix projectionMatrix;
Texture2D shaderTexture;
float3 lightOrigin;
float lightRadius;
float3 viewOrigin;

#ifdef HAS_ALPHAFUNC
int alphaFuncType;
float alphaFuncValue;
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
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 positionRAW : TEXCOORD1;
	float3 normal : TEXCOORD2;
};

PixelInputType VS_SimpleTexturing(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

	output.positionRAW = input.position;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
  
    output.tex = input.tex;

	output.normal = input.normal;

	return output;
}

float4 PS_SimpleTexturing(PixelInputType input) : SV_Target
{
	float4 textureColor;
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
	
	float3 lightToVert = lightOrigin - input.positionRAW;
	float distance = length(lightToVert);
	if(distance > lightRadius) {
		// pixel is too far from the light
		return float4(0,0,0,0);
	}
    float3 lightDirection = normalize(lightToVert);
    // calculate the diffuse value based on light angle	
    float angleFactor = dot(input.normal, lightDirection);
    if(angleFactor < 0) {
		// light is behind the surface
		return float4(0,0,0,0);
    }
	//  apply distnace scale
  	float distanceFactor = 1 - distance / lightRadius;
  	
	textureColor *= (distanceFactor * angleFactor);

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
