#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

#include <UI/UIPanel.h>

class UILayer
{
public:
    UILayer() = default;

    void AddPanel(std::unique_ptr<UIPanel> panel);
    void AddCustomPanel(const std::string& name, std::function<void()> drawFn);

    // Called from Game::OnImGui(). Sets up dockspace and draws all panels.
    void Draw();

private:
    class LambdaPanel : public UIPanel
    {
    public:
        LambdaPanel(std::string name, std::function<void()> fn)
            : UIPanel(std::move(name)), m_DrawFn(std::move(fn)) {}
        void Draw() override { m_DrawFn(); }
    private:
        std::function<void()> m_DrawFn;
    };

    std::vector<std::unique_ptr<UIPanel>> m_Panels;
    bool m_DefaultLayoutApplied = false;
};
