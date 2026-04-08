#pragma once
#include <imgui.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/TagComponent.h>
#include <ECS/Components/MeshComponent.h>
#include <ECS/Components/DirectionalLightComponent.h>
#include <ECS/Components/PointLightComponent.h>
#include <ECS/Components/RigidBodyComponent.h>

inline void InspectComponent(TransformComponent& tc)
{
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Position", &tc.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &tc.rotation.x, 0.01f);
        ImGui::DragFloat3("Scale",    &tc.scale.x,    0.05f);
    }
}

inline void InspectComponent(TagComponent& /*tag*/)
{
    // Tag is shown as a header in InspectorPanel — no separate section needed.
}

inline void InspectComponent(MeshComponent& mc)
{
    if (ImGui::CollapsingHeader("Mesh"))
        ImGui::Text("Mesh index: %u", mc.meshIndex);
}

inline void InspectComponent(DirectionalLightComponent& dl)
{
    if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Direction", &dl.direction.x, 0.01f);
        ImGui::ColorEdit3("Color",     &dl.color.x);
        ImGui::DragFloat ("Intensity", &dl.intensity, 0.1f, 0.0f, 100.0f);
    }
}

inline void InspectComponent(PointLightComponent& pl)
{
    if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Color",    &pl.color.x);
        ImGui::DragFloat ("Intensity", &pl.intensity, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat ("Radius",    &pl.radius,    0.1f, 0.0f,  50.0f);
    }
}

inline void InspectComponent(RigidBodyComponent& rb)
{
    if (ImGui::CollapsingHeader("Rigid Body"))
    {
        ImGui::DragFloat3("Velocity",   &rb.velocity.x,        0.1f);
        ImGui::DragFloat3("Angular V",  &rb.angularVelocity.x, 0.01f);
        ImGui::DragFloat ("Mass",       &rb.mass,              0.1f, 0.0f, 1000.0f);
    }
}
