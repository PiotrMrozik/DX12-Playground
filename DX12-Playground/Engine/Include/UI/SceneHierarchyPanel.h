#pragma once
#include <string>

#include <imgui.h>

#include <ECS/Globals.h>
#include <ECS/World.h>
#include <ECS/Components/TagComponent.h>
#include <UI/UIPanel.h>

class SceneHierarchyPanel : public UIPanel
{
public:
    SceneHierarchyPanel(World& world)
        : UIPanel("Scene Hierarchy"), m_World(world) {}

    void Draw() override;

    Entity GetSelectedEntity() const { return m_SelectedEntity; }

private:
    World&  m_World;
    Entity  m_SelectedEntity = INVALID_ENTITY;
};
