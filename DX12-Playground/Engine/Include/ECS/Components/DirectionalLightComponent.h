#pragma once
#include <DirectXMath.h>

struct DirectionalLightComponent
{
    DirectX::XMFLOAT3 direction{ 0.f, -1.f, 0.f }; // World-space direction (should be normalised)
    DirectX::XMFLOAT3 color    { 1.f,  1.f, 1.f }; // Linear RGB
    float             intensity{ 1.f };
};
