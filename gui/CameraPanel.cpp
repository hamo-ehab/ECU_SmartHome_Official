#include "CameraPanel.h"
#include <cmath>

CameraPanel::CameraPanel(std::shared_ptr<SmartCamera> front,
                         std::shared_ptr<SmartCamera> back)
    : m_front(front), m_back(back) {}

void CameraPanel::render() {
    ImGui::BeginChild("CamPanel", ImVec2(0, 120), true);

    // Motion banner
    bool anyRec = m_front->isRecording() || m_back->isRecording();
    float alpha = 0.6f + 0.4f * sinf(ImGui::GetTime() * 4.0f);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImU32 col = anyRec
        ? ImGui::GetColorU32(ImVec4(1.0f, 0.6f, 0.0f, alpha)) // amber
        : ImGui::GetColorU32(ImVec4(0.0f, 0.6f, 0.0f, 0.8f)); // dim green

    dl->AddRectFilled(p, ImVec2(p.x + availW, p.y + 20), col);
    ImGui::SetCursorScreenPos(ImVec2(p.x + availW / 2 - 60, p.y + 2));
    ImGui::TextColored(ImVec4(1,1,1,1), anyRec ? "^ MOTION DETECTED" : "v ALL CLEAR");
    ImGui::Dummy(ImVec2(0, 22));

    // Camera row helper
    auto renderCameraRow = [&](std::shared_ptr<SmartCamera> cam, const char* camName, int idx) {
        ImGui::Text("%-14s", camName);

        ImGui::SameLine(150);
        bool powered = cam->getPowerStatus();
        ImVec4 powerColor = powered ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1);
        ImGui::TextColored(powerColor, powered ? "ON" : "OFF");

        ImGui::SameLine(215);
        bool recording = cam->isRecording();
        ImVec4 recColor = recording ? ImVec4(1,0.6f,0,1) : ImVec4(0.5f,0.5f,0.5f,1);
        ImGui::TextColored(recColor, recording ? ". REC" : "o IDLE");

        ImGui::SameLine();
        if (ImGui::SmallButton(recording ? ("Stop Rec##cam" + std::to_string(idx)).c_str()
                                         : ("Start Rec##cam" + std::to_string(idx)).c_str())) {
            cam->toggleRecording();
        }

        ImGui::SameLine();
        if (ImGui::SmallButton(("Alert##cam" + std::to_string(idx)).c_str())) {
            cam->raiseMotionAlert(true);
        }

        ImGui::SameLine();
        if (ImGui::SmallButton(("On/Off##cam" + std::to_string(idx)).c_str())) {
            if (powered) cam->turnOff();
            else cam->turnOn();
        }
    };

    renderCameraRow(m_front, "Front Camera", 0);
    renderCameraRow(m_back, "Back Camera", 1);

    ImGui::EndChild();
}
