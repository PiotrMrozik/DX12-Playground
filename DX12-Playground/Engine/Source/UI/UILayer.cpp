#include <DX12LibPCH.h>
#include <UI/UILayer.h>

#include <cstdio>

// Default docking layout — applied once on first launch (when imgui.ini doesn't exist yet).
// Regenerate by arranging panels to taste, then copying the relevant sections from imgui.ini.
static const char* s_DefaultLayout = R"ini(
[Window][WindowOverViewport_11111111]
Pos=0,0
Size=1600,900
Collapsed=0

[Window][Scene Hierarchy]
Pos=0,0
Size=250,258
Collapsed=0
DockId=0x00000003,0

[Window][Inspector]
Pos=0,279
Size=238,441
Collapsed=0
DockId=0x00000004,0

[Window][Camera]
Pos=0,19
Size=238,258
Collapsed=1
DockId=0x00000003,1

[Docking][Data]
DockSpace     ID=0x08BD597D Window=0x1BBC0F80 Pos=0,19 Size=1280,701 Split=X
  DockNode    ID=0x00000001 Parent=0x08BD597D SizeRef=238,701 Split=Y Selected=0xB8729153
    DockNode  ID=0x00000003 Parent=0x00000001 SizeRef=305,258 Selected=0xB8729153
    DockNode  ID=0x00000004 Parent=0x00000001 SizeRef=305,441 Selected=0x36DC96AB
  DockNode    ID=0x00000002 Parent=0x08BD597D SizeRef=1040,701 CentralNode=1
)ini";

void UILayer::AddPanel(std::unique_ptr<UIPanel> panel)
{
    m_Panels.push_back(std::move(panel));
}

void UILayer::AddCustomPanel(const std::string& name, std::function<void()> drawFn)
{
    m_Panels.push_back(std::make_unique<LambdaPanel>(name, std::move(drawFn)));
}

void UILayer::Draw()
{
    // On first draw, apply the default layout if no imgui.ini exists yet.
    if (!m_DefaultLayoutApplied)
    {
        m_DefaultLayoutApplied = true;

        const char* iniPath = ImGui::GetIO().IniFilename;
        bool iniExists = false;
        if (iniPath)
        {
            FILE* f = nullptr;
            fopen_s(&f, iniPath, "r");
            if (f) { fclose(f); iniExists = true; }
        }

        if (!iniExists)
            ImGui::LoadIniSettingsFromMemory(s_DefaultLayout);
    }

    // Full-window dockspace — PassthruCentralNode keeps the 3D viewport visible.
    ImGui::DockSpaceOverViewport(
        0,
        ImGui::GetMainViewport(),
        ImGuiDockNodeFlags_PassthruCentralNode);

    // Main menu bar: View menu toggles panel visibility.
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            for (auto& panel : m_Panels)
            {
                bool visible = panel->IsVisible();
                if (ImGui::MenuItem(panel->GetName().c_str(), nullptr, &visible))
                    panel->SetVisible(visible);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Draw visible panels.
    for (auto& panel : m_Panels)
    {
        if (panel->IsVisible())
            panel->Draw();
    }
}
