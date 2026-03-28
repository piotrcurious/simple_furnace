#include "Arduino.h"
#include "Simulator.h"
#include <iostream>
#include <iomanip>

extern void setup();
extern void loop();

int main() {
    setup();
    FurnaceSimulator sim;

    std::cout << "Time(s) | Tin | Tout | Fin | Fout | Power | FanDuty | PumpDuty" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;

    extern bool _loop_enabled;
    for (int i = 0; i < 50; i++) {
        if (!_loop_enabled) break;
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
