#!/bin/bash
echo ">>> Running full test suite and benchmarking..."
python3 run_tests.py

echo ">>> Generating performance graphs..."
python3 generate_graphs.py

echo ">>> Capturing VT100 terminal snapshots..."
python3 vt100_to_png.py furnace_vt100.ino.log docs/images/vt100_grab.png
python3 vt100_to_png.py gaming_scrubber.ino.log docs/images/gaming_vt100.png
python3 vt100_to_png.py scrubber_vt100.ino.log docs/images/scrubber_vt100.png

echo ">>> Asset generation complete."
ls -l docs/images/
