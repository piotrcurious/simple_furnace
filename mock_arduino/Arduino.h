#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define FALLING 2
#define RISING 3

#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4
#define A5 5

#define PI 3.1415926535897932384626433832795

typedef uint8_t byte;
typedef bool boolean;

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);

uint32_t millis(void);
uint32_t micros(void);
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
void noLoop();

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode);
uint8_t digitalPinToInterrupt(uint8_t pin);

void randomSeed(unsigned long seed);
long random(long howbig);
long random(long howsmall, long howbig);

template<class T, class L>
auto min(T a, L b) -> decltype(a < b ? a : b) { return a < b ? a : b; }
template<class T, class L>
auto max(T a, L b) -> decltype(a > b ? a : b) { return a > b ? a : b; }
template<class T, class L, class M>
auto constrain(T amt, L low, M high) -> decltype(amt) { return (amt < low) ? low : ((amt > high) ? high : amt); }
long map(long x, long in_min, long in_max, long out_min, long out_max);

class Serial_ {
public:
    void begin(unsigned long baud);
    void print(const char* s);
    void print(int n, int base = 10);
    void print(double n, int digits = 2);
    void println(const char* s);
    void println(int n, int base = 10);
    void println(double n, int digits = 2);
    void println();
};

extern Serial_ Serial;

#endif
