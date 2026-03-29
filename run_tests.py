#!/usr/bin/env python3
import os
import subprocess
import sys
import re

def test_file(ino_file):
    print(f"--- Testing {ino_file} ---")
    cpp_file = ino_file.replace(".ino", ".cpp")

    with open(ino_file, "r") as f:
        content = f.read()

    if '#include "Arduino.h"' not in content:
        content = '#include "Arduino.h"\n#include "Ticker.h"\n#include "SmoothThermistor.h"\n#include "VT100.h"\n#include "avr/wdt.h"\n#include "EEPROM.h"\n' + content

    if "scrubber" in ino_file:
        if "power" not in content: content = "float power;\n" + content
        if "fan_duty" not in content: content = "float fan_duty;\n" + content
        if "pump_duty" not in content: content = "float pump_duty;\n" + content

    if "safety_monitor.ino" in ino_file:
        for var in ["float inputFanRPM", "int temperature", "int outputFanSpeed", "bool beepState", "bool overloadState"]:
            if var not in content: content = var + ";\n" + content

    if "vt100_cooling.ino" in ino_file:
        content = content.replace("thresholdTemperature = analogRead(THRESHOLD_PIN);", "int thresholdTemperature = analogRead(THRESHOLD_PIN);")
        content = content.replace("temperature < thresholdTemperature", "temperature < (int)thresholdTemperature")
        content = content.replace("thresholdTemperature - temperature", "(int)thresholdTemperature - temperature")
        content = content.replace("temperature > thresholdTemperature", "temperature > (int)thresholdTemperature")
        content = content.replace("temperature - thresholdTemperature", "temperature - (int)thresholdTemperature")

    with open(cpp_file, "w") as f:
        f.write(content)

    is_furnace = any(x in ino_file for x in ["furnace", "cooling", "safety"])
    is_scrubber = "scrubber" in ino_file or "greedy" in ino_file
    runner = "mock_arduino/test_runner_furnace.cpp" if is_furnace else "mock_arduino/test_runner_scrubber.cpp"

    output_bin = ino_file.replace(".ino", ".out")
    cmd = ["g++", "-I", "mock_arduino", "mock_arduino/Arduino.cpp", "mock_arduino/EEPROM.cpp", cpp_file, runner, "-o", output_bin]

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Compilation FAILED for {ino_file}:\n{result.stderr}")
        return False, []

    result = subprocess.run(["./" + output_bin], capture_output=True, text=True)
    with open(ino_file + ".log", "w") as f: f.write(result.stdout)
    print(result.stdout)

    passed = not ("Warning" in result.stdout or "Overload: ON" in result.stdout or result.stderr)

    powers = []
    # More robust parsing for data lines, stripping ANSI escape codes
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    clean_stdout = ansi_escape.sub('', result.stdout)

    for line in clean_stdout.split('\n'):
        if '|' not in line: continue
        if not is_furnace and "[DATA]" not in line: continue

        parts = [p.strip() for p in line.replace("[DATA]", "").split('|')]
        if len(parts) >= 6:
            try:
                if not is_furnace:
                    # scrubber parts: [time, tin, tout, fin, fout, power, fan_duty, pump_duty]
                    # Try to find the first numeric-looking part that could be power
                    # Usually parts[5] but let's be safe
                    p_val = -1.0
                    for p in parts[4:]: # Power is usually around 5 or 6
                        try:
                            p_val = float(p)
                            if p_val > -100: break # Found it
                        except: continue
                else:
                    # furnace parts: [fan_rpm, temperature, fan_duty, beep_pin, overload, scrubber]
                    p_val = float(parts[1]) * float(parts[2]) / 100.0
                if p_val != -1.0:
                    powers.append(p_val)
            except: continue

    if powers:
        interval = 1.0 if is_furnace else 5.0
        avg_p = sum(powers) / len(powers)
        total_e = sum(powers) * interval
        metrics_line = f"Metrics: Avg Power = {avg_p:.2f} W, Total Energy = {total_e:.1f} J"
        print(metrics_line)
        # Force write metrics to log so we can parse it later
        with open(ino_file + ".log", "a") as f:
             f.write("\n" + metrics_line + "\n")

    return passed, powers

import re

if __name__ == "__main__":
    results, metrics = {}, []
    files = [sys.argv[1]] if len(sys.argv) > 1 else sorted([f for f in os.listdir(".") if f.endswith(".ino")])

    for f in files:
        status, powers = test_file(f)
        results[f] = status

        # Benchmarking logic
        log_path = f + ".log"
        if os.path.exists(log_path):
            with open(log_path, "r") as lf:
                out_str = lf.read()
                m = re.search(r"Metrics: Avg Power = ([\d.]+) W, Total Energy = ([\d.]+) J", out_str)
                if m:
                    p_avg = float(m.group(1))
                    e_total = float(m.group(2))
                    stability = (sum((x - p_avg)**2 for x in powers)/len(powers))**0.5 if powers else 0
                    metrics.append({"file": f, "avg_power": p_avg, "total_energy": e_total, "stability": stability, "status": "PASS" if status else "FAIL"})

    print("\n--- TEST SUMMARY ---")
    for f, s in results.items(): print(f"{f}: {'PASS' if s else 'FAIL'}")

    if metrics:
        header = "| Controller | Avg Power (W) | Total Energy (J) | Stability (σ) | Status |"
        sep = "|------------|---------------|------------------|---------------|--------|"
        report = f"# Benchmarking\n\n{header}\n{sep}\n"
        for m in sorted(metrics, key=lambda x: x['total_energy'], reverse=True):
            line = f"| {m['file']} | {m['avg_power']:.2f} | {m['total_energy']:.1f} | {m['stability']:.2f} | {m['status']} |"
            print(line)
            report += line + "\n"
        with open("BENCHMARK.md", "w") as f: f.write(report)

    if not all(results.values()): sys.exit(1)
