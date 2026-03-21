struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float3 Normal : NORMAL;
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
    float4 PositionWorld : POSITIONWORLD;
    float4 Normal : NORMAL;
};

cbuffer WorldMatrix : register(b0)
{
    matrix World;
}

cbuffer ViewProjection : register(b1)
{
    matrix VP;
}

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    float4 worldPos  = mul(World, float4(IN.Position/10, 1.0f));
    OUT.Position      = mul(VP, worldPos);
    OUT.PositionWorld = worldPos;
    OUT.Color         = float4(IN.Color, 1.0f);
    OUT.Normal        = float4(mul((float3x3)World, IN.Normal), 0.0f);
    return OUT;
}
