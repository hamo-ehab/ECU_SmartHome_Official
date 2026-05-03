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
#include "../include/VoiceService.h"
#include <algorithm>  // std::clamp
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

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
    // Day 7 formula: brightness × 0.75 W (0 W when power is off)
    return m_powerStatus ? (m_brightnessLevel * 0.75) : 0.0;
}

void SmartLight::displayStatus() const
{
    // Day 6 format: right-aligned brightness, left-aligned power state
    std::cout << "[SmartLight]"
              << " ID=" << m_deviceID
              << std::setw(12) << std::right << m_brightnessLevel
              << std::setw(12) << std::left  << (m_powerStatus ? " ON" : " OFF")
              << " Wattage=" << ratedWattage() << " W\n";
}

// ---- Persistence (Day 4) ---------------------------------------------------

std::string SmartLight::serialise() const
{
    // Start with base class serialisation, then append brightness as last CSV token
    return SmartDevice::serialise() + "," + std::to_string(m_brightnessLevel);
}

void SmartLight::deserialise(const std::string& data)
{
    // Let base class consume its own tokens first
    SmartDevice::deserialise(data);

    // Extract the brightness token (last comma-separated field)
    std::stringstream ss(data);
    std::string token;
    while (std::getline(ss, token, ',')) { /* advance to last token */ }

    try
    {
        int value = std::stoi(token);

        // Clamp brightness between 0 and 100
        if (value < 0)   value = 0;
        if (value > 100) value = 100;

        m_brightnessLevel = value;
    }
    catch (const std::invalid_argument&) { m_brightnessLevel = 50; } // non-numeric fallback
    catch (const std::out_of_range&)     { m_brightnessLevel = 50; } // overflow fallback
}

// ---- Simulation tick (Day 5) -----------------------------------------------

void SmartLight::update()
{
    ++m_tickCounter;

#ifdef _DEBUG
    // Debug mode: print this-pointer address delta between consecutive ticks
    static void* lastAddress = nullptr;
    void* currentAddress = static_cast<void*>(this);

    if (lastAddress != nullptr)
    {
        std::ptrdiff_t delta =
            reinterpret_cast<char*>(currentAddress) -
            reinterpret_cast<char*>(lastAddress);
        std::cout << "[SmartLight Debug] Tick: " << m_tickCounter
                  << " | Address Delta: " << delta << "\n";
    }
    else
    {
        std::cout << "[SmartLight Debug] Tick: " << m_tickCounter
                  << " | First Address: " << currentAddress << "\n";
    }
    lastAddress = currentAddress;
#endif
}
