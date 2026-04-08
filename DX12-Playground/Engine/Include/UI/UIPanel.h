#pragma once
#include <string>

class UIPanel
{
public:
    UIPanel(std::string name, bool visible = true)
        : m_Name(std::move(name)), m_Visible(visible) {}

    virtual ~UIPanel() = default;

    virtual void Draw() = 0;

    const std::string& GetName() const { return m_Name; }

    bool IsVisible() const  { return m_Visible; }
    void SetVisible(bool v) { m_Visible = v; }

private:
    std::string m_Name;
    bool        m_Visible;
};
