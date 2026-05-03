#include "LightPanel.h"

LightPanel::LightPanel(std::shared_ptr<SmartLight> living, 
                       std::shared_ptr<SmartLight> bed, 
                       std::shared_ptr<SmartLight> kitchen) {
    m_lights[0] = living;
    m_lights[1] = bed;
    m_lights[2] = kitchen;

    for (int i = 0; i < 3; ++i) 
    {
        if (m_lights[i]) 
        {
            m_bri[i] = m_lights[i]->getBrightness();
        } 
        else 
        {
            m_bri[i] = 0;
        }
    }
}

void LightPanel::render() 
{
    ImGui::BeginChild("LightPanel", {0.0f, 160.0f}, true);

    for (int i = 0; i < 3; ++i) 
    {
        if (!m_lights[i]) continue;

        ImGui::Text("%-14s", m_lights[i]->getDeviceName().c_str());
        ImGui::SameLine(150.0f);

        if (m_lights[i]->getPowerStatus()) 
        {
            ImGui::TextColored({0.0f, 1.0f, 0.0f, 1.0f}, "[● ON]"); 
        } 
        else 
        {
            ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "[○ OFF]");
        }
        ImGui::SameLine();

        float t = m_bri[i] / 100.0f;
        int r = (int)(t * 1.0f * 255);
        int g = (int)(t * 0.97f * 255);
        int b = (int)(t * 0.75f * 255);
        unsigned int col32 = (255 << 24) | (b << 16) | (g << 8) | r;

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(p, {p.x + 16, p.y + 16}, col32);
        
        ImGui::Dummy({20, 16});
        ImGui::SameLine();

        ImGui::SetNextItemWidth(160.0f);
        std::string sliderId = "##bri" + std::to_string(i); 
        
        if (ImGui::SliderInt(sliderId.c_str(), &m_bri[i], 0, 100)) 
        {
            m_lights[i]->setBrightness(m_bri[i]);
        }
        ImGui::SameLine();

        ImGui::TextColored({1.0f, 0.75f, 0.0f, 1.0f}, "%.1f W", m_lights[i]->ratedWattage());
        ImGui::SameLine();

        std::string btnId = "On/Off##light" + std::to_string(i);
        if (ImGui::SmallButton(btnId.c_str())) 
        {
            if (m_lights[i]->getPowerStatus())
                m_lights[i]->turnOff();
            else
                m_lights[i]->turnOn();
            m_bri[i] = m_lights[i]->getPowerStatus() ? 100 : 0; 
            m_lights[i]->setBrightness(m_bri[i]);
        }
    }
    ImGui::EndChild();
}