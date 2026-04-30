/*
 * =============================================================================
 * FILE: src/SmartLight.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Adham Mohamed Mohamed (ID: 692500606)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - SmartLight.cpp     (setBrightness / turnOff / ratedWattage body)
 *   - SmartLight-1.cpp   (announceVoiceFeedback body)
 * =============================================================================
 * SmartLight method implementations:
 *   - Brightness clamping with state-machine parity (off at 0)
 *   - Proportional wattage calculation (0–10 W)
 *   - VoiceService LightClick audio feedback
 * =============================================================================
 */

#include "../include/SmartLight.h"
#include <algorithm>  // std::clamp
#include <iostream>
#include <iomanip>

// ---- Constructors ----------------------------------------------------------

SmartLight::SmartLight(int brightnessLevel)
    : SmartDevice(false),
      m_brightnessLevel(std::clamp(brightnessLevel, 0, 100))
{
    if (m_brightnessLevel > 0) m_powerStatus = true;
}

SmartLight::SmartLight(const std::string& deviceID,
                       const std::string& name,
                       int brightnessLevel)
    : SmartDevice(deviceID, name),
      m_brightnessLevel(std::clamp(brightnessLevel, 0, 100))
{
    if (m_brightnessLevel > 0) m_powerStatus = true;
}

// ---- Brightness API --------------------------------------------------------

void SmartLight::setBrightness(int level)
{
    m_brightnessLevel = std::clamp(level, 0, 100);

    // State-machine parity: brightness 0 must match power-off state
    if (m_brightnessLevel == 0)
        turnOff();
    else
        m_powerStatus = true;
}

void SmartLight::turnOff()
{
    m_brightnessLevel = 0;
    m_powerStatus     = false;
}

// ---- Voice feedback --------------------------------------------------------

void SmartLight::announceVoiceFeedback(bool /*powerOn*/)
{
    // Play a click sound instead of TTS to indicate light toggling
    VoiceService::instance().playSound(VoiceService::SoundType::LightClick);
}

// ---- SmartDevice overrides -------------------------------------------------

double SmartLight::ratedWattage() const
{
    // Proportional: 0 W at brightness 0, 10 W at brightness 100
    return (m_brightnessLevel / 100.0) * 10.0;
}

void SmartLight::displayStatus() const
{
    std::cout << std::left
              << std::setw(15) << "[SmartLight]"
              << " ID="         << m_deviceID
              << " Brightness=" << m_brightnessLevel
              << " Wattage="    << ratedWattage() << " W\n";
}
