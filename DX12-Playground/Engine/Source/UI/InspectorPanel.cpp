#include <DX12LibPCH.h>
#include <UI/InspectorPanel.h>

void InspectorPanel::Draw()
{
    ImGui::Begin("Inspector");

    Entity entity = m_Hierarchy.GetSelectedEntity();

    if (entity == INVALID_ENTITY)
    {
        ImGui::TextDisabled("No entity selected");
    }
    else
    {
        if (m_World.HasComponent<TagComponent>(entity))
            ImGui::Text("%s", m_World.GetComponent<TagComponent>(entity).name.c_str());
        else
            ImGui::Text("Entity #%u", entity);

        ImGui::Separator();
        m_Registry.InspectEntity(m_World, entity);
    }

    ImGui::End();
}
