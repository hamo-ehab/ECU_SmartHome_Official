#ifndef ACPANEL_H
#define ACPANEL_H

#include <memory>
#include "../include/SmartAC.h"
#include "imgui.h"

class ACPanel {
public:
    ACPanel(std::shared_ptr<SmartAC> mainAC);

    void render();

private:
    std::shared_ptr<SmartAC> m_mainAC;
    float m_acTemperature { 22.0f };
    int   m_acModeIndex   { 0 }; // 0=Cooling 1=Heating 2=Fan
};

#endif // ACPANEL_H
