#include <DX12LibPCH.h>
#include <UI/SceneHierarchyPanel.h>

void SceneHierarchyPanel::Draw()
{
    ImGui::Begin("Scene Hierarchy");

    for (Entity entity : m_World.GetLivingEntities())
    {
        std::string label;
        if (m_World.HasComponent<TagComponent>(entity))
            label = m_World.GetComponent<TagComponent>(entity).name;
        else
            label = "Entity #" + std::to_string(entity);

        const bool selected = (entity == m_SelectedEntity);
        std::string selectableId = label + "##" + std::to_string(entity);
        if (ImGui::Selectable(selectableId.c_str(), selected))
            m_SelectedEntity = entity;
    }

    ImGui::End();
}
