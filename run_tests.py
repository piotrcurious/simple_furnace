#!/usr/bin/env python3
import os
import subprocess
import sys

def test_file(ino_file):
    print(f"--- Testing {ino_file} ---")
    cpp_file = ino_file.replace(".ino", ".cpp")

    # Pre-process .ino to .cpp (basic: include mocked headers)
    with open(ino_file, "r") as f:
        content = f.read()

    # Add necessary includes if they are not there
    if '#include "Arduino.h"' not in content:
        content = '#include "Arduino.h"\n#include "Ticker.h"\n#include "SmoothThermistor.h"\n#include "VT100.h"\n#include "avr/wdt.h"\n#include "EEPROM.h"\n' + content

    # Patch some common issues in these .ino files
    # Define common variables for the test runner to access
    if "scrubber" in ino_file:
        # Avoid defining if already present, but handle different types (int vs float)
        if "power" not in content:
            content = "float power;\n" + content
        if "fan_duty" not in content:
            content = "float fan_duty;\n" + content
        if "pump_duty" not in content:
            content = "float pump_duty;\n" + content

    if "vt100_cooling.ino" in ino_file:
        content = content.replace("thresholdTemperature = analogRead(THRESHOLD_PIN);", "int thresholdTemperature = analogRead(THRESHOLD_PIN);")
        content = content.replace("temperature < thresholdTemperature", "temperature < (int)thresholdTemperature")
        content = content.replace("thresholdTemperature - temperature", "(int)thresholdTemperature - temperature")
        content = content.replace("temperature > thresholdTemperature", "temperature > (int)thresholdTemperature")
        content = content.replace("temperature - thresholdTemperature", "temperature - (int)thresholdTemperature")

    with open(cpp_file, "w") as f:
        f.write(content)

    # Compile with test runner
    is_furnace = "furnace" in ino_file or "cooling" in ino_file
    runner = "mock_arduino/test_runner_furnace.cpp" if is_furnace else "mock_arduino/test_runner_scrubber.cpp"

    output_bin = ino_file.replace(".ino", ".out")
    cmd = [
        "g++", "-I", "mock_arduino",
        "mock_arduino/Arduino.cpp",
        "mock_arduino/EEPROM.cpp",
        cpp_file,
        runner,
        "-o", output_bin
    ]

    print(f"Compiling: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Compilation FAILED for {ino_file}:")
        print(result.stderr)
        return False

    # Run the test
    print(f"Running {output_bin}...")
    result = subprocess.run(["./" + output_bin], capture_output=True, text=True)
    print(result.stdout)

    # Simple pass/fail based on output
    passed = True
    if "Warning" in result.stdout:
        passed = False
    if "Overload: ON" in result.stdout:
        passed = False

    # Stability check: Detect large oscillations in duty cycles
    import re
    fan_duties = [float(f) for f in re.findall(r"FanDuty: ([\d.]+)", result.stdout)]
    if not fan_duties:
        fan_duties = [float(f) for f in re.findall(r"Fan duty: ([\d.]+)", result.stdout)]
    if not fan_duties:
        fan_duties = [float(f) for f in re.findall(r"Fan duty cycle: ([\d.]+)", result.stdout)]

    if len(fan_duties) > 10:
        # Calculate standard deviation of the last 10 readings
        recent = fan_duties[-10:]
        avg = sum(recent) / 10
        variance = sum((x - avg) ** 2 for x in recent) / 10
        std_dev = variance ** 0.5
        if std_dev > 25.0: # Threshold for high oscillation
            print(f"Warning: High fan duty oscillation (std_dev={std_dev:.2f})")
            passed = False

    # Thermal runaway check
    temps = [float(t) for t in re.findall(r"sim_temp: ([\d.]+)", result.stdout)]
    if not temps:
        temps = [float(t) for t in re.findall(r"Temp\(C\): ([\d.]+)", result.stdout)]
    if not temps:
        # Check standard runner output format
        import re
        # The furnace runner outputs in a table format.
        # Time(s) | RPM | Temp(C) | OutFan | Beep | Overload
        # 0.0 | 0.0 | 0 | 0 | OFF | OFF
        for line in result.stdout.split('\n'):
            parts = line.split('|')
            if len(parts) >= 3:
                try:
                    t_val = float(parts[2].strip())
                    temps.append(t_val)
                except ValueError:
                    continue

    if len(temps) > 20:
        # If temperature is rising at the end of simulation despite max cooling
        recent_temps = temps[-10:]
        if recent_temps[-1] > recent_temps[0] + 5.0 and recent_temps[-1] > 100:
             print(f"Warning: Potential thermal runaway detected (Temp rising: {recent_temps[0]:.1f} -> {recent_temps[-1]:.1f})")
             passed = False

    if result.stderr:
        print("Errors:")
        print(result.stderr)
        passed = False

    return passed

if __name__ == "__main__":
    results = {}
    if len(sys.argv) > 1:
        results[sys.argv[1]] = test_file(sys.argv[1])
    else:
        # Test all .ino files
        for f in os.listdir("."):
            if f.endswith(".ino"):
                results[f] = test_file(f)

    print("\n--- TEST SUMMARY ---")
    all_pass = True
    for file, status in results.items():
        print(f"{file}: {'PASS' if status else 'FAIL'}")
        if not status: all_pass = False

    if not all_pass:
        sys.exit(1)
