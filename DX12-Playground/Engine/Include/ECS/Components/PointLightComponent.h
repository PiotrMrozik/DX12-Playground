#pragma once
#include <DirectXMath.h>

struct PointLightComponent
{
    DirectX::XMFLOAT3 color    { 1.f, 1.f, 1.f }; // Linear RGB
    float             radius   { 10.f };            // Influence radius in world units
    float             intensity{  1.f };
};
