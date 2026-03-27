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
