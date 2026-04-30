/*
 * =============================================================================
 * FILE: include/SmartDevice.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Eng. Zain (Lead Architect)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - SmartDeviceABC-PowerControl&TimerLogic.cpp     (original base class body)
 *   - SmartDeviceABC-PowerControl&TimerLogic (1).cpp (duplicate iteration)
 *   - SmartDoor2.h  (re-declared SmartDevice inline — removed)
 *   - SmartDoor3.h  (re-declared SmartDevice inline — removed)
 *   - SmartDoor4.h  (added serialise/deserialise/virtual destructor — absorbed)
 *   - Dynamic Logging.cpp  (SmartDevice re-declaration — removed)
 *   - Task 2 .cpp          (SmartDevice re-declaration — removed)
 * =============================================================================
 * Abstract polymorphic base for all smart-home devices.
 * Provides power API, identity, persistence interface, and simulation tick.
 * DECLARATION ONLY — implementations live in src/SmartDevice.cpp
 * =============================================================================
 */

#ifndef SMARTDEVICE_H
#define SMARTDEVICE_H

#include <string>
#include <iostream>
#include <sstream>

class SmartDevice
{
protected:
    std::string m_deviceID;
    std::string m_deviceName;
    bool        m_powerStatus;

public:
    // ---- Construction / Destruction ----------------------------------------
    explicit SmartDevice(const std::string& deviceID, const std::string& name);
    explicit SmartDevice(bool power);
    virtual ~SmartDevice() = default;

    SmartDevice(const SmartDevice&)            = delete;
    SmartDevice& operator=(const SmartDevice&) = delete;

    // ---- Power API ---------------------------------------------------------
    virtual void turnOn();
    virtual void turnOff();

    bool               getPowerStatus() const { return m_powerStatus; }
    const std::string& getDeviceID()    const { return m_deviceID;    }
    const std::string& getDeviceName()  const { return m_deviceName;  }

    // ---- Pure-virtual interface ---------------------------------------------
    virtual double ratedWattage() const = 0;

    // ---- Persistence -------------------------------------------------------
    virtual std::string serialise()                        const;
    virtual void        deserialise(const std::string& data);

    // ---- Optional overridable hooks ----------------------------------------
    virtual void update()               {}
    virtual void displayStatus()  const;
};

#endif // SMARTDEVICE_H
