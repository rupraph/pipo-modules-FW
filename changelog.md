# 0.0.4

    ## Highlights

    - Integrated Wi‑Fi management in the UI — simultaneous AP + STA
    - Added setting to invert ranges
    - Added setting to make an input cyclic
    - Improved filtering
    - Added Arpeggio and Interval MIDI Note output modes
    - Multicore improvements: begin distributing workload across cores

    ## Pipo Motion
    - Access to magnetometer channels
    - Boot in absolute or relative orientation
    - Quaternion output available over OSC

    ## Pipo Analog (beta)
    - Analog Output mode
        - Toggle pin for input or output
        - Receive OSC data
        - Output as servo or PWM

    ## Notes
    - BLE output temporarily disabled in this release.

# 0.0.5

    - Bring back BLE
    - Solves main heap issue
    - General improvements
    - UI serving optimizations
    - Made compatible for Motion/Range 1.1 and Analog 2.0

# 0.0.6

    ## Highlights
    - New UI style
    - Choose between global Midi or OSC mode
    - Improve Motion relative mode
    - General debug and improvements
    - Add note velocity
    - add play/pause

# 0.0.7

    ## This version was mostly about backend robustness improvements
    ## Working good but not considered a full release since si only a step for 0.0.8 that will undergo thorough testing as major release.

    ## Highlights
    - Quaternion output filtering: send OSC data only when values change
    - Quaternion mode: switch between relative and absolute orientation output
    - Robust configuration management with atomic writes and validation
    - Battery monitoring improvements: OSC output, low battery shutdown, plugged detection
    - Enhanced logging system with detailed debug information
    - Task watchdogs for main loop monitoring and overrun detection
    - Network management: forget network feature, improved AP mode
    - UI persistence: save and restore UI state across sessions
    - Configuration presets system

    ## Pipo Motion
    - Persist relative orientation reference across reboots (stored in NVS)
    - Added gyroscope and accelerometer channel access
    - Button callbacks for setting reference orientation and toggling relative mode

    ## Pipo Range
    - Added "over_out" feature: return output minimum when sensor exceeds maximum
    - Improved offset calibration and boot offset measurement
    - Fixed data jump issues in note trigger mode

    ## OSC Improvements
    - Improved OSC lifecycle management (start/stop/cleanup)
    - Custom OSC address prefixes removed/simplified
    - UDP start only when OSC is enabled
    - Battery level sent over OSC

    ## MIDI Improvements
    - Fixed axis MIDI note crosstalk
    - Fixed instant note-off issue in threshold mode
    - USB and BLE device herit names from configuration

    ## WiFi/Network
    - Robust WiFi event callback handling
    - Improved disconnect handling
    - Password form improvements

    ## System
    - Pulldown all unused pins at setup to avoid floating
    - Improved heap management and reduced fragmentation
    - Multicore task distribution improvements
    - ESP32 watchdog integration with configurable timeouts
    - Cleanup temporary files on boot
    - Better reset reason reporting
