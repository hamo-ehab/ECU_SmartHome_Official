/*
 * =============================================================================
 * FILE: include/SmartDoor.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Omar Hosam Eskndr (ID: 692500809)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - SmartDoor.h                             (Day 1: basic lock stub)
 *   - SmartDoor2.h / SmartDoor2.cpp           (Day 2: access-log ring buffer)
 *   - SmartDoor3.h / SmartDoor3.cpp           (Day 3: VoiceService integration)
 *   - SmartDoor4.h / SmartHome4.cpp           (Day 4: serialise/deserialise)
 *   - SmartDoor.cpp                           (lock/unlock/toggleLock impls)
 *   - Dynamic Logging.cpp                     (duplicate SmartDoor with log)
 *   - Task 2 .cpp                             (duplicate SmartDoor with log)
 *   - SmartDeviceABC-PowerControl&TimerLogic.cpp (inline SmartDoor + main)
 *   - SmartDeviceABC-PowerControl&TimerLogic (1).cpp (duplicate)
 * =============================================================================
 * SmartDoor: fail-secure locking, O(1) ring-buffer access log,
 * VoiceService audio feedback, and CSV serialise/deserialise.
 * DECLARATION ONLY — implementations live in src/SmartDoor.cpp
 * =============================================================================
 */

#ifndef SMARTDOOR_H
#define SMARTDOOR_H

#include "SmartDevice.h"
#include <vector>
#include <string>

class SmartDoor : public SmartDevice
{
private:
    bool m_isLocked;

    static constexpr int MAX_LOG = 10;
    std::vector<std::string> m_accessLog;
    int m_logIndex;

    void logEvent(const std::string& event);

public:
    explicit SmartDoor(bool initialPower);
    explicit SmartDoor(const std::string& deviceID,
                       const std::string& name,
                       bool initialPower = true);

    SmartDoor(const SmartDoor&)            = delete;
    SmartDoor& operator=(const SmartDoor&) = delete;

    virtual ~SmartDoor() override = default;

    void lock();
    void unlock();
    void toggleLock();

    void update() override; // Integrated from Task 5

    bool getLockStatus() const { return m_isLocked; }
    void showLogs() const;

    std::string serialise() const override;
    void deserialise(const std::string& data) override;

    double ratedWattage() const override;
    void   displayStatus() const override;
};

#endif // SMARTDOOR_H
