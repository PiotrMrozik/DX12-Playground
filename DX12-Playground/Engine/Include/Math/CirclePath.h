#pragma once

#include <Math/Path.h>
#include <DirectXMath.h>

namespace Math {

// A full circle in 3D space.
// t = 0 and t = 1 both map to the same point, completing one revolution.
// The circle lies in the plane perpendicular to 'normal'.
class CirclePath : public Path
{
public:
    // center  - world-space center of the circle
    // radius  - circle radius
    // normal  - axis the circle is perpendicular to (default: Y-up)
    CirclePath(DirectX::XMFLOAT3 center, float radius,
               DirectX::XMFLOAT3 normal = { 0.f, 1.f, 0.f });

    DirectX::XMFLOAT3 Evaluate(float t) const override;
    DirectX::XMFLOAT3 Tangent(float t) const override;

    DirectX::XMFLOAT3 GetCenter() const { return m_Center; }
    float             GetRadius() const { return m_Radius; }
    DirectX::XMFLOAT3 GetNormal() const { return m_Normal; }

private:
    DirectX::XMFLOAT3 m_Center;
    float             m_Radius;
    DirectX::XMFLOAT3 m_Normal;  // axis the circle is perpendicular to (normalised)
    // Precomputed orthonormal basis spanning the circle's plane.
    DirectX::XMFLOAT3 m_Right;   // points to t=0
    DirectX::XMFLOAT3 m_Up;      // 90° ahead in the winding direction
};

} // namespace Math
