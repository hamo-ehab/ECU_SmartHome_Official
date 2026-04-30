/*
 * =============================================================================
 * FILE: src/AutomationRuleEngine.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Mohamed Sameh Ghonem (ID: 692500817)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - AutomationRuleEngine.cpp       (onSensorUpdate / evaluateCondition impls)
 *   - AutomationRuleEngine (1)(1).cpp (executeAction weak_ptr dispatch impls)
 * =============================================================================
 * AutomationRuleEngine implementations:
 *   - evaluateCondition: GT / LT / EQ with epsilon float comparison
 *   - executeAction: weak_ptr lock, polymorphic vtable dispatch (no dynamic_cast)
 *   - onSensorUpdate: hot-path rule iteration
 *   - addRule / registerDevice / clearRules / ruleCount
 * =============================================================================
 */

#include "../include/AutomationRuleEngine.h"
#include <iostream>
#include <cmath>   // std::abs for epsilon EQ check

// ---- Private: condition evaluation ----------------------------------------
// EQ uses an epsilon check because raw double == double comparisons are
// unreliable due to IEEE 754 binary floating-point representation.

bool AutomationRuleEngine::evaluateCondition(const AutomationRule& rule,
                                              double value) const
{
    if (rule.condition == RuleCondition::GT) return value > rule.threshold;
    if (rule.condition == RuleCondition::LT) return value < rule.threshold;
    if (rule.condition == RuleCondition::EQ)
    {
        constexpr double epsilon = 0.000001;
        return std::abs(value - rule.threshold) < epsilon;
    }
    return false;
}

// ---- Private: action dispatch ----------------------------------------------
// Step 1: O(1) hash-map lookup by device string ID.
// Step 2: Promote weak_ptr → shared_ptr to verify device is still alive.
//         If expired, the rule is silently aborted (device was deleted).
// Step 3: Invoke turnOn() / turnOff() polymorphically via the vtable.
//         No dynamic_cast required — virtual dispatch handles it cleanly.

void AutomationRuleEngine::executeAction(const AutomationRule& rule)
{
    auto it = deviceMap.find(rule.deviceId);
    if (it == deviceMap.end())
    {
        std::cerr << "[RuleEngine] Error: Device ID '"
                  << rule.deviceId << "' not found in registry.\n";
        return;
    }

    std::shared_ptr<SmartDevice> devicePtr = it->second.lock();
    if (!devicePtr)
    {
        std::cerr << "[RuleEngine] Error: Target device is no longer valid "
                     "(expired weak_ptr). Aborting rule.\n";
        return;
    }

    if (rule.action == RuleAction::TURN_ON)
    {
        devicePtr->turnOn();
        std::cout << "[RuleEngine] Action dispatched: turnOn() for '"
                  << rule.deviceId << "'\n";
    }
    else if (rule.action == RuleAction::TURN_OFF)
    {
        devicePtr->turnOff();
        std::cout << "[RuleEngine] Action dispatched: turnOff() for '"
                  << rule.deviceId << "'\n";
    }
    else
    {
        std::cout << "[RuleEngine] Log: Unhandled action for rule ID "
                  << rule.ruleID << "\n";
    }
}

// ---- Public API ------------------------------------------------------------

void AutomationRuleEngine::addRule(const AutomationRule& rule)
{
    m_rules.push_back(rule);
}

// registerDevice stores a weak_ptr — the engine does NOT own the device.
// This prevents the engine from accidentally extending a device's lifetime
// beyond the Home class's lifetime, which would cause the shared_ptr
// reference count to never reach zero and block the virtual destructor.
void AutomationRuleEngine::registerDevice(const std::string& id,
                                          const std::shared_ptr<SmartDevice>& device)
{
    deviceMap[id] = device;
}

// Hot path: called on every sensor tick.
// Iterates the entire rule list for each sensor update.
// For large rule sets, a sensor-ID-indexed hash map would reduce this
// from O(N) to O(K) where K = rules matching that sensor.
void AutomationRuleEngine::onSensorUpdate(int sensorID, double value)
{
    for (const auto& rule : m_rules)
    {
        if (rule.targetSensorID == sensorID && rule.enabled)
        {
            if (evaluateCondition(rule, value))
                executeAction(rule);
        }
    }
}

void AutomationRuleEngine::clearRules()
{
    m_rules.clear();
}

size_t AutomationRuleEngine::ruleCount() const
{
    return m_rules.size();
}
