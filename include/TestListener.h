#ifndef TESTLISTENER_H
#define TESTLISTENER_H

class TestListener {
public:
    TestListener();
    ~TestListener();
    void onTemperatureChange(float temperature);
};

#endif