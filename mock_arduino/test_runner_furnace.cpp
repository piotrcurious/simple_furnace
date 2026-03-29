#include "Arduino.h"
#include "Ticker.h"
#include "Simulator.h"
#include <iostream>
#include <iomanip>

extern void setup();
extern void loop();
extern Ticker safetyTicker;
// Use a weak reference or check if it exists if possible,
// but in C++ we'll just provide a dummy if not defined in the .ino
Ticker dummyTicker;
#pragma weak visualizationTicker
extern Ticker visualizationTicker;

int main() {
    setup();
    FurnaceSimulator sim;

    Ticker* vTicker = &visualizationTicker;
    if (vTicker == nullptr) vTicker = &dummyTicker;

    std::cout << "=== SCENARIO: STRESS TEST WITH FAILURE ===" << std::endl;
    std::cout << "Time(s) | RPM | Temp(C) | OutFan | Beep | Overload" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    for (int i = 0; i < 400; i++) {
        uint32_t now = i * 100; // Step 100ms

        // Scenario 1: Heavy load (0-5s)
        if (i < 50) analogWrite(0, 800); // High combustion
        // Scenario 2: Sudden drop (5-10s)
        else if (i < 100) analogWrite(0, 200);
        // Scenario 3: Oscillating Combustion (10-25s)
        else if (i < 250) analogWrite(0, 500 + 300 * sin(i / 10.0));

        // Failure scenario: Sensor fails (at 30s)
        if (i == 300) {
            std::cout << "[SIM] Triggering critical sensor failure on pin 1" << std::endl;
            sim.sensor_fail[1] = true;
        }

        // Advance time and update simulator
        sim.update(0.1);

        // Update Arduino global time
        extern void advance_millis(uint32_t ms);
        advance_millis(100);

        // Run safety tasks from Ticker
        safetyTicker.update(now);
        vTicker->update(now);

        // Run loop
        loop();

        if (i % 10 == 0) {
            extern float inputFanRPM;
            extern int temperature;
            extern int outputFanSpeed;
            extern bool beepState;
            extern bool overloadState;

            std::cout << std::fixed << std::setprecision(1)
                      << i * 0.1 << " | "
                      << inputFanRPM << " | "
                      << temperature << " | "
                      << outputFanSpeed << " | "
                      << (beepState ? "ON" : "OFF") << " | "
                      << (overloadState ? "ON" : "OFF") << std::endl;
        }
    }
    return 0;
}
