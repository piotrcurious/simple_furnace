#include <Ticker.h>
#include <avr/wdt.h>
#include <VT100.h>
#include "VT100Visualizer.h"
#include <EEPROM.h>

#define TEMP_IN A0
#define TEMP_OUT A1
#define TEMP_FLUID A2
#define FAN 9
#define PUMP 10

#define MAX_TEMP 100
#define MAX_POWER 200
#define Kp 0.1
#define Ki 0.01
#define Kd 0.05
#define EEPROM_ADDR 0

float temp_in, temp_out, temp_fluid, power;
float fan_duty = 0, pump_duty = 0;
float error, integral = 0, derivative, prev_error = 0;
float a, b, c;
float lyapunov, best_lyapunov = 1000;

VT100 vt100;
VT100Visualizer visualizer(vt100);
Ticker safetyTicker;
Ticker visualizationTicker;

int powerHistory[20];
int hIdx = 0;

void safetyTask() {
  temp_in = analogRead(TEMP_IN) * 0.48828125;
  temp_out = analogRead(TEMP_OUT) * 0.48828125;
  temp_fluid = analogRead(TEMP_FLUID) * 0.48828125;

  power = (temp_in - temp_out) * fan_duty / 100.0;
  error = MAX_POWER - power;
  integral += error;
  derivative = error - prev_error;
  prev_error = error;

  fan_duty = constrain(Kp * error + Ki * integral + Kd * derivative, 0, 100);
  analogWrite(FAN, fan_duty * 2.55);

  pump_duty = constrain(a * power + b * temp_fluid + c, 0, 100);
  if (temp_fluid > MAX_TEMP * 0.9) pump_duty += 20;
  pump_duty = constrain(pump_duty, 0, 100);
  analogWrite(PUMP, pump_duty * 2.55);

  lyapunov = pow(error, 2) + pow(temp_fluid - MAX_TEMP, 2);
  if (lyapunov < best_lyapunov) {
    best_lyapunov = lyapunov;
    EEPROM.put(EEPROM_ADDR, a);
  }
}

void visualizationTask() {
  vt100.clearScreen();
  visualizer.drawBorder(1, 1, 39, 23, VT100::BLUE);
  visualizer.drawHeader("SCRUBBER OPTIMIZER v3.0");

  visualizer.drawScrubberArt(5, 4, (int)(fan_duty * 30), (int)pump_duty);

  visualizer.drawBorder(20, 3, 38, 10, VT100::WHITE);
  vt100.setCursorPosition(21, 4); vt100.print("POWER:"); vt100.print(power);
  visualizer.drawProgressBar(21, 5, 15, (power / MAX_POWER) * 100.0, VT100::YELLOW);

  vt100.setCursorPosition(21, 7); vt100.print("FLUID:"); vt100.print(temp_fluid);
  visualizer.drawProgressBar(21, 8, 15, (temp_fluid / MAX_TEMP) * 100.0, VT100::BLUE);

  powerHistory[hIdx] = (int)power;
  hIdx = (hIdx + 1) % 20;
  vt100.setCursorPosition(5, 15); vt100.print("POWER TREND:");
  visualizer.drawGraph(5, 16, 20, 5, powerHistory, 20, 200, VT100::GREEN);

  vt100.setCursorPosition(25, 18);
  if (lyapunov < 100) { vt100.setForeground(VT100::GREEN); vt100.print("STABLE"); }
  else { vt100.setForeground(VT100::YELLOW); vt100.print("ADAPTING"); }
}

void setup() {
  Serial.begin(9600);
  vt100.begin(&Serial);
  pinMode(FAN, OUTPUT);
  pinMode(PUMP, OUTPUT);
  a = 0.5; b = 0.1; c = 10;
  safetyTicker.attach(0.1, safetyTask);
  visualizationTicker.attach(1.0, visualizationTask);
  wdt_enable(WDTO_2S);
}

void loop() {
  safetyTicker.update(millis());
  visualizationTicker.update(millis());
  wdt_reset();
}
