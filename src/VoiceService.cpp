/*
 * =============================================================================
 * FILE: src/VoiceService.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Yassin Amgad Mohamed (ID: 692500850)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - project day 1.cpp   (VoiceService inline class body)
 *   - SmartDoor3.h        (VoiceService minimal stub body)
 * =============================================================================
 * VoiceService Meyers-Singleton — constructor, destructor, and public API.
 * =============================================================================
 */

#include "../include/VoiceService.h"
#include <iostream>

// ---- Singleton accessor ----------------------------------------------------
// The static local 'inst' is constructed on first call and destroyed
// by the C++ runtime at program exit — this is guaranteed thread-safe (C++11).

VoiceService& VoiceService::instance()
{
    static VoiceService inst;
    return inst;
}

// ---- Private constructor / destructor --------------------------------------

VoiceService::VoiceService()
{
    std::cout << "[VoiceService] Initialized\n";
}

VoiceService::~VoiceService()
{
    std::cout << "[VoiceService] Destroyed cleanly\n";
}

// ---- Public API ------------------------------------------------------------

void VoiceService::speak(const std::string& text)
{
    // Qt build: replace with m_tts->say(QString::fromStdString(text));
    std::cout << "[TTS Engine] Speaking: " << text << "\n";
}

void VoiceService::playSound(SoundType type)
{
    switch (type)
    {
        case SoundType::MechanicalGear:
            std::cout << "[Audio] Playing: Mechanical ratchet\n";
            break;
        case SoundType::LightClick:
            std::cout << "[Audio] Playing: Light click\n";
            break;
        case SoundType::IntroChime:
            std::cout << "[Audio] Playing: Intro chime\n";
            break;
        case SoundType::Alert:
            std::cout << "[Audio] Playing: Security alert\n";
            break;
    }
}
