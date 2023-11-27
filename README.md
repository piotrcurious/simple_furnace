# simple_furnace
simple furnace controller with active exhaust pump dreamed by BingAI 

I'll try to create that. Here is the README file I generated for describing the functionality of the code, design assumptions and design goals:

# Oil Furnace Controller

This is a Arduino sketch that implements an oil furnace controller with airflow control and watchdog subsystems. The sketch uses a VT100 terminal to visualize the state of the furnace in 40x24 mode.

## Functionality

The sketch performs the following functions:

- It measures the input fan RPM using a hall sensor attached to an interrupt pin.
- It controls the output fan speed using a PWM pin based on the combustion level read from an analog input pin.
- It boosts the output fan speed if the input fan RPM falls below 200 RPM or the temperature exceeds 50 Celsius.
- It reads the temperature using a thermistor connected to an analog input pin and a SmoothThermistor library.
- It checks the airflow rate and activates a beep pin if the input fan RPM falls below 100 RPM.
- It restarts the input fan by activating a digital pin for 500 ms if the input fan stalls.
- It activates an overload warning light pin if the output fan speed plus the boost value exceeds the maximum PWM value.
- It uses a watchdog timer to reset the Arduino in case of a software failure.
- It uses a Ticker library to schedule the safety tasks and the visualization tasks at regular intervals.
- It uses a VT100 library to clear the screen and set the cursor position for the visualization objects.
- It visualizes the input fan RPM, output fan speed, combustion level, temperature, ratio of input RPM to output PWM, beep state, and overload state on the VT100 terminal.

## Design Assumptions

The sketch makes the following design assumptions:

- The input fan hall sensor generates one pulse per two revolutions.
- The combustion level is a linear function of the output fan speed.
- The temperature sensor is a 10k NTC thermistor with a beta value of 3950 and a nominal temperature of 25 Celsius.
- The VT100 terminal is connected to the Arduino via serial communication at 9600 baud rate.
- The sketch is part of a bigger design that implements other functions of the oil furnace controller, such as ignition, flame detection, fuel injection, etc.

## Design Goals

The sketch aims to achieve the following design goals:

- To ensure the safety and efficiency of the oil furnace operation by controlling the airflow and monitoring the temperature.
- To prevent the input fan from stalling and the output fan from overloading by using feedback mechanisms and corrective actions.
- To detect and recover from software failures by using a watchdog timer.
- To provide a clear and informative visualization of the state of the furnace by using a VT100 terminal.
