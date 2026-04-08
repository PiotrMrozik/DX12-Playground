#pragma once
#include <imgui.h>
#include <UI/UIPanel.h>

class StatsPanel : public UIPanel
{
public:
    StatsPanel()
        : UIPanel("Stats")
    {
        SetVisible(false);
    }

    void Draw() override
    {
        ImGui::Begin("Stats");
        const float fps = ImGui::GetIO().Framerate;
        ImGui::Text("%.1f FPS  (%.2f ms)", fps, 1000.0f / fps);
        ImGui::End();
    }
};
