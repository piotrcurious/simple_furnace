#ifndef SMOOTHTHERMISTOR_H
#define SMOOTHTHERMISTOR_H

#include "Arduino.h"

#define ADC_SIZE_10_BIT 1024

class SmoothThermistor {
public:
    SmoothThermistor(uint8_t pin, int adc_size, int series_resistor, int thermistor_resistor, int beta, int nominal_temp, int samples)
    : pin(pin) {}

    float temperature() {
        // Simple mock: assume linear for now or just return mapped analog value
        // Actual thermistor math isn't critical for initial tests as long as we can control it.
        // Let's implement a basic mapping for simulation.
        int val = analogRead(pin);
        return (val * 100.0 / 1024.0); // Simple 0-100 degrees C mapping
    }

private:
    uint8_t pin;
};

#endif
