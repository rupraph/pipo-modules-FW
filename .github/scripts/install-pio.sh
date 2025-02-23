#!/bin/bash

echo "Installing Python Wheel ..."
pip install wheel > /dev/null 2>&1

echo "Installing PlatformIO ..."
pip install -U platformio > /dev/null 2>&1

echo "PlatformIO has been installed"
echo ""

echo "PlatformIO CLI Version:"
platformio --version

platformio platform install espressif32
platformio run -e motion