#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Arduino.h"
#include <vector>

extern int analog_pins[16];
extern int digital_pins[100];
extern void (*interrupt_funcs[100])(void);

class FurnaceSimulator {
public:
    float temperature = 20.0;
    float input_fan_rpm = 0.0;
    int combustion_level = 512;
    float target_rpm = 0.0;

    // Scrubber variables
    float exhaust_in_temp = 300.0;
    float exhaust_out_temp = 100.0;
    float fluid_in_temp = 25.0;
    float fluid_out_temp = 50.0;

    void update(float dt_s) {
        // Dynamic scenarios based on millis()
        uint32_t t = millis();
        // Fluctuate exhaust in temp every 10 seconds
        exhaust_in_temp = 300.0 + 50.0 * sin(t / 10000.0);
        // Constants for physics
        const float ambient_temp = 20.0;
        const float furnace_thermal_mass = 50.0; // Seconds to heat/cool
        const float exhaust_heat_transfer_coeff = 0.5;
        const float fluid_heat_transfer_coeff = 0.3;
        const float scrubber_thermal_mass = 20.0;

        // Simple furnace physics
        // Fan RPM follows target (proportional to PWM)
        int output_fan_pwm = digital_pins[3]; // OUTPUT_FAN_PIN
        target_rpm = (output_fan_pwm / 255.0) * 3000.0;
        input_fan_rpm += (target_rpm - input_fan_rpm) * dt_s * 0.5;

        // Pulse the hall sensor
        static float pulse_accumulator = 0;
        pulse_accumulator += (input_fan_rpm / 60.0) * dt_s * 2.0; // 2 pulses per rev
        while (pulse_accumulator >= 1.0) {
            if (interrupt_funcs[2]) interrupt_funcs[2](); // INPUT_FAN_PIN
            pulse_accumulator -= 1.0;
        }

        // Temperature dynamics with thermal inertia
        combustion_level = analogRead(0);
        float heat_in = (combustion_level / 1024.0) * 50.0;
        float heat_out = (input_fan_rpm / 3000.0) * 20.0 + (temperature - ambient_temp) * 0.5;
        temperature += (heat_in - heat_out) * dt_s / furnace_thermal_mass;

        // Scrubber physics with more realism
        float scrubber_fan_duty = digital_pins[9] / 255.0;
        float scrubber_pump_duty = digital_pins[10] / 255.0;

        // Heat exchange from exhaust to scrubber fluid
        float delta_t_exhaust = (exhaust_in_temp - fluid_out_temp);
        float exchange_rate = delta_t_exhaust * scrubber_fan_duty * exhaust_heat_transfer_coeff;

        exhaust_out_temp = exhaust_in_temp - exchange_rate;

        // Fluid temperature changes with heat from exhaust and cooling from pump/ambient
        float heat_gain = (exhaust_in_temp - exhaust_out_temp) * 2.0;
        float heat_loss = scrubber_pump_duty * (fluid_out_temp - fluid_in_temp) * 5.0 + (fluid_out_temp - ambient_temp) * 0.2;
        fluid_out_temp += (heat_gain - heat_loss) * dt_s / scrubber_thermal_mass;

        // Map to analog pins with sensor noise (+/- 1 LSB jitter)
        auto add_noise = [](int val) {
            return val + (rand() % 3 - 1);
        };
        // Combustion pin remains A0 for furnace, but scrubber also uses A0 for exhaust_in
        // We'll handle this by assuming the test runner sets these pins before or during the update
        analog_pins[0] = add_noise((int)(exhaust_in_temp / 0.48828125));
        analog_pins[1] = add_noise((int)(exhaust_out_temp / 0.48828125));
        analog_pins[2] = add_noise((int)(fluid_in_temp / 0.48828125));
        analog_pins[3] = add_noise((int)(fluid_out_temp / 0.48828125));

        // For furnace tests, A0 is combustion level, A1 is temperature
        // The runners will need to handle the overlap if they test both at once.
    }
};

#endif
