#include "DoorPanel.h"
#include <string>

DoorPanel::DoorPanel(std::shared_ptr<SmartDoor> frontDoor,
                     std::shared_ptr<SmartDoor> backDoor)
    : m_frontDoor(frontDoor), m_backDoor(backDoor) {}

void DoorPanel::render() {
    ImGui::BeginChild("DoorPanel", ImVec2(0, 130), true);

    renderDoorRow("Front Door", m_frontDoor, "door1");
    ImGui::Separator();
    renderDoorRow("Back Door", m_backDoor, "door2");

    ImGui::EndChild();
}

void DoorPanel::renderDoorRow(const char* doorName,
                              std::shared_ptr<SmartDoor> door,
                              const char* idSuffix) {
    // Door name
    ImGui::Text("%s", doorName);
    ImGui::SameLine();

    // Lock status badge
    if (door->getLockStatus()) {
        ImGui::TextColored(ImVec4(0, 1, 0.5f, 1), "● LOCKED"); // green
    } else {
        ImGui::TextColored(ImVec4(1, 0.65f, 0, 1), "● UNLOCKED"); // amber
    }
    ImGui::SameLine();

    // Power status badge
    if (door->getPowerStatus()) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "● ON"); // green
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "● OFF"); // red
    }

    // Action buttons
    if (ImGui::SmallButton(std::string("Lock##").append(idSuffix).c_str())) {
        door->lock(true);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(std::string("Unlock##").append(idSuffix).c_str())) {
        door->unlock(true);
    }
    ImGui::SameLine();
    if (door->getPowerStatus()) {
        if (ImGui::SmallButton(std::string("Power Off##").append(idSuffix).c_str())) {
            door->turnOff();
        }
    } else {
        if (ImGui::SmallButton(std::string("Power On##").append(idSuffix).c_str())) {
            door->turnOn();
        }
    }
}
