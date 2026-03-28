#include <Ticker.h>
#include <avr/wdt.h>
#include <VT100.h>

// Pins as defined in README and Simulator
#define CO_SENSOR_PIN A4
#define ALARM_PIN 5
#define SHUTDOWN_PIN 7

#define CO_THRESHOLD 100 // Warning level
#define CO_DANGER 250 // Shutdown level

int coLevel = 0;
Ticker safetyTicker;
VT100 vt100;

void monitorSafety() {
    coLevel = analogRead(CO_SENSOR_PIN);

    if (coLevel > CO_DANGER) {
        digitalWrite(SHUTDOWN_PIN, HIGH);
        digitalWrite(ALARM_PIN, HIGH);
    } else if (coLevel > CO_THRESHOLD) {
        digitalWrite(ALARM_PIN, HIGH);
    } else {
        digitalWrite(ALARM_PIN, LOW);
    }
}

void updateDisplay() {
    vt100.clearScreen();
    vt100.setCursorPosition(5, 2);
    vt100.setBold(true);
    vt100.setForeground(VT100::WHITE);
    vt100.print("--- EXTERNAL SAFETY MONITOR ---");
    vt100.setBold(false);

    vt100.setCursorPosition(5, 5);
    vt100.print("CO LEVEL: ");
    if (coLevel > CO_DANGER) vt100.setForeground(VT100::RED);
    else if (coLevel > CO_THRESHOLD) vt100.setForeground(VT100::YELLOW);
    else vt100.setForeground(VT100::GREEN);
    vt100.print(coLevel);

    vt100.setCursorPosition(5, 7);
    vt100.setForeground(VT100::WHITE);
    vt100.print("STATUS:   ");
    if (coLevel > CO_DANGER) {
        vt100.setForeground(VT100::RED);
        vt100.setBold(true);
        vt100.print("!!! SHUTDOWN !!!");
    } else if (coLevel > CO_THRESHOLD) {
        vt100.setForeground(VT100::YELLOW);
        vt100.print("WARNING: VENTILATE");
    } else {
        vt100.setForeground(VT100::GREEN);
        vt100.print("SAFE");
    }
}

void setup() {
    pinMode(ALARM_PIN, OUTPUT);
    pinMode(SHUTDOWN_PIN, OUTPUT);
    Serial.begin(9600);
    vt100.begin(&Serial);
    safetyTicker.attach(1.0, monitorSafety);
    Ticker displayTicker; // Dummy for scope
    // We'll just run it in loop for now or add more tickers
    wdt_enable(WDTO_2S);
}

void loop() {
    safetyTicker.update(millis());
    static unsigned long lastDisp = 0;
    if (millis() - lastDisp > 1000) {
        updateDisplay();
        lastDisp = millis();
    }
    wdt_reset();
}
