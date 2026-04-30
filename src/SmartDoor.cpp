/*
 * =============================================================================
 * FILE: src/SmartDoor.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Omar Hosam Eskndr (ID: 692500809)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - SmartDoor.cpp                           (Day 1: lock/unlock impls)
 *   - SmartDoor2.cpp                          (Day 2: logEvent impl)
 *   - SmartDoor3.cpp                          (Day 3: VoiceService sounds)
 *   - SmartHome4.cpp                          (Day 4: serialise impl)
 * =============================================================================
 * SmartDoor method implementations:
 *   - Access-log ring buffer (modulo wrap, std::chrono timestamps — no Qt)
 *   - Fail-Secure locking logic with VoiceService audio feedback
 *   - Full CSV serialise / deserialise with firewall delimiter check
 * =============================================================================
 */

#include "../include/SmartDoor.h"
#include "../include/VoiceService.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

// ---- Private: ring-buffer event logger ------------------------------------
// Stamps the event with HH:MM:SS and inserts it into the ring buffer.
// After MAX_LOG entries, the oldest is overwritten via modulo index arithmetic.

void SmartDoor::logEvent(const std::string& event)
{
    // Build timestamp using <chrono> + <ctime> (no Qt dependency)
    auto now      = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[10];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));

    std::string entry = std::string(buf) + " " + event;

    // Ring-buffer insertion: O(1), no reallocation
    if (static_cast<int>(m_accessLog.size()) < MAX_LOG)
        m_accessLog.push_back(entry);
    else
        m_accessLog[m_logIndex] = entry;

    m_logIndex = (m_logIndex + 1) % MAX_LOG;
}

// ---- Constructor (legacy bool-power form) ----------------------------------

SmartDoor::SmartDoor(bool initialPower)
    : SmartDevice(initialPower), m_isLocked(true), m_logIndex(0)
{
    m_accessLog.reserve(MAX_LOG); // Single heap allocation up-front
    logEvent("INIT");
    std::cout << "[Init] SmartDoor created. Fail-Secure mode active (Locked = true).\n";
    VoiceService::instance().speak(
        m_deviceName.empty() ? "Smart Door Initialized" : m_deviceName + " Initialized");
}

// ---- Constructor (full identity form) --------------------------------------

SmartDoor::SmartDoor(const std::string& deviceID,
                     const std::string& name,
                     bool initialPower)
    : SmartDevice(deviceID, name), m_isLocked(true), m_logIndex(0)
{
    m_powerStatus = initialPower;
    m_accessLog.reserve(MAX_LOG);
    logEvent("INIT");
    std::cout << "[Init] SmartDoor '" << m_deviceName
              << "' created. Fail-Secure (Locked=true).\n";
}

// ---- Lock / Unlock / Toggle ------------------------------------------------

void SmartDoor::lock()
{
    if (m_isLocked) return;       // Guard: already locked — no-op
    turnOn();                     // Ensure power is on before engaging bolt
    m_isLocked = true;
    logEvent("LOCKED");
    std::cout << "[Action] Door is now LOCKED.\n";
    VoiceService::instance().playSound(VoiceService::SoundType::MechanicalGear);
    VoiceService::instance().speak(m_deviceName + " is now Locked");
}

void SmartDoor::unlock()
{
    if (!m_isLocked) return;      // Guard: already unlocked — no-op
    m_isLocked = false;
    logEvent("UNLOCKED");
    std::cout << "[Action] Door is now UNLOCKED.\n";
    VoiceService::instance().playSound(VoiceService::SoundType::MechanicalGear);
    VoiceService::instance().speak(m_deviceName + " is now Unlocked");
}

void SmartDoor::toggleLock()
{
    if (m_isLocked) unlock();
    else             lock();
}

// ---- Log display -----------------------------------------------------------

void SmartDoor::showLogs() const
{
    std::cout << "\n--- Access Logs (Last " << MAX_LOG << " Events) ---\n";
    for (const auto& entry : m_accessLog)
        std::cout << entry << "\n";
}

// ---- Persistence -----------------------------------------------------------

std::string SmartDoor::serialise() const
{
    // Format: "<base_power>,<lock_state>"  e.g. "1,1"
    std::ostringstream oss;
    oss << SmartDevice::serialise() << "," << (m_isLocked ? "1" : "0");
    return oss.str();
}

void SmartDoor::deserialise(const std::string& data)
{
    // Firewall: a comma delimiter is mandatory
    size_t delimPos = data.find(',');
    if (delimPos == std::string::npos)
    {
        std::cerr << "[Error] SmartDoor::deserialise — malformed data "
                     "(no delimiter). Aborted.\n";
        return;
    }
    SmartDevice::deserialise(data.substr(0, delimPos));
    const std::string doorData = data.substr(delimPos + 1);
    if (!doorData.empty())
        m_isLocked = (doorData[0] == '1');
}

// ---- SmartDevice overrides -------------------------------------------------

double SmartDoor::ratedWattage() const
{
    return m_powerStatus ? 0.5 : 0.0; // 0.5 W standby; 0 W off
}

void SmartDoor::update()
{
    // Integrated from Task 5
    if (!getPowerStatus() && !m_isLocked)
    {
        std::cout << "[EMERGENCY HEARTBEAT] Power loss detected! Auto-securing door...\n";
        lock();
    }
}

void SmartDoor::displayStatus() const
{
    std::cout << "--- Smart Door Status ---\n";
    std::cout << "Power:  " << (getPowerStatus() ? "ON" : "OFF") << "\n";
    std::cout << "Status: " << (m_isLocked ? "LOCKED" : "UNLOCKED") << "\n";
    std::cout << "Access Log:\n";

    for (const auto& event : m_accessLog)
    {
        std::cout << "    -> " << event << "\n";
    }
    std::cout << "-------------------------\n";
}
