/*
 * =============================================================================
 * FILE: src/SmartAC.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Mohab Shaaban Mahrous (ID: 692500813)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - Source1 Day 2.cpp    (setTemperature / modeToString impls)
 * =============================================================================
 * SmartAC method implementations:
 *   - Two constructors (full identity / legacy int-ID)
 *   - setTemperature with std::clamp enforcement
 *   - modeToString / stringToMode for PersistenceManager factory
 *   - serialise / deserialise with 3-token CSV and try-catch firewall
 *   - ratedWattage: mode-dependent model (COOLING=1500W, HEATING=1200W, FAN=100W)
 * =============================================================================
 */

#include "../include/SmartAC.h"
#include <algorithm>  // std::clamp
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>

// ---- Constructors ----------------------------------------------------------

SmartAC::SmartAC(const std::string& deviceID,
                 const std::string& name,
                 Mode   mode,
                 double temperature)
    : SmartDevice(deviceID, name),
      m_mode(mode),
      m_temperature(std::clamp(temperature, 16.0, 30.0))
{}

SmartAC::SmartAC(int id, Mode mode, double temperature)
    : SmartDevice(std::to_string(id), "AC-" + std::to_string(id)),
      m_mode(mode),
      m_temperature(std::clamp(temperature, 16.0, 30.0))
{}

// ---- Temperature API -------------------------------------------------------

void SmartAC::setTemperature(double temp)
{
    m_temperature = std::clamp(temp, 16.0, 30.0);
}

// ---- String converters (used by PersistenceManager factory) ----------------

std::string SmartAC::modeToString(Mode m)
{
    switch (m)
    {
        case Mode::COOLING:  return "COOLING";
        case Mode::HEATING:  return "HEATING";
        case Mode::FAN_ONLY: return "FAN_ONLY";
    }
    return "FAN_ONLY";
}

SmartAC::Mode SmartAC::stringToMode(const std::string& s)
{
    if (s == "COOLING") return Mode::COOLING;
    if (s == "HEATING") return Mode::HEATING;
    return Mode::FAN_ONLY;
}

// ---- Persistence -----------------------------------------------------------

std::string SmartAC::serialise() const
{
    // Format: "<base_power>,<mode>,<temperature>"   e.g. "1,COOLING,22.5"
    std::ostringstream oss;
    oss << SmartDevice::serialise()
        << "," << modeToString(m_mode)
        << "," << m_temperature;
    return oss.str();
}

void SmartAC::deserialise(const std::string& data)
{
    try
    {
        std::istringstream iss(data);
        std::string token;

        // Token 0: base power
        if (!std::getline(iss, token, ','))
            throw std::invalid_argument("Missing power token");
        SmartDevice::deserialise(token);

        // Token 1: mode string
        if (!std::getline(iss, token, ','))
            throw std::invalid_argument("Missing mode token");
        m_mode = stringToMode(token);

        // Token 2: temperature double
        if (!std::getline(iss, token, ','))
            throw std::invalid_argument("Missing temperature token");
        m_temperature = std::clamp(std::stod(token), 16.0, 30.0);
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "[SmartAC] deserialise out_of_range: " << e.what() << "\n";
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "[SmartAC] deserialise invalid_argument: " << e.what() << "\n";
    }
}

// ---- SmartDevice overrides -------------------------------------------------

double SmartAC::ratedWattage() const
{
    if (!m_powerStatus) return 0.0;
    switch (m_mode)
    {
        case Mode::COOLING:  return 1500.0;
        case Mode::HEATING:  return 1200.0;
        case Mode::FAN_ONLY: return 100.0;
    }
    return 100.0;
}

void SmartAC::displayStatus() const
{
    std::cout << std::left
              << std::setw(15) << "[SmartAC]"
              << " ID="   << m_deviceID
              << " Mode=" << modeToString(m_mode)
              << " Temp=" << m_temperature << " C"
              << " W="    << ratedWattage() << " W\n";
}
