/*
 * =============================================================================
 * FILE: include/WavUtils.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Yassin Amgad Mohamed (ID: 692500850)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - day 2.cpp  (makePcmWav — basic sine-wave WAV builder)
 *   - day 3.cpp  (writeWavHeader / makeLightClick / makeMechanicalGear /
 *                 makeIntroChime)
 * =============================================================================
 * PCM WAV buffer generation utilities used by VoiceService to produce
 * in-memory audio clips for the door lock, light click, and intro chime.
 * NOTE: Defines header-only namespace functions.
 * =============================================================================
 */

#ifndef WAVUTILS_H
#define WAVUTILS_H

#ifndef NO_QT
#  include <QByteArray>
#  include <cmath>
#  include <cstdint>
#  include <cstdlib>  // rand() for noise in MechanicalGear

namespace WavUtils
{
    // ---- Byte-order helpers (Little Endian) --------------------------------
    inline void writeLE16(QByteArray& buf, uint16_t value)
    {
        buf.append(char(value & 0xFF));
        buf.append(char((value >> 8) & 0xFF));
    }

    inline void writeLE32(QByteArray& buf, uint32_t value)
    {
        buf.append(char(value & 0xFF));
        buf.append(char((value >>  8) & 0xFF));
        buf.append(char((value >> 16) & 0xFF));
        buf.append(char((value >> 24) & 0xFF));
    }

    // ---- WAV RIFF header writer -------------------------------------------
    // Writes the 44-byte PCM WAV header for mono 16-bit audio.
    inline void writeWavHeader(QByteArray& buffer, int dataSize, int sampleRate = 44100)
    {
        const int numChannels  = 1;
        const int bitsPerSample = 16;

        buffer.append("RIFF");
        writeLE32(buffer, 36 + dataSize);
        buffer.append("WAVE");

        buffer.append("fmt ");
        writeLE32(buffer, 16);                              // Sub-chunk size
        writeLE16(buffer, 1);                               // PCM format
        writeLE16(buffer, numChannels);
        writeLE32(buffer, sampleRate);
        writeLE32(buffer, sampleRate * numChannels * bitsPerSample / 8);
        writeLE16(buffer, numChannels * bitsPerSample / 8); // Block align
        writeLE16(buffer, bitsPerSample);

        buffer.append("data");
        writeLE32(buffer, dataSize);
    }

    // ---- Sound clip generators --------------------------------------------

    // Generic sine-wave tone (used by early Day 2 code)
    inline QByteArray makePcmWav(int freq, int durationMs, float amplitude)
    {
        constexpr double PI = 3.141592653589793;
        const int sampleRate    = 44100;
        const int numChannels   = 1;
        const int bitsPerSample = 16;

        int numSamples = (sampleRate * durationMs) / 1000;
        int dataSize   = numSamples * numChannels * (bitsPerSample / 8);

        QByteArray buffer;
        writeWavHeader(buffer, dataSize, sampleRate);

        for (int i = 0; i < numSamples; ++i)
        {
            double t      = static_cast<double>(i) / sampleRate;
            double sample = amplitude * std::sin(2.0 * PI * freq * t);
            writeLE16(buffer, static_cast<uint16_t>(static_cast<int16_t>(sample * 32767)));
        }
        return buffer;
    }

    // (1) Light switch click — 10 ms impulse spike
    inline QByteArray makeLightClick()
    {
        const int sampleRate = 44100;
        const int durationMs = 10;
        const int numSamples = sampleRate * durationMs / 1000;

        QByteArray buffer;
        int dataSize = numSamples * 2;
        writeWavHeader(buffer, dataSize, sampleRate);

        for (int i = 0; i < numSamples; ++i)
        {
            int16_t sample = (i < 60) ? 30000 : 0; // Short sharp spike
            writeLE16(buffer, static_cast<uint16_t>(sample));
        }
        return buffer;
    }

    // (2) Door lock mechanical ratchet — 250 ms burst noise
    inline QByteArray makeMechanicalGear()
    {
        const int sampleRate = 44100;
        const int durationMs = 250;
        const int numSamples = sampleRate * durationMs / 1000;

        QByteArray buffer;
        int dataSize = numSamples * 2;
        writeWavHeader(buffer, dataSize, sampleRate);

        const int burstLength   = 180;
        const int silenceLength = 300;

        for (int i = 0; i < numSamples; ++i)
        {
            int cycle   = burstLength + silenceLength;
            bool inBurst = (i % cycle) < burstLength;

            int16_t sample = 0;
            if (inBurst)
            {
                float noise = (rand() / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
                sample = static_cast<int16_t>(noise * 20000);
            }
            writeLE16(buffer, static_cast<uint16_t>(sample));
        }
        return buffer;
    }

    // (3) Boot intro chime — 400 ms C5/E5/G5 chord
    inline QByteArray makeIntroChime()
    {
        constexpr double PI      = 3.141592653589793;
        const int sampleRate     = 44100;
        const int durationMs     = 400;
        const int numSamples     = sampleRate * durationMs / 1000;

        QByteArray buffer;
        int dataSize = numSamples * 2;
        writeWavHeader(buffer, dataSize, sampleRate);

        for (int i = 0; i < numSamples; ++i)
        {
            double t = static_cast<double>(i) / sampleRate;
            double sample =
                0.5 * std::sin(2.0 * PI * 523.25 * t) +  // C5
                0.3 * std::sin(2.0 * PI * 659.25 * t) +  // E5
                0.2 * std::sin(2.0 * PI * 783.99 * t);   // G5

            int16_t pcm = static_cast<int16_t>(sample * 30000);
            writeLE16(buffer, static_cast<uint16_t>(pcm));
        }
        return buffer;
    }

} // namespace WavUtils

#endif // NO_QT
#endif // WAVUTILS_H
