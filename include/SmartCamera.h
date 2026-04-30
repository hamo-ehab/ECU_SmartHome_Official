/*
 * =============================================================================
 * FILE: include/SmartCamera.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Hamza Ali Bakry (ID: 692500812)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - SmartCamera.h       (declaration with m_recording, m_motionDetectedAlert)
 *   - SmartCameracpp.cpp  (startRec / stopRec / toggleRecording / ratedWattage)
 *   - Source1.cpp         (raiseMotionAlert edge detection + inline test main)
 * =============================================================================
 * SmartCamera: motion-alert FALSE->TRUE edge detection, video ring-buffer,
 * CSV persistence, VoiceService Alert sound, const-correct + explicit ctor.
 * DECLARATION ONLY — implementations live in src/SmartCamera.cpp
 * =============================================================================
 */

#ifndef SMARTCAMERA_H
#define SMARTCAMERA_H

#include "SmartDevice.h"
#include "VoiceService.h"
#include <string>

class SmartCamera : public SmartDevice
{
private:
    bool   m_recording;
    bool   m_motionDetectedAlert;

    // Video ring-buffer (Day 3): size_t for 64-bit index safety
    static constexpr size_t MAX_BUFFER_FRAMES = 1024;
    size_t m_bufferIndex;
    size_t m_maxFrames;

public:
    // ---- Constructor / Destructor ------------------------------------------
    // 'explicit' blocks silent single-argument implicit conversions
    explicit SmartCamera(const std::string& deviceID, const std::string& name);

    SmartCamera(const SmartCamera&)            = delete;
    SmartCamera& operator=(const SmartCamera&) = delete;

    virtual ~SmartCamera() override = default;

    // ---- Recording API -----------------------------------------------------
    void startRec();
    void stopRec();
    void toggleRecording();
    bool isRecording() const { return m_recording; }

    // ---- Observer hook — motion-alert state machine (Day 2) ---------------
    void raiseMotionAlert(bool detected);

    // ---- SmartDevice overrides ---------------------------------------------
    void        update()                             override;
    double      ratedWattage()                 const override;
    std::string serialise()                    const override;
    void        deserialise(const std::string& data) override;
    void        displayStatus()                const override;
};

#endif // SMARTCAMERA_H
