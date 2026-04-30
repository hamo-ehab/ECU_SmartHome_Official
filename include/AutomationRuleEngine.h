/*
 * =============================================================================
 * FILE: include/AutomationRuleEngine.h
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Mohamed Sameh Ghonem (ID: 692500817)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - AutomationRuleEngine.cpp       (onSensorUpdate / evaluateCondition)
 *   - AutomationRuleEngine (1).cpp   (duplicate — identical logic, removed)
 *   - AutomationRuleEngine (1)(1).cpp (executeAction with weak_ptr dispatch)
 * =============================================================================
 * Rule evaluation engine: sensor hot-path, GT/LT/EQ epsilon evaluation,
 * weak_ptr-safe polymorphic action dispatch to registered devices.
 * DECLARATION ONLY — implementations live in src/AutomationRuleEngine.cpp
 * =============================================================================
 */

#ifndef AUTOMATIONRULEENGINE_H
#define AUTOMATIONRULEENGINE_H

#include "SmartDevice.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// RuleCondition — condition tokens
// ---------------------------------------------------------------------------
enum class RuleCondition { GT, LT, EQ };

// ---------------------------------------------------------------------------
// RuleAction — action identifiers
// ---------------------------------------------------------------------------
enum class RuleAction { TURN_ON, TURN_OFF };

// ---------------------------------------------------------------------------
// AutomationRule — plain data struct (one IF-THEN rule)
// ---------------------------------------------------------------------------
struct AutomationRule
{
    int           ruleID;
    int           targetSensorID;
    RuleCondition condition;
    double        threshold;
    std::string   deviceId;
    RuleAction    action;
    bool          enabled;
};

// ---------------------------------------------------------------------------
// AutomationRuleEngine
// ---------------------------------------------------------------------------
class AutomationRuleEngine
{
private:
    std::vector<AutomationRule>                                    m_rules;
    std::unordered_map<std::string, std::weak_ptr<SmartDevice>>    deviceMap;

    // Private helpers — implementations in AutomationRuleEngine.cpp
    bool evaluateCondition(const AutomationRule& rule, double value) const;
    void executeAction    (const AutomationRule& rule);

public:
    void   addRule        (const AutomationRule& rule);
    void   registerDevice (const std::string& id,
                           const std::shared_ptr<SmartDevice>& device);
    void   onSensorUpdate (int sensorID, double value);
    void   clearRules     ();
    size_t ruleCount      () const;
};

#endif // AUTOMATIONRULEENGINE_H
