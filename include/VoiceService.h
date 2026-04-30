/*
 * =============================================================================
 * FILE: include/VoiceService.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Yassin Amgad Mohamed (ID: 692500850)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - project day 1.cpp   (VoiceService class defined inline with Qt TTS)
 *   - SmartDoor3.h        (minimal VoiceService stub with MechanicalGear only)
 * =============================================================================
 * Meyers-Singleton audio/TTS service. Provides speak() and playSound().
 * NEVER store a reference as a class member — always call instance() locally.
 * DECLARATION ONLY — implementations live in src/VoiceService.cpp
 * =============================================================================
 */

#ifndef VOICESERVICE_H
#define VOICESERVICE_H

#include <string>
#include <iostream>

class VoiceService
{
public:
    // ---- Sound catalogue ---------------------------------------------------
    enum class SoundType
    {
        MechanicalGear, // Door lock / unlock ratchet
        LightClick,     // Light switch click
        IntroChime,     // Boot chime
        Alert           // Security-camera alert
    };

    // ---- Meyers-Singleton accessor -----------------------------------------
    // Static-local initialisation is lazy and thread-safe (C++11 guarantee).
    static VoiceService& instance();

    // ---- Public API --------------------------------------------------------
    void speak(const std::string& text);
    void playSound(SoundType type);

private:
    VoiceService();
    ~VoiceService();

    VoiceService(const VoiceService&)            = delete;
    VoiceService& operator=(const VoiceService&) = delete;
};

#endif // VOICESERVICE_H
