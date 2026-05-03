#include "ACPanel.h"
#include <string>

ACPanel::ACPanel(std::shared_ptr<SmartAC> mainAC)
    : m_mainAC(mainAC) {}

void ACPanel::render() {
    ImGui::BeginChild("ACPanel", ImVec2(0, 115), true);

    ImGui::Text("Main AC");
    ImGui::SameLine(150);
    
    // Status Badge
    if (m_mainAC->getPowerStatus()) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "● ON");
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "● OFF");
    }
    
    ImGui::SameLine(215);

    // Temperature slider
    ImGui::SetNextItemWidth(200);
    if (ImGui::SliderFloat("##actemp", &m_acTemperature, 16.0f, 30.0f, "%.1f °C")) {
        m_mainAC->setTemperature(static_cast<double>(m_acTemperature));
    }
    ImGui::SameLine(430);
    ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "%.0f W", m_mainAC->ratedWattage());

    // Mode selector
    ImGui::SetCursorPosX(150);
    const char* modes[] = { "COOLING", "HEATING", "FAN ONLY" };
    ImGui::SetNextItemWidth(160);
    if (ImGui::Combo("##acmode", &m_acModeIndex, modes, 3)) {
        SmartAC::Mode m[3] = {
            SmartAC::Mode::COOLING, SmartAC::Mode::HEATING, SmartAC::Mode::FAN_ONLY
        };
        m_mainAC->setMode(m[m_acModeIndex]);
    }
    ImGui::SameLine(330);
    if (ImGui::SmallButton(m_mainAC->getPowerStatus() ? "Power Off##ac" : "Power On##ac")) {
        if (m_mainAC->getPowerStatus()) {
            m_mainAC->turnOff();
        } else {
            m_mainAC->turnOn();
        }
    }
    ImGui::EndChild();
}
