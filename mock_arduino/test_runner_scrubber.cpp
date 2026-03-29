#include "Arduino.h"
#include "Simulator.h"
#include <iostream>
#include <iomanip>

extern void setup();
extern void loop();

int main() {
    setup();
    FurnaceSimulator sim;

    std::cout << "=== SCENARIO: EXTENDED EFFICIENCY AND AGING TEST ===" << std::endl;
    std::cout << "Time(s) | Tin | Tout | Fin | Fout | Power | FanDuty | PumpDuty" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;

    extern bool _loop_enabled;
    for (int i = 0; i < 200; i++) {
        if (!_loop_enabled) break;

        // Phase 1: High Dynamic Load (0-100s)
        if (i < 100) {
            sim.mass_flow = 1.2 + 0.4 * sin(i / 15.0);
            sim.exhaust_in_temp = 350.0 + 100.0 * cos(i / 8.0);
        }
        // Phase 2: Rapid Aging/Fouling under Stress (100-200s)
        else {
            sim.exhaust_in_temp = 450.0;
            sim.mass_flow = 1.5;
            sim.aging_factor = std::max(0.3f, 1.0f - (i - 100) * 0.015f);
        }

        // Advance time and update simulator
        sim.update(1.0); // 1s step

        extern void advance_millis(uint32_t ms);
        advance_millis(1000);

        // Run loop (scrubbers usually use loop-based control)
        loop();

        if (i % 5 == 0) {
            extern float power;
            extern float fan_duty;
            extern float pump_duty;
            std::cout << std::fixed << std::setprecision(1)
                      << (float)i << " | "
                      << sim.exhaust_in_temp << " | "
                      << sim.exhaust_out_temp << " | "
                      << sim.fluid_in_temp << " | "
                      << sim.fluid_out_temp << " | "
                      << power << " | "
                      << fan_duty << " | "
                      << pump_duty << std::endl;
        }
    }
    return 0;
}
