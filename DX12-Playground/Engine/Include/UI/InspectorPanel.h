#pragma once
#include <imgui.h>

#include <ECS/Globals.h>
#include <ECS/World.h>
#include <ECS/Components/TagComponent.h>
#include <UI/UIPanel.h>
#include <UI/SceneHierarchyPanel.h>
#include <UI/ComponentInspectorRegistry.h>

class InspectorPanel : public UIPanel
{
public:
    InspectorPanel(SceneHierarchyPanel& hierarchy,
                   ComponentInspectorRegistry& registry,
                   World& world)
        : UIPanel("Inspector")
        , m_Hierarchy(hierarchy)
        , m_Registry(registry)
        , m_World(world) {}

    void Draw() override;

private:
    SceneHierarchyPanel&        m_Hierarchy;
    ComponentInspectorRegistry& m_Registry;
    World&                      m_World;
};
