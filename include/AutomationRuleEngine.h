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

// NEW: دعم Qt
#include <QtCore/QObject>
#include <QtCore/QString>

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

#ifndef SINGLETON_H
#define SINGLETON_H

#include <iostream>
#include <memory>
#include <vector>
#include <QtCore/QObject>
#include <QtCore/QString>

class MySingleton : public QObject {
    Q_OBJECT

public:
    // الوصول إلى النسخة الوحيدة
    static std::shared_ptr<MySingleton> getInstance();

    // الدالة المطلوبة لعمل teardown يدوي
    static void resetInstance();

    // مثال على إدارة البيانات (rules vector)
    void addRule(const QString& rule);
    
    // التدمير (Destructor) - هنا يقع المنطق الحرج
    ~MySingleton();

private:
    // إخفاء الـ Constructor لضمان نمط الـ Singleton
    explicit MySingleton(QObject* parent = nullptr);

    // منع النسخ والنقل
    MySingleton(const MySingleton&) = delete;
    MySingleton& operator=(const MySingleton&) = delete;

    static std::shared_ptr<MySingleton> s_instance;
    std::vector<QString> m_rules; // Rules vector المذكور في المتطلبات
};

#endif // SINGLETON_H

#endif // AUTOMATIONRULEENGINE_H
