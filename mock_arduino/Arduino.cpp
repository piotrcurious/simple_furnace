#include "Arduino.h"
#include <chrono>
#include <thread>
#include <vector>

Serial_ Serial;

static uint32_t start_time = 0; // In ms

uint32_t millis() {
    return start_time; // Modified to be manual for simulation control
}

void advance_millis(uint32_t ms) {
    start_time += ms;
}

void delay(uint32_t ms) {
    advance_millis(ms);
}

bool _loop_enabled = true;
void noLoop() { _loop_enabled = false; }

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Serial_::begin(unsigned long baud) {}
void Serial_::print(const char* s) { printf("%s", s); }
void Serial_::print(int n, int base) { printf("%d", n); }
void Serial_::print(double n, int digits) { printf("%.*f", digits, n); }
void Serial_::println(const char* s) { printf("%s\n", s); }
void Serial_::println(int n, int base) { printf("%d\n", n); }
void Serial_::println(double n, int digits) { printf("%.*f\n", digits, n); }
void Serial_::println() { printf("\n"); }

long random(long howbig) { return rand() % howbig; }
long random(long howsmall, long howbig) { return howsmall + (rand() % (howbig - howsmall)); }
void randomSeed(unsigned long seed) { srand(seed); }

// Global state for pin values (mocking hardware)
int pin_modes[100];
int digital_pins[100];
int analog_pins[16];
void (*interrupt_funcs[100])(void);

void pinMode(uint8_t pin, uint8_t mode) { pin_modes[pin] = mode; }
void digitalWrite(uint8_t pin, uint8_t val) { digital_pins[pin] = val; }
int digitalRead(uint8_t pin) { return digital_pins[pin]; }
int analogRead(uint8_t pin) { return analog_pins[pin]; }
void analogWrite(uint8_t pin, int val) { digital_pins[pin] = val; } // Simplified

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
    interrupt_funcs[interruptNum] = userFunc;
}
uint8_t digitalPinToInterrupt(uint8_t pin) { return pin; }
