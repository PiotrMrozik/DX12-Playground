struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 PositionWorld : POSITIONWORLD;
    float4 Normal : NORMAL;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    float3 norm     = normalize(IN.Normal.xyz);
    float3 lightDir = normalize(float3(12.0f, 5.0f, 0.0f) - IN.PositionWorld.xyz);
    float  diff     = max(dot(norm, lightDir), 0.0f);
    float3 diffuse  = diff * float3(1.5f, 1.5f, 1.5f);

    return float4(diffuse, 1.0f);
}
