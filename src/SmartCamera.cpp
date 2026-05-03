/*
 * =============================================================================
 * FILE: src/SmartCamera.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Hamza Ali Bakry (ID: 692500812)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - SmartCameracpp.cpp  (startRec / stopRec / toggleRecording impls)
 *   - Source1.cpp         (raiseMotionAlert impl)
 * =============================================================================
 * SmartCamera method implementations:
 *   - startRec / stopRec / toggleRecording with VoiceService alert
 *   - raiseMotionAlert: FALSE→TRUE edge detection → auto startRec
 *   - update(): video ring-buffer modulo advance (O(1), size_t safe)
 *   - serialise / deserialise: 3-token CSV with try-catch firewall
 *   - displayStatus: tabular iomanip output with object address (debug)
 * =============================================================================
 */

#include "../include/SmartCamera.h"
#include "../include/VoiceService.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>

// ---- Constructor -----------------------------------------------------------

SmartCamera::SmartCamera(const std::string& deviceID, const std::string& name)
    : SmartDevice(deviceID, name),
      m_recording(false),
      m_motionDetectedAlert(false),
      m_bufferIndex(0),
      m_maxFrames(0)
{}

// ---- Recording API ---------------------------------------------------------

void SmartCamera::startRec()
{
    m_recording = true;
    VoiceService::instance().playSound(VoiceService::SoundType::Alert);
    VoiceService::instance().speak("Security Camera Recording Initiated");
}

void SmartCamera::stopRec()
{
    m_recording   = false;
    m_bufferIndex = 0;
    m_maxFrames   = 0;
}

void SmartCamera::toggleRecording()
{
    if (m_recording) stopRec();
    else              startRec();
}

// ---- Observer hook — motion-alert state machine ----------------------------
// Uses std::ostringstream to compose the log string without string-concatenation
// operator+ heap fragmentation, which matters in high-frequency alert paths.

void SmartCamera::raiseMotionAlert(bool detected)
{
    // Rising-edge: FALSE → TRUE transition only triggers recording
    if (!m_motionDetectedAlert && detected)
    {
        startRec();

        std::ostringstream oss;
        oss << "[" << m_deviceID << "] Motion Transition: TRUE";
        std::cout << oss.str() << "\n";
    }
    m_motionDetectedAlert = detected;
}

// ---- Simulation tick — video ring-buffer advance ---------------------------
// Called once per engine tick by the Home loop.
// Modulo arithmetic keeps m_bufferIndex within [0, MAX_BUFFER_FRAMES).
// size_t index type guarantees 64-bit architecture compatibility.

void SmartCamera::update()
{
    if (!m_recording) return;

    // O(1) ring-buffer advance — no heap reallocation ever occurs
    m_bufferIndex = (m_bufferIndex + 1) % MAX_BUFFER_FRAMES;
    ++m_maxFrames;
}

// ---- Persistence -----------------------------------------------------------

std::string SmartCamera::serialise() const
{
    // Format: "<base>,<recording>,<motionAlert>"   e.g. "1,0,0"
    std::ostringstream oss;
    oss << SmartDevice::serialise()
        << "," << (m_recording          ? "1" : "0")
        << "," << (m_motionDetectedAlert ? "1" : "0");
    return oss.str();
}

void SmartCamera::deserialise(const std::string& data)
{
    // All parsing is wrapped in try-catch: malformed or truncated data must
    // never crash the application — just log and return.
    try
    {
        std::istringstream iss(data);
        std::string token;

        // Token 0: base power
        if (!std::getline(iss, token, ','))
            throw std::invalid_argument("Missing power token");
        SmartDevice::deserialise(token);

        // Token 1: recording flag
        if (!std::getline(iss, token, ','))
            throw std::invalid_argument("Missing recording token");
        m_recording = (token == "1");

        // Token 2: motionDetectedAlert flag
        if (!std::getline(iss, token, ','))
            throw std::invalid_argument("Missing motionAlert token");
        m_motionDetectedAlert = (token == "1");
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "[SmartCamera] deserialise out_of_range: " << e.what() << "\n";
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "[SmartCamera] deserialise invalid_argument: " << e.what() << "\n";
    }
}

// ---- SmartDevice overrides -------------------------------------------------

double SmartCamera::ratedWattage() const
{
    return m_recording ? 10.0 : 5.0; // 5 W idle, 10 W active recording
}

// Tabular status dump with object memory address (debug: trace object lifetime)
void SmartCamera::displayStatus() const
{
    std::cout << std::left
              << std::setw(15) << "[SmartCamera]"
              << " ID="        << m_deviceID
              << " Addr="      << static_cast<const void*>(this)
              << " Recording=" << m_recording
              << " Motion="    << m_motionDetectedAlert
              << " Buf="       << m_bufferIndex
              << "/"           << MAX_BUFFER_FRAMES << "\n";
}
