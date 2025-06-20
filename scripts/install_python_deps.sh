#!/bin/bash

echo "Installing Python dependencies for SSH key installation..."
echo

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Check if python3 is available
if command -v python3 &> /dev/null; then
    echo "Python3 found, installing dependencies..."
    python3 -m pip install -r "$SCRIPT_DIR/requirements.txt"
    if [ $? -eq 0 ]; then
        echo
        echo "[SUCCESS] Dependencies installed successfully"
        echo "You can now use the SSH key installation feature"
    else
        echo
        echo "[ERROR] Failed to install dependencies"
        echo "Please check your Python installation and try again"
        echo "You may need to install pip: sudo apt-get install python3-pip"
    fi
elif command -v python &> /dev/null; then
    echo "Python found, installing dependencies..."
    python -m pip install -r "$SCRIPT_DIR/requirements.txt"
    if [ $? -eq 0 ]; then
        echo
        echo "[SUCCESS] Dependencies installed successfully"
        echo "You can now use the SSH key installation feature"
    else
        echo
        echo "[ERROR] Failed to install dependencies"
        echo "Please check your Python installation and try again"
    fi
else
    echo "[ERROR] Python not found"
    echo "Please install Python 3.x first:"
    echo "  Ubuntu/Debian: sudo apt-get install python3 python3-pip"
    echo "  CentOS/RHEL: sudo yum install python3 python3-pip"
    echo "  macOS: brew install python3"
fi

echo
echo "Press Enter to continue..."
read 