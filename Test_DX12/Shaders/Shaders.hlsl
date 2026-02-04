
cbuffer cbPerObject : register(bO)
{
    float4x4 gWorldViewProj;
}

struct VertexIn
{
    float3 Pos;
    float4 Color;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn input)
{
    VertexOut output;

    // Transform to homogenous clip space.
    output.PosH = mul(float4(input.Pos, 1.0f), gWorldViewProj);

    output.Color = input.Color;
    
    return output;
}

float4 PS(VertexOut pinput)
{
    return pinput.Color;
}