import matplotlib.pyplot as plt
import re
import os

def parse_furnace_log(filename):
    times, rpms, temps = [], [], []
    with open(filename, "r") as f:
        for line in f:
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
    with open(filename, "r") as f:
        for line in f:
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

    # Furnace graph
    if os.path.exists("furnace.ino.log"):
        t, r, temp = parse_furnace_log("furnace.ino.log")
        plt.figure(figsize=(10, 5))
        plt.plot(t, temp, label="Temperature (C)")
        plt.plot(t, [x/10 for x in r], label="RPM / 10", alpha=0.5)
        plt.title("Furnace Controller Response")
        plt.xlabel("Time (s)")
        plt.legend()
        plt.grid(True)
        plt.savefig("docs/images/furnace_graph.png")
        plt.close()

    # Scrubber graph
    if os.path.exists("scrubber_optimized.ino.log"):
        t, tin, tout, fin, fout = parse_scrubber_log("scrubber_optimized.ino.log")
        plt.figure(figsize=(10, 5))
        plt.plot(t, tin, label="Exhaust In")
        plt.plot(t, tout, label="Exhaust Out")
        plt.plot(t, fout, label="Fluid Out")
        plt.title("Scrubber Performance")
        plt.xlabel("Time (s)")
        plt.legend()
        plt.grid(True)
        plt.savefig("docs/images/scrubber_graph.png")
        plt.close()

if __name__ == "__main__":
    generate_graphs()
