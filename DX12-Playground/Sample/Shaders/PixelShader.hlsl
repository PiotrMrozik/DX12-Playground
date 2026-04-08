cbuffer CameraData : register(b0)
{
    float4 CameraPos;
};

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 PositionWorld : POSITIONWORLD;
    float4 Normal : NORMAL;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    float3 color = float3(1.0f, 1.0f, 1.0f);
    float3 norm     = normalize(IN.Normal.xyz);
    float3 lightDir = normalize(float3(5.0f, 10.0f, 2.0f) - IN.PositionWorld.xyz);
   
    
    float  diff     = max(dot(norm, lightDir), 0.0f);
    float3 diffuse  = diff * color;
    
    // specular
    float3 viewDir = normalize(CameraPos.xyz - IN.PositionWorld.xyz);
    float3 reflectDir = reflect(-lightDir, norm);
    
    float3 halfwayDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(norm, halfwayDir), 0.0), 128.0);


    return float4(diffuse + specular, 1.0f);
}
