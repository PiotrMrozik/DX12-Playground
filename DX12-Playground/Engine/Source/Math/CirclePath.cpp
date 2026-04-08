#include <DX12LibPCH.h>
#include <Math/CirclePath.h>

#include <DirectXMath.h>

using namespace DirectX;

namespace Math {

CirclePath::CirclePath(XMFLOAT3 center, float radius, XMFLOAT3 normal)
    : m_Center(center)
    , m_Radius(radius)
{
    // Normalise the provided axis and store it.
    XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&normal));
    XMStoreFloat3(&m_Normal, n);

    // Choose an arbitrary vector not parallel to n to build the first basis vector.
    XMVECTOR ref = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    if (XMVectorGetX(XMVectorAbs(XMVector3Dot(n, ref))) > 0.99f)
        ref = XMVectorSet(1.f, 0.f, 0.f, 0.f);

    XMVECTOR right = XMVector3Normalize(XMVector3Cross(ref, n));
    XMVECTOR up    = XMVector3Cross(n, right); // already unit length

    XMStoreFloat3(&m_Right, right);
    XMStoreFloat3(&m_Up,    up);
}

XMFLOAT3 CirclePath::Evaluate(float t) const
{
    const float angle = t * XM_2PI;
    const float c     = cosf(angle);
    const float s     = sinf(angle);

    XMVECTOR pos = XMLoadFloat3(&m_Center);
    pos = XMVectorAdd(pos, XMVectorScale(XMLoadFloat3(&m_Right), m_Radius * c));
    pos = XMVectorAdd(pos, XMVectorScale(XMLoadFloat3(&m_Up),    m_Radius * s));

    XMFLOAT3 result;
    XMStoreFloat3(&result, pos);
    return result;
}

XMFLOAT3 CirclePath::Tangent(float t) const
{
    // d/dt [ cos(2πt)*right + sin(2πt)*up ] = 2π * (-sin(2πt)*right + cos(2πt)*up)
    // Normalized, the 2π factor cancels.
    const float angle = t * XM_2PI;
    const float c     = cosf(angle);
    const float s     = sinf(angle);

    XMVECTOR tangent = XMVectorAdd(
        XMVectorScale(XMLoadFloat3(&m_Right), -s),
        XMVectorScale(XMLoadFloat3(&m_Up),     c));

    tangent = XMVector3Normalize(tangent);

    XMFLOAT3 result;
    XMStoreFloat3(&result, tangent);
    return result;
}

} // namespace Math
