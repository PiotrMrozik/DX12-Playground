#pragma once
#include <DirectXMath.h>

struct TransformComponent
{
    DirectX::XMFLOAT3 position{ 0.f, 0.f, 0.f };
    DirectX::XMFLOAT3 rotation{ 0.f, 0.f, 0.f }; // Euler angles (radians): pitch, yaw, roll
    DirectX::XMFLOAT3 scale{ 1.f, 1.f, 1.f };

    DirectX::XMMATRIX GetWorldMatrix() const
    {
        using namespace DirectX;
        return XMMatrixScaling(scale.x, scale.y, scale.z)
             * XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)
             * XMMatrixTranslation(position.x, position.y, position.z);
    }
};
