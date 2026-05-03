#ifndef DOOR_PANEL_H
#define DOOR_PANEL_H

#include <memory>
#include "../include/SmartDoor.h"
#include "imgui/imgui.h"

class DoorPanel {
public:
    DoorPanel(std::shared_ptr<SmartDoor> frontDoor, std::shared_ptr<SmartDoor> backDoor);
    void render();

private:
    void renderDoorRow(const char* doorName, std::shared_ptr<SmartDoor> door, const char* idSuffix);
    std::shared_ptr<SmartDoor> m_frontDoor;
    std::shared_ptr<SmartDoor> m_backDoor;
};

#endif
