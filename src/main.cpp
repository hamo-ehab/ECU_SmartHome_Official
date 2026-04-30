/*
 * =============================================================================
 * FILE: src/main.cpp
 * PROJECT: ECU Smart Home System
 * ORIGINAL AUTHOR: Eng. Zain (Lead Architect)
 * REVIEWED & OPTIMIZED BY: Eng. Zain
 * MERGED FROM ORIGINAL FILES:
 *   - main2.cpp       (SmartDoor log test)
 *   - main3.cpp       (SmartDoor voice test)
 *   - main4.cpp       (SmartDoor serialise/deserialise test)
 *   - Source1.cpp     (SmartCamera inline test)
 *   - project day 1.cpp  (VoiceService inline class + manual main)
 * =============================================================================
 * Canonical integration test / entry point.
 * Exercises device instantiation, logging, persistence, ring buffers,
 * rule engine dispatch, and graceful shutdown sequence.
 * =============================================================================
 */

#include "../include/SmartDevice.h"
#include "../include/SmartDoor.h"
#include "../include/SmartLight.h"
#include "../include/SmartCamera.h"
#include "../include/SmartAC.h"
#include "../include/MotionSensor.h"
#include "../include/AutomationRuleEngine.h"
#include "../include/VoiceService.h"

#include <memory>
#include <vector>
#include <iostream>

// ---------------------------------------------------------------------------
// Helper: prints a section banner
// ---------------------------------------------------------------------------
static void banner(const char* title)
{
    std::cout << "\n=== " << title << " ===\n";
}

// ---------------------------------------------------------------------------
// main — system integration test
// ---------------------------------------------------------------------------
int main()
{
    std::cout << "--- ECU Smart Home System: Integration Test ---\n\n";

    // ---- 1. Device instantiation (heap-managed via shared_ptr) -------------
    banner("Device Instantiation");

    auto frontDoor  = std::make_shared<SmartDoor>("DOOR-01", "Front Door", true);
    auto livingLight = std::make_shared<SmartLight>("LIGHT-01", "Living Room Light", 80);
    auto secCamera  = std::make_shared<SmartCamera>("CAM-01", "Front Camera");
    auto airCon     = std::make_shared<SmartAC>("AC-01", "Main AC",
                                                SmartAC::Mode::COOLING, 22.0);

    // ---- 2. Door lock / unlock cycle with access-log ring buffer -----------
    banner("SmartDoor Lock Cycle");

    frontDoor->unlock();
    frontDoor->lock();
    frontDoor->unlock();
    frontDoor->lock();
    frontDoor->showLogs();

    // ---- 3. SmartDoor serialise / deserialise (persistence test) -----------
    banner("SmartDoor Serialise / Deserialise");

    std::string savedState = frontDoor->serialise();
    std::cout << "[Disk I/O] Saved CSV: " << savedState << "\n";

    auto restoredDoor = std::make_shared<SmartDoor>("DOOR-02", "Restored Door", false);
    restoredDoor->deserialise(savedState);
    std::cout << "[Restored] Power=" << restoredDoor->getPowerStatus()
              << " Locked=" << restoredDoor->getLockStatus() << "\n";

    // Firewall test: corrupted / truncated data
    restoredDoor->deserialise("10"); // Missing delimiter → rejected gracefully

    // ---- 4. SmartLight brightness + wattage --------------------------------
    banner("SmartLight Brightness");

    livingLight->setBrightness(50);
    livingLight->displayStatus();
    livingLight->setBrightness(0);   // Should trigger turnOff()
    livingLight->displayStatus();

    // ---- 5. SmartCamera motion-alert edge detection + ring buffer ----------
    banner("SmartCamera Motion Alert");

    secCamera->raiseMotionAlert(false);   // No transition
    secCamera->raiseMotionAlert(true);    // FALSE → TRUE: triggers startRec()
    secCamera->raiseMotionAlert(true);    // No transition (already true)

    for (int i = 0; i < 10; ++i)
        secCamera->update();              // Advance ring buffer 10 ticks

    secCamera->displayStatus();

    // ---- 6. SmartAC temperature clamping -----------------------------------
    banner("SmartAC Temperature");

    airCon->setTemperature(35.0);  // Clamps to 30.0
    airCon->displayStatus();
    airCon->setTemperature(10.0);  // Clamps to 16.0
    airCon->displayStatus();

    // ---- 7. AutomationRuleEngine -------------------------------------------
    banner("AutomationRuleEngine");

    AutomationRuleEngine engine;
    engine.registerDevice("DOOR-01", frontDoor);
    engine.registerDevice("LIGHT-01", livingLight);

    AutomationRule rule1;
    rule1.ruleID         = 1;
    rule1.targetSensorID = 42;
    rule1.condition      = RuleCondition::GT;
    rule1.threshold      = 0.5;
    rule1.deviceID       = "LIGHT-01";
    rule1.action         = RuleAction::TURN_ON;
    rule1.enabled        = true;

    engine.addRule(rule1);

    // Simulate a sensor firing above threshold
    engine.onSensorUpdate(42, 1.0);

    // ---- 8. MotionSensor observer -------------------------------------------
    banner("MotionSensor Simulation");

    auto motionSensor = std::make_shared<MotionSensor>();

    // SmartCamera as listener: wrap in a lightweight adapter
    struct CameraAdapter : public ISensorListener
    {
        SmartCamera& cam;
        explicit CameraAdapter(SmartCamera& c) : cam(c) {}
        void onMotionChanged(double value) override
        {
            cam.raiseMotionAlert(value > 0.5);
        }
    };
    auto adapter = std::make_shared<CameraAdapter>(*secCamera);
    motionSensor->addListener(adapter);

    for (int i = 0; i < 20; ++i)
        motionSensor->simulate();

    // ---- 9. SmartDoor Day 5 Simulation --------------------------------------
    banner("SmartDoor Day 5 Simulation");
    {
        std::cout << "--- ECU Smart Home: Day 5 Simulation ---\n\n";
        SmartDoor myDoor("DOOR-05", "Day 5 Task Door", true);

        myDoor.unlock();
        std::cout << "[Normal State] User opened the door.\n";
        myDoor.displayStatus();

        std::cout << "\n[SIMULATION] SIMULATING POWER OUTAGE...\n\n";
        myDoor.turnOff(); // Architectural equivalent of setPowerStatus(false)

        myDoor.update();
        std::cout << "\n[Status after Emergency Tick]\n";
        myDoor.displayStatus();
    }

    // ---- 10. Shutdown sequence (governed by Eng Zain / Muhammad Ehab) -------
    banner("System Shutdown");

    engine.clearRules();  // Step 1: clear rule references

    // Step 2: release all device shared_ptrs → ref-count hits 0 → destructors fire
    std::vector<std::shared_ptr<SmartDevice>> allDevices =
        { frontDoor, restoredDoor, livingLight, secCamera, airCon };

    allDevices.clear();
    frontDoor.reset();
    restoredDoor.reset();
    livingLight.reset();
    secCamera.reset();
    airCon.reset();

    // Step 3: VoiceService static local is destroyed by C++ runtime at program exit
    std::cout << "\n[System] All devices released. Shutdown complete.\n";

    return 0;
}
