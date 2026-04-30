/*
 * =============================================================================
 * FILE: src/SmartDevice.cpp
 * PROJECT: ECU Smart Home System
 * LEAD ARCHITECT: Eng. Zain
 * =============================================================================
 * Implementations of non-inline SmartDevice methods.
 * =============================================================================
 */

#include "../include/SmartDevice.h"
#include <iostream>

// ---- Constructors ----------------------------------------------------------

SmartDevice::SmartDevice(const std::string& deviceID, const std::string& name)
    : m_deviceID(deviceID), m_deviceName(name), m_powerStatus(false)
{}

SmartDevice::SmartDevice(bool power)
    : m_deviceID(""), m_deviceName(""), m_powerStatus(power)
{}

// ---- Power API -------------------------------------------------------------

void SmartDevice::turnOn()
{
    m_powerStatus = true;
    std::cout << "[SmartDevice] Power ON: " << m_deviceName << "\n";
}

void SmartDevice::turnOff()
{
    m_powerStatus = false;
    std::cout << "[SmartDevice] Power OFF: " << m_deviceName << "\n";
}

// ---- Persistence -----------------------------------------------------------

std::string SmartDevice::serialise() const
{
    // Base format: "1" (on) or "0" (off)
    return m_powerStatus ? "1" : "0";
}

void SmartDevice::deserialise(const std::string& data)
{
    if (!data.empty())
        m_powerStatus = (data[0] == '1');
}

// ---- Debug status dump -----------------------------------------------------

void SmartDevice::displayStatus() const
{
    std::cout << "[Device] ID="    << m_deviceID
              << " Name="          << m_deviceName
              << " Power="         << m_powerStatus << "\n";
}
