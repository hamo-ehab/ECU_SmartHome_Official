#include "TemperatureSensor.h"
#include <cstdlib> // for rand

TemperatureSensor::TemperatureSensor() {}

TemperatureSensor::~TemperatureSensor() {}

void TemperatureSensor::addListener(std::shared_ptr<TestListener> listener) {
    listeners.push_back(listener);
}

void TemperatureSensor::simulate() {
    // Simulate temperature change
    temperature += (rand() % 3 - 1); // Random change: -1, 0, or 1
    for (auto& listener : listeners) {
        listener->onTemperatureChange(temperature);
    }
}