#include "TestListener.h"
#include <iostream>

TestListener::TestListener() {}

TestListener::~TestListener() {}

void TestListener::onTemperatureChange(float temperature) {
    (void)temperature; // Silence unused parameter warning
    // Test listener implementation - can be extended for specific behavior
}