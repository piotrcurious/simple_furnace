# Mock Arduino Test System

This directory contains a mock Arduino environment and a furnace/scrubber simulator to test the `.ino` files without real hardware.

## Structure

- `mock_arduino/`: Contains the mocked Arduino core and libraries.
    - `Arduino.h`, `Arduino.cpp`: Core functions (IO, time, Serial, etc.).
    - `Ticker.h`, `SmoothThermistor.h`, `VT100.h`, `EEPROM.h`: Mocked libraries.
    - `Simulator.h`: Physical model of the furnace and scrubber.
    - `test_runner_furnace.cpp`: Main entry point for furnace sketches.
    - `test_runner_scrubber.cpp`: Main entry point for scrubber sketches.
- `run_tests.py`: Python script to compile and run all tests.

## Running Tests

To test all files, run:
```bash
python3 run_tests.py
```

To test a specific file, run:
```bash
python3 run_tests.py <filename.ino>
```

## How it Works

The `run_tests.py` script:
1. Pre-processes the `.ino` file into a `.cpp` file.
2. Compiles it together with the mock Arduino core and a test runner using `g++`.
3. Executes the resulting binary, which runs the `setup()` and `loop()` functions against the `FurnaceSimulator`.
4. Outputs the simulation state over time to the console.

## Physics Model

The `FurnaceSimulator` models:
- Fan RPM dynamics.
- Hall sensor pulse generation for RPM measurement.
- Temperature changes based on combustion and fan cooling.
- Scrubber heat exchange between exhaust and fluid.
- Sensor mapping to Arduino analog pins.

## Production-Level Testing

The system now includes:
- **Sensor Noise**: Simulates real-world signal jitter.
- **Sensor Failure**: Mechanism to test controller robustness against stuck-at-zero or out-of-range sensor readings.
- **Stability Analysis**: The test runner detects unstable duty cycle oscillations.
- **Dynamic Scenarios**: Time-varying heat loads to test adaptation of genetic and greedy algorithms.
