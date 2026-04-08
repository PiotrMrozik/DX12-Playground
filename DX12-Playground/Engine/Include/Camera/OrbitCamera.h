#pragma once
#include <DirectXMath.h>

// Standalone orbit camera - not an ECS component.
// Owned directly by World and updated by Game each frame.
// Orbits around `target` using spherical coordinates (yaw / pitch).
class OrbitCamera
{
public:
    // --- Orbit parameters ---
    DirectX::XMFLOAT3 target{ 0.f, 0.f, 0.f }; // Point to orbit around
    float             radius{ 10.f };            // Distance from target
    float             yaw   { 0.f };             // Horizontal angle (radians)
    float             pitch { 0.4f };            // Vertical angle (radians)

    static constexpr float MinPitch{  0.05f };
    static constexpr float MaxPitch{ DirectX::XM_PI - 0.05f };

    // --- Input sensitivity ---
    float rotationSensitivity{ 0.005f }; // Radians per pixel (drag)
    float panSensitivity{ 0.05f }; // Meters per pixel (drag)
    float zoomSpeed  { 0.5f   }; // World units per wheel tick

    // --- Projection parameters ---
    float fovY     { DirectX::XM_PIDIV4 };
    float nearPlane{ 0.1f };
    float farPlane { 1000.f };

    // --- Derived basis vectors (world-space) ---
    // Call UpdateBasis() after any change to yaw/pitch/radius/target.
    DirectX::XMFLOAT3 right{ 1.f, 0.f, 0.f };
    DirectX::XMFLOAT3 up   { 0.f, 1.f, 0.f };
    DirectX::XMFLOAT3 front{ 0.f, 0.f, 1.f };

    // Recompute right/up/front from current spherical coordinates.
    void UpdateBasis();

    // Translate both target and eye in the right/up plane.
    // dx/dy are raw pixel deltas; internally scaled by sensitivity * radius.
    void Pan(float dx, float dy);

    // --- Queries ---
    DirectX::XMFLOAT3 GetPosition()                       const;
    DirectX::XMMATRIX GetViewMatrix()                     const;
    DirectX::XMMATRIX GetProjMatrix(float aspectRatio)    const;
    DirectX::XMMATRIX GetViewProjMatrix(float aspectRatio) const;
};
