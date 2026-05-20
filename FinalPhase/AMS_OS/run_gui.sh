#!/bin/bash
# AMS OS — GUI Launcher
# Builds the project (if needed) and starts in GUI mode.
# Usage: ./run_gui.sh [RAM_GB] [HDD_GB] [CORES]
#   e.g. ./run_gui.sh 2 256 8

set -e

# Ensure emoji fonts are available
if ! fc-list | grep -qi "emoji" 2>/dev/null; then
    echo "[AMS OS] Installing emoji font for GUI icons..."
    sudo apt-get install -y fonts-noto-color-emoji 2>/dev/null || true
fi

# Ensure GTK3 dev headers are available
if ! pkg-config --exists gtk+-3.0 2>/dev/null; then
    echo "[AMS OS] Installing GTK3 development headers..."
    sudo apt-get install -y libgtk-3-dev 2>/dev/null || {
        echo "ERROR: Could not install libgtk-3-dev. Please install it manually."
        exit 1
    }
fi

# Build everything
echo "[AMS OS] Building project..."
make all

# Launch GUI
echo "[AMS OS] Starting GUI desktop..."
./build/ams_desktop "${@:-2 256 8}"
