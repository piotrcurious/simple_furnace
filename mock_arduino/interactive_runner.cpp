#include "Arduino.h"
#include "Ticker.h"
#include "Simulator.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

extern void setup();
extern void loop();
extern Ticker safetyTicker;
Ticker dummyTicker;
#pragma weak visualizationTicker
extern Ticker visualizationTicker;

// Global variables that might be in .ino
#pragma weak inputFanRPM
extern float inputFanRPM;
#pragma weak temperature
extern int temperature;
#pragma weak outputFanSpeed
extern int outputFanSpeed;
#pragma weak beepState
extern bool beepState;
#pragma weak overloadState
extern bool overloadState;
#pragma weak power
extern float power;
#pragma weak fan_duty
extern float fan_duty;
#pragma weak pump_duty
extern float pump_duty;

void print_json(FurnaceSimulator& sim) {
    std::cout << "{\"time\":" << millis()
              << ",\"sim_temp\":" << sim.temperature
              << ",\"sim_rpm\":" << sim.input_fan_rpm
              << ",\"sim_fluid_out\":" << sim.fluid_out_temp
              << ",\"sim_exhaust_out\":" << sim.exhaust_out_temp;

    // Attempt to access .ino variables if they exist (weak refs)
    if (&inputFanRPM) std::cout << ",\"ino_rpm\":" << inputFanRPM;
    if (&temperature) std::cout << ",\"ino_temp\":" << temperature;
    if (&outputFanSpeed) std::cout << ",\"ino_out_fan\":" << outputFanSpeed;
    if (&beepState) std::cout << ",\"ino_beep\":" << (beepState ? 1 : 0);
    if (&overloadState) std::cout << ",\"ino_overload\":" << (overloadState ? 1 : 0);
    if (&power) std::cout << ",\"ino_power\":" << power;
    if (&fan_duty) std::cout << ",\"ino_fan_duty\":" << fan_duty;
    if (&pump_duty) std::cout << ",\"ino_pump_duty\":" << pump_duty;

    std::cout << "}" << std::endl;
}

#include <fcntl.h>
#include <unistd.h>

int main() {
    // Make stdin non-blocking
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);

    setup();
    FurnaceSimulator sim;
    Ticker* vTicker = &visualizationTicker;
    if (vTicker == nullptr) vTicker = &dummyTicker;

    while (true) {
        for(int i=0; i<10; i++) {
            sim.update(0.01);
            extern void advance_millis(uint32_t ms);
            advance_millis(10);
            safetyTicker.update(millis());
            vTicker->update(millis());
            loop();
        }

        print_json(sim);

        char buf[1024];
        int n = read(0, buf, sizeof(buf)-1);
        if (n > 0) {
            buf[n] = 0;
            std::string cmd(buf);
            std::stringstream ss(cmd);
            std::string key;
            float val;
            while(ss >> key >> val) {
                if (key == "ambient") { /* Update ambient if added to sim */ }
                if (key == "fail") sim.sensor_fail[(int)val] = true;
                if (key == "fix") sim.sensor_fail[(int)val] = false;
                if (key == "exhaust_in") sim.exhaust_in_temp = val;
            }
        }
        usleep(10000); // Reduce CPU usage slightly
    }
    return 0;
}
