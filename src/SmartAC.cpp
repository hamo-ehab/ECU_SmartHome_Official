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
#include <vector>

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
    std::stringstream ss;
    // نضع اسم الكلاس في البداية لتمكين الـ PersistenceManager من التعرف عليه
    ss << "SmartAC" << ",";
    ss << m_deviceID << ",";
    ss << (m_powerStatus ? "1" : "0") << ",";
    ss << modeToString(m_mode) << ",";
    
    // استخدام fixed و setprecision لضمان رقم عشري واحد فقط (مثل 24.5)
    ss << std::fixed << std::setprecision(1) << m_temperature;
    
    return ss.str();
}

void SmartAC::deserialise(const std::string& data)
{
    std::stringstream ss(data);
    std::string token;
    std::vector<std::string> tokens;

    // تقطيع السطر بناءً على الفاصلة
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    // التأكد من أن السطر يحتوي على البيانات المطلوبة
    // وندعم كلا الصيغتين: القديمة (3) والجديدة (5) التي تبدأ بـ SmartAC
    bool isNewFormat = (!tokens.empty() && tokens[0] == "SmartAC");
    
    if (isNewFormat && tokens.size() < 5) {
        std::cerr << "Error: Invalid CSV format for SmartAC\n";
        return;
    } else if (!isNewFormat && tokens.size() < 3) {
        return;
    }

    try {
        if (isNewFormat) {
            m_deviceID = tokens[1];
            m_powerStatus = (tokens[2] == "1");
            m_mode = stringToMode(tokens[3]);
            m_temperature = std::stod(tokens[4]);
        } else {
            m_powerStatus = (tokens[0] == "1");
            m_mode = stringToMode(tokens[1]);
            m_temperature = std::stod(tokens[2]);
        }
        
        m_temperature = std::clamp(m_temperature, 16.0, 30.0);
        
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid numeric data in CSV -> " << e.what() << "\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Temperature value out of range -> " << e.what() << "\n";
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
            