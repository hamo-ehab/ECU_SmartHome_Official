#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include <memory>
#include <vector>
#include "TestListener.h"

class TemperatureSensor {
private:
    std::vector<std::shared_ptr<TestListener>> listeners;
    float temperature = 20.0f;

public:
    TemperatureSensor();
    ~TemperatureSensor();
    void addListener(std::shared_ptr<TestListener> listener);
    void simulate();
};

#endif