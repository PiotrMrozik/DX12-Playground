#pragma once

#include <Math/Path.h>
#include <DirectXMath.h>
#include <vector>

namespace Math {

// Bezier curve of arbitrary degree defined by N control points.
// Evaluation uses the de Casteljau algorithm (numerically stable).
// Tangent uses the analytical derivative: a degree-(N-1) Bezier
// over the forward-difference control points, scaled by N.
class BezierPath : public Path
{
public:
    explicit BezierPath(std::vector<DirectX::XMFLOAT3> controlPoints);

    DirectX::XMFLOAT3 Evaluate(float t) const override;
    DirectX::XMFLOAT3 Tangent(float t) const override;

    const std::vector<DirectX::XMFLOAT3>& GetControlPoints() const { return m_ControlPoints; }

private:
    std::vector<DirectX::XMFLOAT3> m_ControlPoints;
};

} // namespace Math
