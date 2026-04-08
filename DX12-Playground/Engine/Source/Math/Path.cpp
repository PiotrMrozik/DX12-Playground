#include <DX12LibPCH.h>
#include <Math/Path.h>

#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

namespace Math {

static constexpr float kFiniteDiffEps = 1e-4f;

XMFLOAT3 Path::Tangent(float t) const
{
    // Central differences, clamped so both samples stay in [0, 1].
    float t0 = t - kFiniteDiffEps;
    float t1 = t + kFiniteDiffEps;

    if (t0 < 0.f) { t0 = 0.f; t1 = 2.f * kFiniteDiffEps; }
    if (t1 > 1.f) { t1 = 1.f; t0 = 1.f - 2.f * kFiniteDiffEps; }

    XMFLOAT3 p0 = Evaluate(t0);
    XMFLOAT3 p1 = Evaluate(t1);

    XMVECTOR v = XMVectorSubtract(XMLoadFloat3(&p1), XMLoadFloat3(&p0));
    v = XMVector3Normalize(v);

    XMFLOAT3 result;
    XMStoreFloat3(&result, v);
    return result;
}

float Path::ArcLength(uint32_t samples) const
{
    if (samples < 2) samples = 2;

    float length = 0.f;
    XMFLOAT3 prev = Evaluate(0.f);

    for (uint32_t i = 1; i <= samples; ++i)
    {
        float    t    = static_cast<float>(i) / static_cast<float>(samples);
        XMFLOAT3 curr = Evaluate(t);

        XMVECTOR delta = XMVectorSubtract(XMLoadFloat3(&curr), XMLoadFloat3(&prev));
        length += XMVectorGetX(XMVector3Length(delta));

        prev = curr;
    }

    return length;
}

} // namespace Math
