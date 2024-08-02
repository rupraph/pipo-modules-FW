#!/bin/bash
wget -O get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
rm get-platformio.py
cp ~/.platformio/penv/bin/platformio ~/.local/bin/pio