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

        // Temperature depends on combustion (simulated by COMBUSTION_PIN A0 input)
        // and cooling from fan
        combustion_level = analogRead(0);
        temperature += (combustion_level / 1024.0) * 10.0 * dt_s;
        temperature -= (input_fan_rpm / 3000.0) * 5.0 * dt_s;
        temperature -= (temperature - 20.0) * 0.1 * dt_s; // Ambient cooling

        // Scrubber physics
        int scrubber_fan_pwm = digital_pins[9];
        int scrubber_pump_pwm = digital_pins[10];

        // Exhaust out temp decreases with fan speed
        exhaust_out_temp = exhaust_in_temp - (scrubber_fan_pwm / 255.0) * (exhaust_in_temp - fluid_in_temp) * 0.8;
        // Fluid out temp increases with heat from exhaust, decreases with pump speed
        fluid_out_temp = fluid_in_temp + (exhaust_in_temp - exhaust_out_temp) * 0.5 - (scrubber_pump_pwm / 255.0) * 20.0;

        // Map to analog pins
        analog_pins[0] = combustion_level;
        analog_pins[1] = (int)(temperature * 1024.0 / 100.0); // SmoothThermistor mapping

        // Scrubber mapping
        analog_pins[0] = (int)(exhaust_in_temp / 0.48828125);
        analog_pins[1] = (int)(exhaust_out_temp / 0.48828125);
        analog_pins[2] = (int)(fluid_in_temp / 0.48828125);
        analog_pins[3] = (int)(fluid_out_temp / 0.48828125);
    }
};

#endif
