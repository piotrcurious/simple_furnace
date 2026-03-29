#include "Arduino.h"
#include "Simulator.h"
#include <iostream>
#include <iomanip>

extern void setup();
extern void loop();

int main() {
    // Enable scrubber by default (Pin 2 is SCRUBBER_ACTIVE_PIN in most sketches)
    extern int digital_pins[100];
    digital_pins[2] = 1;

    setup();
    FurnaceSimulator sim;

    std::cout << "=== SCENARIO: EXTENDED EFFICIENCY AND AGING TEST ===" << std::endl;
    std::cout << "Time(s) | Tin | Tout | Fin | Fout | Power | FanDuty | PumpDuty" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;

    extern bool _loop_enabled;
    for (int i = 0; i < 600; i++) {
        if (!_loop_enabled) break;

        // Complex real-world scenario: Multi-phase Stress Test
        if (i < 100) {
            // Phase 1: Startup & Dynamic Stabilization
            sim.mass_flow = 1.2 + 0.4 * sin(i / 15.0);
            sim.exhaust_in_temp = 350.0 + 50.0 * cos(i / 8.0);
        }
        else if (i < 300) {
            // Phase 2: Rapid Seasonal Ambient Shift
            sim.ambient_temp = 20.0 + 30.0 * sin(i / 50.0);
            sim.exhaust_in_temp = 400.0;
        }
        else if (i < 500) {
            // Phase 3: Hardware Aging / Fouling
            sim.aging_factor = std::max(0.3f, 1.0f - (i - 300) * 0.003f);
            sim.exhaust_in_temp = 450.0;
        }
        else {
            // Phase 4: Peak Load Stress
            sim.exhaust_in_temp = 500.0;
            sim.mass_flow = 2.0;
        }

        // Advance time and update simulator
        sim.update(1.0); // 1s step

        extern void advance_millis(uint32_t ms);
        advance_millis(1000);

        // Run loop (scrubbers usually use loop-based control)
        loop();

        if (i % 5 == 0) {
            float current_power = (sim.exhaust_in_temp - sim.exhaust_out_temp) * 1.2;
            float actual_fan_duty = digital_pins[9] / 2.55;
            float actual_pump_duty = digital_pins[10] / 2.55;

            std::cout << "[DATA] " << std::fixed << std::setprecision(1)
                      << (float)i << " | "
                      << sim.exhaust_in_temp << " | "
                      << sim.exhaust_out_temp << " | "
                      << sim.fluid_in_temp << " | "
                      << sim.fluid_out_temp << " | "
                      << current_power << " | "
                      << actual_fan_duty << " | "
                      << actual_pump_duty << std::endl;
        }
    }
    return 0;
}
