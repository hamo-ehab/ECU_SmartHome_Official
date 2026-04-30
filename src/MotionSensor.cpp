/*
 * =============================================================================
 * FILE: src/MotionSensor.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Bassel Mohamed Fathy (ID: 692501345)
 * REVIEWED & OPTIMIZED BY: Ahmed Sherif
 * MERGED FROM ORIGINAL FILES:
 *   - MotionSensor.h  (extracted simulate / addListener / notifyListeners impls)
 * =============================================================================
 * MotionSensor and Sensor base implementations:
 *   - Sensor::addListener / notifyListeners (prunes expired weak_ptr entries)
 *   - MotionSensor::MotionSensor — seeds RNG ONCE in constructor (never in simulate)
 *   - MotionSensor::simulate — Bernoulli draw; fires notifyListeners only on
 *     state change (edge-detection pattern)
 * =============================================================================
 */

#include "../include/MotionSensor.h"
#include <iostream>

// ===========================================================================
// Sensor base class
// ===========================================================================

Sensor::Sensor()
    : m_lastValue(0.0)
{}

void Sensor::addListener(const std::shared_ptr<ISensorListener>& listener)
{
    // Store as weak_ptr — the Sensor does NOT extend the listener's lifetime.
    // This prevents circular shared_ptr chains that block heap reclamation.
    m_listeners.push_back(listener);
}

void Sensor::notifyListeners()
{
    // Iterate and prune in a single pass.
    // Expired weak_ptrs (listeners that were deleted elsewhere) are removed
    // automatically here — preventing UAF (use-after-free) crashes.
    auto it = m_listeners.begin();
    while (it != m_listeners.end())
    {
        if (auto sp = it->lock())
        {
            sp->onMotionChanged(m_lastValue);
            ++it;
        }
        else
        {
            it = m_listeners.erase(it); // Prune dead observer
        }
    }
}

// ===========================================================================
// MotionSensor
// ===========================================================================

MotionSensor::MotionSensor()
    : m_rng(std::random_device{}()), // Seed ONCE — constructing inside simulate()
      m_dist(0.05),                  // would destroy performance on every tick
      m_motionState(false)
{}

void MotionSensor::simulate()
{
    bool newState = m_dist(m_rng);

    // Fire notifications ONLY when the state actually changes (edge detection).
    // Suppressing unchanged states prevents flooding the Event Bus with
    // redundant payloads when no physical motion transition has occurred.
    if (newState != m_motionState)
    {
        m_motionState = newState;
        m_lastValue   = m_motionState ? 1.0 : 0.0;
        notifyListeners();
    }
}
