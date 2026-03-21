#pragma once
#include <ECS/System/System.h>
#include <ECS/World.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/CameraComponent.h>
#include <ECS/Systems/FrameContext.h>

// Required signature: TransformComponent + CameraComponent
//
// Builds the view-projection matrix for the first entity in the set (the active camera)
// and writes it to FrameContext::viewProj.
// Run after TransformSystem so TransformComponent::worldMatrix is already current.
//
// Multi-camera note: to support multiple cameras, extend this to accept an active-camera
// entity ID rather than always picking *entities.begin().
class CameraSystem : public System
{
public:
    // aspectRatio = viewport width / height
    void Update(World& world, float aspectRatio, FrameContext& ctx)
    {
        using namespace DirectX;

        if (entities.empty())
            return;

        Entity      cam = *entities.begin();
        const auto& tc  = world.GetComponent<TransformComponent>(cam);
        const auto& cc  = world.GetComponent<CameraComponent>(cam);

        // Derive forward vector from Euler rotation stored in TransformComponent.
        XMVECTOR eye     = XMLoadFloat3(&tc.position);
        XMVECTOR forward = XMVector3Rotate(
            XMVectorSet(0.f, 0.f, 1.f, 0.f),
            XMQuaternionRotationRollPitchYaw(tc.rotation.x, tc.rotation.y, tc.rotation.z));
        XMVECTOR focus = XMVectorAdd(eye, forward);
        XMVECTOR up    = XMVectorSet(0.f, 1.f, 0.f, 0.f);

        XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);

        XMMATRIX proj;
        if (cc.projectionType == ProjectionType::Perspective)
            proj = XMMatrixPerspectiveFovLH(cc.fovY, aspectRatio, cc.nearPlane, cc.farPlane);
        else
            proj = XMMatrixOrthographicLH(cc.orthoWidth, cc.orthoHeight, cc.nearPlane, cc.farPlane);

        ctx.viewProj = view * proj;
    }
};
