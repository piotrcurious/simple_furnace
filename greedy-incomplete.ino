// Incomplete greedy scrubber controller
// Missing setup and loop implementation

#include "Arduino.h"
#include "EEPROM.h"

#define FAN_PIN 9
#define PUMP_PIN 10
#define IN_TEMP_PIN A0
#define OUT_TEMP_PIN A1
#define FLUID_IN_TEMP_PIN A2
#define FLUID_OUT_TEMP_PIN A3

float in_temp, out_temp, fluid_in_temp, fluid_out_temp;
float fan_duty, pump_duty, power, best_power;

void setup() {
    Serial.begin(9600);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    best_power = 0;
}

void loop() {
    in_temp = analogRead(IN_TEMP_PIN) * 0.48828125;
    out_temp = analogRead(OUT_TEMP_PIN) * 0.48828125;
    power = (in_temp - out_temp) * 1.2;

    if (power > best_power) {
        best_power = power;
    }

    fan_duty = 50; // Placeholder
    pump_duty = 50; // Placeholder

    analogWrite(FAN_PIN, fan_duty * 2.55);
    analogWrite(PUMP_PIN, pump_duty * 2.55);

    Serial.print("Power: ");
    Serial.println(power);
    delay(1000);
}
