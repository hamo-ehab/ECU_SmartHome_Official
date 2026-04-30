/*
 * =============================================================================
 * FILE: include/SmartAC.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Mohab Shaaban Mahrous (ID: 692500813)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - Header1 (DAY 1).h    (SmartAC declaration with Mode enum)
 *   - Header1 Day2.h       (identical duplicate — removed)
 *   - Source1 Day 2.cpp    (setTemperature / modeToString / stringToMode impls)
 * =============================================================================
 * SmartAC: strongly-typed Mode enum, temperature clamping [16-30]°C,
 * switch-based string converters, and 3-token CSV persistence.
 * DECLARATION ONLY — implementations live in src/SmartAC.cpp
 * =============================================================================
 */

#ifndef SMARTAC_H
#define SMARTAC_H

#include "SmartDevice.h"
#include <string>

class SmartAC : public SmartDevice
{
public:
    enum class Mode { COOLING, HEATING, FAN_ONLY };

private:
    Mode   m_mode;
    double m_temperature; // Always clamped to [16.0, 30.0] °C

public:
    // ---- Constructors / Destructor -----------------------------------------
    explicit SmartAC(const std::string& deviceID,
                     const std::string& name,
                     Mode   mode        = Mode::FAN_ONLY,
                     double temperature = 24.0);

    // Legacy int-ID constructor (backward compatible call sites)
    explicit SmartAC(int    id,
                     Mode   mode        = Mode::FAN_ONLY,
                     double temperature = 24.0);

    SmartAC(const SmartAC&)            = delete;
    SmartAC& operator=(const SmartAC&) = delete;

    virtual ~SmartAC() override = default;

    // ---- Temperature API ---------------------------------------------------
    void   setTemperature(double temp);
    double getTemperature() const { return m_temperature; }

    // ---- Mode API ----------------------------------------------------------
    void setMode(Mode m)    { m_mode = m; }
    Mode getMode()   const  { return m_mode; }

    // ---- String converters (used by PersistenceManager factory) ------------
    static std::string modeToString(Mode m);
    static Mode        stringToMode(const std::string& s);

    // ---- SmartDevice overrides ---------------------------------------------
    double      ratedWattage()                          const override;
    std::string serialise()                             const override;
    void        deserialise(const std::string& data)          override;
    void        displayStatus()                         const override;
};

#endif // SMARTAC_H
