/*
 * =============================================================================
 * FILE: include/SmartLight.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Adham Mohamed Mohamed (ID: 692500606)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - SmartLight.h       (primary: setBrightness, ratedWattage stub)
 *   - SmartLight-1.h     (alternate: announceVoiceFeedback override)
 *   - SmartLight.cpp     (setBrightness / turnOff / ratedWattage impls)
 *   - SmartLight-1.cpp   (announceVoiceFeedback implementation)
 *   - Source.cpp         (minimal SmartLight with inline ratedWattage)
 *   - Day4 SmartLight    (serialise / deserialise)
 *   - Day5 SmartLight    (update / tickCounter debug)
 *   - Day6 SmartLight    (displayStatus format)
 *   - Day7 SmartLight    (wattage formula: brightness × 0.75)
 * =============================================================================
 * Smart dimmable light with brightness clamping, proportional wattage,
 * and LightClick VoiceService audio feedback.
 * DECLARATION ONLY — implementations live in src/SmartLight.cpp
 * =============================================================================
 */

#ifndef SMARTLIGHT_H
#define SMARTLIGHT_H

#include "SmartDevice.h"
#include "VoiceService.h"

class SmartLight : public SmartDevice
{
private:
    int  m_brightnessLevel; // Range: 0–100
    int  m_tickCounter = 0; // Incremented every update() tick (Day 5)

public:
    // ---- Constructors / Destructor -----------------------------------------
    explicit SmartLight(int brightnessLevel = 100);
    explicit SmartLight(const std::string& deviceID,
                        const std::string& name,
                        int brightnessLevel = 100);

    SmartLight(const SmartLight&)            = delete;
    SmartLight& operator=(const SmartLight&) = delete;

    virtual ~SmartLight() override = default;

    // ---- Brightness API ----------------------------------------------------
    void setBrightness(int level);
    void turnOff();
    int  getBrightness() const { return m_brightnessLevel; }

    // ---- Voice feedback ----------------------------------------------------
    void announceVoiceFeedback(bool powerOn);

    // ---- SmartDevice overrides ---------------------------------------------
    void   update() override;          // Day 5: tick counter + debug tracing
    double ratedWattage() const override;
    void   displayStatus() const override;

    std::string serialise()   const override;
    void        deserialise(const std::string& data) override;
};

#endif // SMARTLIGHT_H
