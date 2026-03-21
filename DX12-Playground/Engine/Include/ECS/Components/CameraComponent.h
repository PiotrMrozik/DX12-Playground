#pragma once
#include <cstdint>
#include <DirectXMath.h>

enum class ProjectionType : std::uint8_t
{
    Perspective,
    Orthographic
};

struct CameraComponent
{
    float          fovY          { DirectX::XM_PIDIV4 }; // Vertical FOV in radians (45 deg default)
    float          nearPlane     { 0.1f };
    float          farPlane      { 1000.f };
    ProjectionType projectionType{ ProjectionType::Perspective };

    // Orthographic half-extents (used when projectionType == Orthographic)
    float orthoWidth { 16.f };
    float orthoHeight{  9.f };
};
