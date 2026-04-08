#pragma once
#include <DirectXMath.h>

struct RigidBodyComponent
{
    DirectX::XMFLOAT3 velocity       { 0.f, 0.f, 0.f }; // Linear velocity (m/s)
    DirectX::XMFLOAT3 angularVelocity{ 0.f, 0.f, 0.f }; // Angular velocity (rad/s), axis-angle magnitude
    float             mass           { 1.f };             // kg; 0 == static/kinematic body
};
