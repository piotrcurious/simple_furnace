import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import subprocess
import threading
import json
import os
import time
import select

class FurnaceGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Oil Furnace & Scrubber Simulator")
        self.process = None
        self.running = False

        self.setup_ui()

    def setup_ui(self):
        # File Selection
        self.files_frame = ttk.LabelFrame(self.root, text="Controller Selection")
        self.files_frame.pack(fill="x", padx=10, pady=5)

        self.files_list = [f for f in os.listdir(".") if f.endswith(".ino")]
        self.selected_file = tk.StringVar()
        if self.files_list: self.selected_file.set(self.files_list[0])

        self.file_menu = ttk.OptionMenu(self.files_frame, self.selected_file, *self.files_list)
        self.file_menu.pack(side="left", padx=5, pady=5)

        self.start_btn = ttk.Button(self.files_frame, text="Start Simulation", command=self.start_sim)
        self.start_btn.pack(side="left", padx=5, pady=5)

        self.stop_btn = ttk.Button(self.files_frame, text="Stop", command=self.stop_sim, state="disabled")
        self.stop_btn.pack(side="left", padx=5, pady=5)

        # Monitoring
        self.mon_frame = ttk.LabelFrame(self.root, text="Real-Time Monitoring")
        self.mon_frame.pack(fill="both", expand=True, padx=10, pady=5)

        self.stats = {}
        for i, key in enumerate(["sim_temp", "sim_rpm", "sim_fluid_out", "ino_out_fan", "ino_power"]):
            lbl = ttk.Label(self.mon_frame, text=f"{key}:")
            lbl.grid(row=i, column=0, sticky="w", padx=5)
            val = ttk.Label(self.mon_frame, text="N/A")
            val.grid(row=i, column=1, sticky="w", padx=5)
            self.stats[key] = val

        # Controls
        self.ctrl_frame = ttk.LabelFrame(self.root, text="Simulator Controls")
        self.ctrl_frame.pack(fill="x", padx=10, pady=5)

        ttk.Label(self.ctrl_frame, text="Exhaust In Temp:").pack(side="left", padx=5)
        self.exhaust_in_slider = ttk.Scale(self.ctrl_frame, from_=100, to=500, orient="horizontal", command=self.update_params)
        self.exhaust_in_slider.set(300)
        self.exhaust_in_slider.pack(side="left", fill="x", expand=True, padx=5)

        self.fail_var = tk.BooleanVar()
        self.fail_check = ttk.Checkbutton(self.ctrl_frame, text="Simulate Sensor Failure (A1)", variable=self.fail_var, command=self.update_params)
        self.fail_check.pack(side="left", padx=5)

        ttk.Label(self.ctrl_frame, text="Ambient Temp:").pack(side="left", padx=5)
        self.ambient_slider = ttk.Scale(self.ctrl_frame, from_=-20, to=50, orient="horizontal", command=self.update_params)
        self.ambient_slider.set(20)
        self.ambient_slider.pack(side="left", fill="x", expand=True, padx=5)

        # Serial Terminal
        self.term_frame = ttk.LabelFrame(self.root, text="Serial Terminal")
        self.term_frame.pack(fill="both", expand=True, padx=10, pady=5)
        self.terminal = scrolledtext.ScrolledText(self.term_frame, height=10, state="disabled")
        self.terminal.pack(fill="both", expand=True)

    def start_sim(self):
        ino_file = self.selected_file.get()
        # Pre-process and compile (simplified version of run_tests.py logic)
        self.compile_ino(ino_file)

        try:
            self.process = subprocess.Popen(
                ["./interactive.out"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
            self.running = True
            self.start_btn.config(state="disabled")
            self.stop_btn.config(state="normal")

            # Start reader thread
            threading.Thread(target=self.read_output, daemon=True).start()
        except Exception as e:
            messagebox.showerror("Error", f"Failed to start: {e}")

    def compile_ino(self, ino_file):
        # We'll reuse the logic from run_tests.py but output to interactive.out
        # For this tool, we assume run_tests.py is available or we inline it.
        # Let's inline a simplified version.
        with open(ino_file, "r") as f: content = f.read()
        if '#include "Arduino.h"' not in content:
            content = '#include "Arduino.h"\n#include "Ticker.h"\n#include "SmoothThermistor.h"\n#include "VT100.h"\n#include "avr/wdt.h"\n#include "EEPROM.h"\n' + content

        # Patching
        if "power" not in content: content = "float power;\n" + content
        if "fan_duty" not in content: content = "float fan_duty;\n" + content
        if "pump_duty" not in content: content = "float pump_duty;\n" + content

        with open("interactive.cpp", "w") as f: f.write(content)

        subprocess.run([
            "g++", "-I", "mock_arduino",
            "mock_arduino/Arduino.cpp",
            "mock_arduino/EEPROM.cpp",
            "interactive.cpp",
            "mock_arduino/interactive_runner.cpp",
            "-o", "interactive.out"
        ], check=True)

    def stop_sim(self):
        if self.process:
            self.process.terminate()
            self.process = None
        self.running = False
        self.start_btn.config(state="normal")
        self.stop_btn.config(state="disabled")

    def read_output(self):
        while self.running and self.process:
            line = self.process.stdout.readline()
            if not line: break
            if line.startswith("{"):
                try:
                    data = json.loads(line)
                    self.root.after(0, self.update_ui, data)
                    continue
                except json.JSONDecodeError:
                    pass
            # Regular serial output
            self.root.after(0, self.append_terminal, line)

    def append_terminal(self, text):
        self.terminal.config(state="normal")
        self.terminal.insert(tk.END, text)
        self.terminal.see(tk.END)
        self.terminal.config(state="disabled")

    def update_ui(self, data):
        for key, val in data.items():
            if key in self.stats:
                self.stats[key].config(text=f"{val:.2f}" if isinstance(val, float) else str(val))

    def update_params(self, event=None):
        if self.process and self.process.stdin:
            val = self.exhaust_in_slider.get()
            self.process.stdin.write(f"exhaust_in {val}\n")

            amb = self.ambient_slider.get()
            self.process.stdin.write(f"ambient {amb}\n")

            fail = 1 if self.fail_var.get() else 0
            cmd = f"fail 1\n" if fail else f"fix 1\n"
            self.process.stdin.write(cmd)
            self.process.stdin.flush()

if __name__ == "__main__":
    root = tk.Tk()
    gui = FurnaceGUI(root)
    root.mainloop()
