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

// MERGED: دعم Qt logging
#include <QDebug>

// MERGED: من الكود التاني
#include <iostream>
#include <cmath>

// NEW: constructor من نسخة Qt
AutomationRuleEngine::AutomationRuleEngine(QObject *parent)
    : QObject(parent) {}

// ---------------------------------------------------------------------------
// NEW: logging helper (Qt signal)
// ---------------------------------------------------------------------------
void AutomationRuleEngine::log(const QString& msg)
{
    emit eventLogged(msg); // NEW
    
    // Fallback to std::cout so terminal tests still show logs
    std::cout << msg.toStdString() << "\n";
}

// ---------------------------------------------------------------------------
// NEW: process rule trigger (Qt side)
// ---------------------------------------------------------------------------
void AutomationRuleEngine::processRuleTriggered(const std::string& ruleName)
{
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), "Rule Fired: %s", ruleName.c_str());

    if (len > 0 && len < (int)sizeof(buffer))
    {
        QString qMsg = QString::fromUtf8(buffer, len);
        QString finalMessage = "<b>[LOG]</b> " + qMsg;

        log(finalMessage); // NEW
    }
}

// ---------------------------------------------------------------------------
// MERGED: condition evaluation + دعم GTE / LTE
// ---------------------------------------------------------------------------
bool AutomationRuleEngine::evaluateCondition(const AutomationRule& rule,
                                             double value) const
{
    if (rule.condition == RuleCondition::GT)  return value >  rule.threshold;
    if (rule.condition == RuleCondition::LT)  return value <  rule.threshold;
    if (rule.condition == RuleCondition::GTE) return value >= rule.threshold; // NEW
    if (rule.condition == RuleCondition::LTE) return value <= rule.threshold; // NEW

    if (rule.condition == RuleCondition::EQ)
    {
        constexpr double epsilon = 0.000001;
        return std::abs(value - rule.threshold) < epsilon;
    }
    return false;
}

// ---------------------------------------------------------------------------
// MERGED: executeAction + Qt logging بدل cout/cerr
// ---------------------------------------------------------------------------
void AutomationRuleEngine::executeAction(const AutomationRule& rule)
{
    // CHANGED: deviceId -> deviceID
    auto it = deviceMap.find(rule.deviceID);

    if (it == deviceMap.end())
    {
        // CHANGED: بدل std::cerr → Qt log
        log(QString("[RuleEngine] Error: Device ID '%1' not found")
            .arg(QString::fromStdString(rule.deviceID)));
        return;
    }

    std::shared_ptr<SmartDevice> devicePtr = it->second.lock();

    if (!devicePtr)
    {
        log("[RuleEngine] Error: Device expired (weak_ptr)");
        return;
    }

    if (rule.action == RuleAction::TURN_ON)
    {
        devicePtr->turnOn();

        // NEW: Qt logging
        log(QString("[RuleEngine] turnOn() -> %1")
            .arg(QString::fromStdString(rule.deviceID)));
    }
    else if (rule.action == RuleAction::TURN_OFF)
    {
        devicePtr->turnOff();

        // NEW: Qt logging
        log(QString("[RuleEngine] turnOff() -> %1")
            .arg(QString::fromStdString(rule.deviceID)));
    }
    else
    {
        log(QString("[RuleEngine] Unknown action for rule %1")
            .arg(rule.ruleID));
    }
}

// ---------------------------------------------------------------------------
// MERGED: addRule + تحسين الأداء (rulesBySensor)
// ---------------------------------------------------------------------------
void AutomationRuleEngine::addRule(const AutomationRule& rule)
{
    m_rules.push_back(rule);

    // NEW: indexing by sensor
    rulesBySensor[rule.targetSensorID].push_back(rule);
}

// ---------------------------------------------------------------------------
// MERGED: register device
// ---------------------------------------------------------------------------
void AutomationRuleEngine::registerDevice(const std::string& id,
                                          const std::shared_ptr<SmartDevice>& device)
{
    deviceMap[id] = device;
}

// ---------------------------------------------------------------------------
// CHANGED: optimized باستخدام rulesBySensor بدل loop كامل
// ---------------------------------------------------------------------------
void AutomationRuleEngine::onSensorUpdate(int sensorID, double value)
{
    auto it = rulesBySensor.find(sensorID);

    if (it == rulesBySensor.end())
        return;

    for (const auto& rule : it->second)
    {
        if (rule.enabled && evaluateCondition(rule, value))
        {
            executeAction(rule);

            // NEW: log rule trigger
            processRuleTriggered(std::to_string(rule.ruleID));
        }
    }
}

// ---------------------------------------------------------------------------
// MERGED
// ---------------------------------------------------------------------------
void AutomationRuleEngine::clearRules()
{
    m_rules.clear();
    rulesBySensor.clear(); // NEW
}

size_t AutomationRuleEngine::ruleCount() const
{
    return m_rules.size();
}

// ---------------------------------------------------------------------------
// NEW: enable / disable rules
// ---------------------------------------------------------------------------
void AutomationRuleEngine::enableRule(int ruleID)
{
    for (auto& rule : m_rules)
        if (rule.ruleID == ruleID)
            rule.enabled = true;
}

void AutomationRuleEngine::disableRule(int ruleID)
{
    for (auto& rule : m_rules)
        if (rule.ruleID == ruleID)
            rule.enabled = false;
}
