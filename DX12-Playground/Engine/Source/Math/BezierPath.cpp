#include <DX12LibPCH.h>
#include <Math/BezierPath.h>

#include <DirectXMath.h>
#include <cassert>

using namespace DirectX;

namespace Math {

BezierPath::BezierPath(std::vector<XMFLOAT3> controlPoints)
    : m_ControlPoints(std::move(controlPoints))
{
    assert(m_ControlPoints.size() >= 2 && "BezierPath requires at least 2 control points");
}

// de Casteljau evaluation - O(N^2) in degree, stable for any N.
XMFLOAT3 BezierPath::Evaluate(float t) const
{
    // Copy into a working XMVECTOR buffer.
    const size_t n = m_ControlPoints.size();
    std::vector<XMVECTOR> pts(n);
    for (size_t i = 0; i < n; ++i)
        pts[i] = XMLoadFloat3(&m_ControlPoints[i]);

    // Reduce level by level until one point remains.
    for (size_t r = 1; r < n; ++r)
        for (size_t i = 0; i < n - r; ++i)
            pts[i] = XMVectorLerp(pts[i], pts[i + 1], t);

    XMFLOAT3 result;
    XMStoreFloat3(&result, pts[0]);
    return result;
}

// Analytical tangent: derivative of a degree-N Bezier is a degree-(N-1)
// Bezier with control points Q_i = N * (P_{i+1} - P_i).
XMFLOAT3 BezierPath::Tangent(float t) const
{
    const size_t n = m_ControlPoints.size();
    const float  degree = static_cast<float>(n - 1);

    std::vector<XMVECTOR> dpts(n - 1);
    for (size_t i = 0; i < n - 1; ++i)
    {
        XMVECTOR diff = XMVectorSubtract(
            XMLoadFloat3(&m_ControlPoints[i + 1]),
            XMLoadFloat3(&m_ControlPoints[i]));
        dpts[i] = XMVectorScale(diff, degree);
    }

    // de Casteljau on the derivative control points.
    for (size_t r = 1; r < dpts.size(); ++r)
        for (size_t i = 0; i < dpts.size() - r; ++i)
            dpts[i] = XMVectorLerp(dpts[i], dpts[i + 1], t);

    XMVECTOR tangent = XMVector3Normalize(dpts[0]);

    XMFLOAT3 result;
    XMStoreFloat3(&result, tangent);
    return result;
}

} // namespace Math
