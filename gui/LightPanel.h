#ifndef LIGHTPANEL_H
#define LIGHTPANEL_H

#include <memory>
#include <string>
#include "../include/SmartLight.h" 
#include "imgui.h"                 

class LightPanel 
{
private:
    std::shared_ptr<SmartLight> m_lights[3]; 
    int m_bri[3]; 

public:
    LightPanel(std::shared_ptr<SmartLight> living, 
               std::shared_ptr<SmartLight> bed, 
               std::shared_ptr<SmartLight> kitchen);

    void render();
};

#endif