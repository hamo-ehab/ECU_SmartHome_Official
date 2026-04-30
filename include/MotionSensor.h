/*
 * =============================================================================
 * FILE: include/MotionSensor.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Bassel Mohamed Fathy (ID: 692501345)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - MotionSensor.h  (malformed file containing .h + .cpp + main() in one file;
 *                      extracted into ISensorListener, Sensor, MotionSensor)
 * =============================================================================
 * ISensorListener interface, abstract Sensor base, and MotionSensor concrete:
 * Bernoulli 5%-per-tick probability, weak_ptr listener fan-out, edge detection.
 * DECLARATION ONLY — implementations live in src/MotionSensor.cpp
 * =============================================================================
 */

#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H

#include <random>
#include <memory>
#include <vector>

// ---------------------------------------------------------------------------
// ISensorListener — Observer interface
// ---------------------------------------------------------------------------
class ISensorListener
{
public:
    virtual ~ISensorListener() = default;
    virtual void onMotionChanged(double value) = 0;
};

// ---------------------------------------------------------------------------
// Sensor — abstract base for all sensors
// ---------------------------------------------------------------------------
class Sensor
{
protected:
    double m_lastValue;
    std::vector<std::weak_ptr<ISensorListener>> m_listeners;

public:
    Sensor();
    virtual ~Sensor() = default;

    // Add an observer (stored as weak_ptr — avoids ownership cycles)
    void   addListener(const std::shared_ptr<ISensorListener>& listener);
    double getLastValue() const { return m_lastValue; }

    virtual void simulate() = 0;

protected:
    // Notifies alive observers; prunes expired weak_ptr entries
    void notifyListeners();
};

// ---------------------------------------------------------------------------
// MotionSensor — Bernoulli-random motion (5% chance per tick)
// ---------------------------------------------------------------------------
class MotionSensor : public Sensor
{
private:
    std::mt19937                m_rng;   // Seeded ONCE in constructor — never inside simulate()
    std::bernoulli_distribution m_dist;  // P(motion) = 0.05 per tick
    bool                        m_motionState;

public:
    MotionSensor();

    void simulate()       override;
    bool getMotionState() const { return m_motionState; }
};

#endif // MOTIONSENSOR_H
