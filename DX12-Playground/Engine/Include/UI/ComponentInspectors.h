#pragma once
#include <imgui.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/TagComponent.h>
#include <ECS/Components/MeshComponent.h>
#include <ECS/Components/DirectionalLightComponent.h>
#include <ECS/Components/PointLightComponent.h>
#include <ECS/Components/RigidBodyComponent.h>
#include <ECS/Components/PathFollowerComponent.h>
#include <Math/BezierPath.h>
#include <Math/CirclePath.h>

#include <memory>
#include <vector>

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

inline void InspectComponent(PathFollowerComponent& pf)
{
    if (!ImGui::CollapsingHeader("Path Follower", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    // --- Playback controls ---
    if (pf.playing)
    { if (ImGui::Button("Pause")) pf.playing = false; }
    else
    { if (ImGui::Button("Play"))  pf.playing = true;  }

    ImGui::SameLine();
    ImGui::Checkbox("Loop",          &pf.loop);
    ImGui::Checkbox("Orient to path",&pf.orientToPath);
    ImGui::DragFloat("Speed",        &pf.speed, 0.05f, 0.f, 100.f, "%.2f");
    ImGui::SliderFloat("t",          &pf.t, 0.f, 1.f);

    ImGui::Separator();

    // --- Path type selector ---
    static const char* kTypes[] = { "Circle", "Bezier" };
    auto* circle = pf.path ? dynamic_cast<Math::CirclePath*>(pf.path.get()) : nullptr;
    auto* bezier = pf.path ? dynamic_cast<Math::BezierPath*>(pf.path.get()) : nullptr;
    int   current = circle ? 0 : (bezier ? 1 : -1);

    if (!pf.path)
    {
        ImGui::TextDisabled("No path assigned");
        if (ImGui::Button("Create Circle"))
        {
            pf.path      = std::make_shared<Math::CirclePath>(
                               DirectX::XMFLOAT3{0.f, 0.f, 0.f}, 5.f);
            pf.arcLength = -1.f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Create Bezier"))
        {
            pf.path      = std::make_shared<Math::BezierPath>(
                               std::vector<DirectX::XMFLOAT3>{
                                   {-3.f, 0.f,  0.f},
                                   { 0.f, 3.f,  0.f},
                                   { 3.f, 0.f,  0.f}});
            pf.arcLength = -1.f;
        }
        return;
    }

    // Allow switching type.
    if (ImGui::Combo("Type", &current, kTypes, 2))
    {
        if (current == 0)
        {
            pf.path      = std::make_shared<Math::CirclePath>(
                               DirectX::XMFLOAT3{0.f, 0.f, 0.f}, 5.f);
        }
        else
        {
            pf.path      = std::make_shared<Math::BezierPath>(
                               std::vector<DirectX::XMFLOAT3>{
                                   {-3.f, 0.f, 0.f},
                                   { 0.f, 3.f, 0.f},
                                   { 3.f, 0.f, 0.f}});
        }
        pf.arcLength = -1.f;
        // Re-derive pointers after replacement.
        circle = dynamic_cast<Math::CirclePath*>(pf.path.get());
        bezier = dynamic_cast<Math::BezierPath*>(pf.path.get());
    }

    // --- Circle path editor ---
    if (circle)
    {
        DirectX::XMFLOAT3 center = circle->GetCenter();
        DirectX::XMFLOAT3 normal = circle->GetNormal();
        float             radius = circle->GetRadius();

        bool changed = false;
        changed |= ImGui::DragFloat3("Center", &center.x, 0.1f);
        changed |= ImGui::DragFloat3("Normal", &normal.x, 0.01f, -1.f, 1.f);
        changed |= ImGui::DragFloat ("Radius", &radius,   0.1f,  0.01f, 200.f);

        if (changed)
        {
            pf.path      = std::make_shared<Math::CirclePath>(center, radius, normal);
            pf.arcLength = -1.f;
        }
    }

    // --- Bezier path editor ---
    if (bezier)
    {
        std::vector<DirectX::XMFLOAT3> pts = bezier->GetControlPoints();
        bool changed = false;

        ImGui::Text("Control points (%zu):", pts.size());

        for (int i = 0; i < static_cast<int>(pts.size()); ++i)
        {
            ImGui::PushID(i);
            changed |= ImGui::DragFloat3("##pt", &pts[i].x, 0.1f);
            if (pts.size() > 2)
            {
                ImGui::SameLine();
                if (ImGui::SmallButton("X"))
                {
                    pts.erase(pts.begin() + i);
                    --i;
                    changed = true;
                }
            }
            ImGui::PopID();
        }

        if (ImGui::SmallButton("+ Add point"))
        {
            pts.push_back(pts.back());
            changed = true;
        }

        if (changed)
        {
            pf.path      = std::make_shared<Math::BezierPath>(std::move(pts));
            pf.arcLength = -1.f;
        }
    }
}
