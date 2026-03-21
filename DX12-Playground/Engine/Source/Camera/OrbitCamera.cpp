#include <Camera/OrbitCamera.h>

using namespace DirectX;

XMFLOAT3 OrbitCamera::GetPosition() const
{
    return {
        target.x + radius * sinf(pitch) * sinf(yaw),
        target.y + radius * cosf(pitch),
        target.z + radius * sinf(pitch) * cosf(yaw)
    };
}

void OrbitCamera::UpdateBasis()
{
    XMFLOAT3 pos = GetPosition();
    XMVECTOR eye     = XMLoadFloat3(&pos);
    XMVECTOR tgt     = XMLoadFloat3(&target);
    XMVECTOR worldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    XMVECTOR f = XMVector3Normalize(XMVectorSubtract(tgt, eye));
    XMVECTOR r = XMVector3Normalize(XMVector3Cross(worldUp, f));
    XMVECTOR u = XMVector3Cross(f, r);

    XMStoreFloat3(&front, f);
    XMStoreFloat3(&right, r);
    XMStoreFloat3(&up,    u);
}

void OrbitCamera::Pan(float dx, float dy)
{
    UpdateBasis();

    float scale = rotationSensitivity * radius;

    XMVECTOR tgt = XMLoadFloat3(&target);
    XMVECTOR r   = XMLoadFloat3(&right);
    XMVECTOR u   = XMLoadFloat3(&up);

    tgt = XMVectorAdd(tgt, XMVectorScale(r, -dx * scale));
    tgt = XMVectorAdd(tgt, XMVectorScale(u,   dy * scale));

    XMStoreFloat3(&target, tgt);
}

XMMATRIX OrbitCamera::GetViewMatrix() const
{
    XMFLOAT3 pos   = GetPosition();
    XMVECTOR eye   = XMLoadFloat3(&pos);
    XMVECTOR focus = XMLoadFloat3(&target);
    XMVECTOR up    = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    return XMMatrixLookAtLH(eye, focus, up);
}

XMMATRIX OrbitCamera::GetProjMatrix(float aspectRatio) const
{
    return XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearPlane, farPlane);
}

XMMATRIX OrbitCamera::GetViewProjMatrix(float aspectRatio) const
{
    return GetViewMatrix() * GetProjMatrix(aspectRatio);
}
