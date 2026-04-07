struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct VertexShaderOutput
{
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

VertexShaderOutput main(VertexInput IN)
{
    VertexShaderOutput OUT;
    float4 worldPos   = mul(World, float4(IN.Position, 1.0f));
    OUT.Position      = mul(VP, worldPos);
    OUT.PositionWorld = worldPos;
    OUT.Normal        = float4(mul((float3x3)World, IN.Normal), 0.0f);
    return OUT;
}
