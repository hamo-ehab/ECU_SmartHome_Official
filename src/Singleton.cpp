#include "AutomationRuleEngine.h"

// تعريف المتغير الثابت
std::shared_ptr<MySingleton> MySingleton::s_instance = nullptr;

// تنفيذ getInstance
std::shared_ptr<MySingleton> MySingleton::getInstance() {
    if (!s_instance) {
        s_instance = std::shared_ptr<MySingleton>(new MySingleton());
    }
    return s_instance;
}

// تنفيذ resetInstance
void MySingleton::resetInstance() {
    s_instance.reset();
}

// Constructor
MySingleton::MySingleton(QObject* parent) : QObject(parent) {}

// Destructor
MySingleton::~MySingleton() {
    // منطق التدمير إذا لزم الأمر
    std::cout << "MySingleton destroyed" << std::endl;
}

// تنفيذ addRule
void MySingleton::addRule(const QString& rule) {
    m_rules.push_back(rule);
}