#ifndef CAMERAPANEL_H
#define CAMERAPANEL_H

#include <memory>
#include <string>
#include "../include/SmartCamera.h"
#include "imgui.h"

class CameraPanel {
public:
    CameraPanel(std::shared_ptr<SmartCamera> front,
                std::shared_ptr<SmartCamera> back);

    void render();

private:
    std::shared_ptr<SmartCamera> m_front;
    std::shared_ptr<SmartCamera> m_back;
};

#endif // CAMERAPANEL_H
