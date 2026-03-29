import matplotlib.pyplot as plt
import re
import os

def clean_line(line):
    # Strip ANSI escapes
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    line = ansi_escape.sub('', line)
    return line.replace("[DATA]", "").strip()

def parse_furnace_log(filename):
    times, rpms, temps = [], [], []
    if not os.path.exists(filename): return [], [], []
    with open(filename, "r") as f:
        for raw_line in f:
            line = clean_line(raw_line)
            parts = line.split('|')
            if len(parts) >= 3:
                try:
                    times.append(float(parts[0].strip()))
                    rpms.append(float(parts[1].strip()))
                    temps.append(float(parts[2].strip()))
                except ValueError:
                    continue
    return times, rpms, temps

def parse_scrubber_log(filename):
    times, tin, tout, fin, fout = [], [], [], [], []
    if not os.path.exists(filename): return [], [], [], [], []
    with open(filename, "r") as f:
        for raw_line in f:
            line = clean_line(raw_line)
            parts = line.split('|')
            if len(parts) >= 5:
                try:
                    times.append(float(parts[0].strip()))
                    tin.append(float(parts[1].strip()))
                    tout.append(float(parts[2].strip()))
                    fin.append(float(parts[3].strip()))
                    fout.append(float(parts[4].strip()))
                except (ValueError, IndexError):
                    continue
    return times, tin, tout, fin, fout

def generate_graphs():
    os.makedirs("docs/images", exist_ok=True)
    plt.style.use('bmh')

    # Furnace graph - Comprehensive
    if os.path.exists("furnace.ino.log"):
        t, r, temp = parse_furnace_log("furnace.ino.log")
        if t:
            fig, ax1 = plt.subplots(figsize=(12, 6))
            ax1.set_xlabel('Time (s)')
            ax1.set_ylabel('Temperature (C)', color='tab:red')
            ax1.plot(t, temp, color='tab:red', linewidth=2, label="Temperature")
            ax1.tick_params(axis='y', labelcolor='tab:red')

            ax2 = ax1.twinx()
            ax2.set_ylabel('Fan RPM', color='tab:blue')
            ax2.plot(t, r, color='tab:blue', linestyle='--', label="Fan RPM")
            ax2.tick_params(axis='y', labelcolor='tab:blue')

            plt.title("Furnace Stress Test: Load Fluctuations & Sensor Failure")
            fig.tight_layout()
            plt.savefig("docs/images/furnace_graph.png")
            plt.close()

    # Scrubber graph - Comprehensive
    if os.path.exists("scrubber_optimized.ino.log"):
        t, tin, tout, fin, fout = parse_scrubber_log("scrubber_optimized.ino.log")
        if t:
            plt.figure(figsize=(12, 6))
            plt.plot(t, tin, 'r-', label="Exhaust In (Hot)")
            plt.plot(t, tout, 'g-', label="Exhaust Out (Cooled)")
            plt.plot(t, fout, 'b--', label="Scrubber Fluid Out")
            plt.fill_between(t, tout, tin, color='orange', alpha=0.2, label="Heat Extracted")

            plt.title("Advanced Scrubber Dynamics: Variable Flow & Thermal Adaptation")
            plt.xlabel("Time (s)")
            plt.ylabel("Temperature (C)")
            plt.legend(loc='upper right')
            plt.grid(True)
            plt.savefig("docs/images/scrubber_graph.png")
            plt.close()

    # Gaming Scrubber graph
    if os.path.exists("gaming_scrubber.ino.log"):
        t, tin, tout, fin, fout = parse_scrubber_log("gaming_scrubber.ino.log")
        if t:
            plt.figure(figsize=(12, 6))
            plt.plot(t, tin, 'r-', label="Exhaust In")
            plt.plot(t, tout, 'g-', label="Exhaust Out")
            plt.plot(t, fout, 'b--', label="Fluid Out")
            plt.title("Genetic Algorithm Scrubber Evolution")
            plt.xlabel("Time (s)")
            plt.ylabel("Temperature (C)")
            plt.legend()
            plt.grid(True)
            plt.savefig("docs/images/gaming_graph.png", dpi=150)
            plt.close()

    # Comparison Graph
    plt.figure(figsize=(10, 6))
    for f in ["scrubber_optimized.ino.log", "gaming_scrubber.ino.log", "pso_scrubber.ino.log"]:
        if os.path.exists(f):
            t, tin, tout, fin, fout = parse_scrubber_log(f)
            if not t: continue
            # Power extracted = (tin - tout) * 1.2
            p = [(i - o) * 1.2 for i, o in zip(tin, tout)]
            plt.plot(t, p, label=f.replace(".ino.log", ""), alpha=0.8)

    plt.title("Algorithm Efficiency Comparison (Heat Extraction Power)")
    plt.xlabel("Time (s)")
    plt.ylabel("Power (W)")
    plt.legend()
    plt.grid(True)
    plt.savefig("docs/images/algorithm_comparison.png", dpi=150)
    plt.close()

    # Stress Test Response Comparison
    plt.figure(figsize=(10, 6))
    for f in ["furnace.ino.log", "vt100_cooling.ino.log"]:
         if os.path.exists(f):
            t, r, temp = parse_furnace_log(f)
            if t: plt.plot(t, temp, label=f.replace(".ino.log", ""))
    plt.axvline(x=30, color='r', linestyle='--', label="Sensor Failure Event")
    plt.title("Fail-Safe Response Comparison (Temperature during Sensor Loss)")
    plt.xlabel("Time (s)")
    plt.ylabel("Temperature (C)")
    plt.legend()
    plt.grid(True)
    plt.savefig("docs/images/stress_test_comparison.png", dpi=150)
    plt.close()

if __name__ == "__main__":
    generate_graphs()
