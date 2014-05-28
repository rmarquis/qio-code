matrix worldMatrix;
matrix viewMatrix;
matrix projectionMatrix;

struct VertexInputType
{
    float4 position : POSITION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
};

PixelInputType VS_SimpleTexturing(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	return output;
}

float4 PS_SimpleTexturing(PixelInputType input) : SV_Target
{
    return float4(0,0,0,0);
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
