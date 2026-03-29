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

    if "safety_monitor.ino" in ino_file:
        if "float inputFanRPM" not in content: content = "float inputFanRPM;\n" + content
        if "int temperature" not in content: content = "int temperature;\n" + content
        if "int outputFanSpeed" not in content: content = "int outputFanSpeed;\n" + content
        if "bool beepState" not in content: content = "bool beepState;\n" + content
        if "bool overloadState" not in content: content = "bool overloadState;\n" + content

    if "vt100_cooling.ino" in ino_file:
        content = content.replace("thresholdTemperature = analogRead(THRESHOLD_PIN);", "int thresholdTemperature = analogRead(THRESHOLD_PIN);")
        content = content.replace("temperature < thresholdTemperature", "temperature < (int)thresholdTemperature")
        content = content.replace("thresholdTemperature - temperature", "(int)thresholdTemperature - temperature")
        content = content.replace("temperature > thresholdTemperature", "temperature > (int)thresholdTemperature")
        content = content.replace("temperature - thresholdTemperature", "temperature - (int)thresholdTemperature")

    with open(cpp_file, "w") as f:
        f.write(content)

    # Compile with test runner
    is_furnace = "furnace" in ino_file or "cooling" in ino_file or "safety" in ino_file
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

    # Save output for graphing and terminal grabs
    with open(ino_file + ".log", "w") as f:
        f.write(result.stdout)

    print(result.stdout)

    # Simple pass/fail based on output
    passed = True

    # Efficiency Metrics - Derived from simulation table for fairness
    import re
    powers = []
    # Table format: Time(s) | Tin | Tout | Fin | Fout | Power | FanDuty | PumpDuty
    for line in result.stdout.split('\n'):
        if '|' not in line: continue
        parts = [p.strip() for p in line.split('|')]
        if len(parts) >= 6:
            try:
                # Look for numerical values in 2nd and 3rd columns (Tin, Tout)
                tin = float(parts[1])
                tout = float(parts[2])
                # Power = deltaT * 1.2 (consistent with Simulator.h logic)
                p_val = (tin - tout) * 1.2
                if p_val < 0: p_val = 0
                powers.append(p_val)
            except (ValueError, IndexError):
                continue

    if powers:
        total_energy = sum(powers)
        avg_power = total_energy / len(powers)
        print(f"Metrics: Avg Power = {avg_power:.2f} W, Total Energy = {total_energy:.1f} J")

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

import re

if __name__ == "__main__":
    results = {}
    metrics = []

    files_to_test = []
    if len(sys.argv) > 1:
        files_to_test = [sys.argv[1]]
    else:
        files_to_test = sorted([f for f in os.listdir(".") if f.endswith(".ino")])

    for f in files_to_test:
        # Capture metrics from stdout
        import io
        from contextlib import redirect_stdout
        f_out = io.StringIO()
        with redirect_stdout(f_out):
            status = test_file(f)
        results[f] = status
        out_str = f_out.getvalue()
        print(out_str) # Print to real stdout too

        # Parse metrics for benchmarking
        m = re.search(r"Metrics: Avg Power = ([\d.]+) W, Total Energy = ([\d.]+) J", out_str)
        if m:
            metrics.append({
                "file": f,
                "avg_power": float(m.group(1)),
                "total_energy": float(m.group(2)),
                "status": "PASS" if status else "FAIL"
            })

    print("\n--- TEST SUMMARY ---")
    all_pass = True
    for file, status in results.items():
        print(f"{file}: {'PASS' if status else 'FAIL'}")
        if not status: all_pass = False

    if metrics:
        print("\n--- BENCHMARKING REPORT ---")
        print("| Controller | Avg Power (W) | Total Energy (J) | Status |")
        print("|------------|---------------|------------------|--------|")
        for m in sorted(metrics, key=lambda x: x['total_energy'], reverse=True):
            print(f"| {m['file']} | {m['avg_power']:.2f} | {m['total_energy']:.1f} | {m['status']} |")

        with open("BENCHMARK.md", "w") as f:
            f.write("# Controller Benchmarking Results\n\n")
            f.write("| Controller | Avg Power (W) | Total Energy (J) | Status |\n")
            f.write("|------------|---------------|------------------|--------|\n")
            for m in sorted(metrics, key=lambda x: x['total_energy'], reverse=True):
                 f.write(f"| {m['file']} | {m['avg_power']:.2f} | {m['total_energy']:.1f} | {m['status']} |\n")

    if not all_pass:
        sys.exit(1)
