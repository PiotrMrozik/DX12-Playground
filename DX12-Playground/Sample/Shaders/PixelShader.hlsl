#define MAX_POINT_LIGHTS 8

cbuffer CameraData : register(b0)
{
    float4 CameraPos;
};

struct PointLight
{
    float3 position;
    float  radius;
    float3 color;
    float  intensity;
};

cbuffer PointLightBuffer : register(b1)
{
    uint       PointLightCount;
    float3     _padding;
    PointLight PointLights[MAX_POINT_LIGHTS];
};

struct PixelShaderInput
{
    float4 Position      : SV_Position;
    float4 PositionWorld : POSITIONWORLD;
    float4 Normal        : NORMAL;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    float3 norm    = normalize(IN.Normal.xyz);
    float3 viewDir = normalize(CameraPos.xyz - IN.PositionWorld.xyz);

    float3 result = float3(0.03f, 0.03f, 0.03f); // ambient

    for (uint i = 0; i < PointLightCount; ++i)
    {
        PointLight light = PointLights[i];

        float3 toLight = light.position - IN.PositionWorld.xyz;
        float  dist    = length(toLight);

        if (dist >= light.radius)
            continue;

        float3 L = toLight / dist;

        // Quadratic attenuation - full at centre, zero at radius
        float t           = saturate(dist / light.radius);
        float attenuation = (1.0f - t) * (1.0f - t);

        // Diffuse
        float  diff    = max(dot(norm, L), 0.0f);
        float3 diffuse = diff * light.color * light.intensity;

        // Specular (Blinn-Phong)
        float3 halfDir  = normalize(L + viewDir);
        float  spec     = pow(max(dot(norm, halfDir), 0.0f), 128.0f);
        float3 specular = spec * light.color * light.intensity;

        result += (diffuse + specular) * attenuation;
    }

    return float4(result, 1.0f);
}
