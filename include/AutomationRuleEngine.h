#ifndef AUTOMATIONRULEENGINE_H
#define AUTOMATIONRULEENGINE_H

// NEW: دعم Qt
#include <QObject>
#include <QString>

// MERGED: من الكود الأول
#include "SmartDevice.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// RuleCondition — condition tokens
// ---------------------------------------------------------------------------
// NEW: إضافة GTE و LTE لمرونة أكبر
enum class RuleCondition { GT, LT, EQ, GTE, LTE };

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

    // CHANGED: deviceId -> deviceID (توحيد naming)
    std::string   deviceID;

    RuleAction    action;
    bool          enabled;
};

// ---------------------------------------------------------------------------
// AutomationRuleEngine (MERGED with QObject)
// ---------------------------------------------------------------------------
class AutomationRuleEngine : public QObject
{
    Q_OBJECT   // NEW: required for signals/slots

private:
    // MERGED: من الكود الأول
    std::vector<AutomationRule> m_rules;

    // MERGED: device map
    std::unordered_map<std::string, std::weak_ptr<SmartDevice>> deviceMap;

    // NEW: تحسين الأداء (rules لكل sensor)
    std::unordered_map<int, std::vector<AutomationRule>> rulesBySensor;

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    bool evaluateCondition(const AutomationRule& rule, double value) const;

    // CHANGED: بقت بتعمل logging كمان
    void executeAction(const AutomationRule& rule);

    // NEW: logging helper من كود Qt
    void log(const QString& msg);

public:
    // NEW: constructor بتاع Qt
    explicit AutomationRuleEngine(QObject *parent = nullptr);

    // -----------------------------------------------------------------------
    // Rule Management
    // -----------------------------------------------------------------------
    void   addRule(const AutomationRule& rule);        // CHANGED: هتحدث rulesBySensor
    void   clearRules();
    size_t ruleCount() const;

    // NEW: enable/disable rules بدل التعديل اليدوي
    void enableRule(int ruleID);
    void disableRule(int ruleID);

    // -----------------------------------------------------------------------
    // Device Management
    // -----------------------------------------------------------------------
    void registerDevice(const std::string& id,
                        const std::shared_ptr<SmartDevice>& device);

    // -----------------------------------------------------------------------
    // Runtime
    // -----------------------------------------------------------------------
    void onSensorUpdate(int sensorID, double value);

    // NEW: من كود Qt — تشغيل rule بالاسم
    void processRuleTriggered(const std::string& ruleName);

signals:
    // MERGED: signal من كود Qt
    void eventLogged(const QString& message);
};

#endif // AUTOMATIONRULEENGINE_H
