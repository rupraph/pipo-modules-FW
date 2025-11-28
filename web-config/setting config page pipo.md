MOTION CONFIG

# General:

## Save/load config

## Quick settings:

### Sensor Axis:

| setting  | RX                      | RY                      | RZ                      | TX                      | TY                      | TZ                      |
| -------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- |
| enable   | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> |
| Invert   | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> |
| deadband | 5%                      | 5%                      | 5%                      | 5%                      | 5%                      | 5%                      |
| smooth   |                         |                         |                         |                         |                         |                         |

### Buttons:

- ">/II" :
  - click behaviour: <select> <option>maintain</option> <option>toggle</option> </select>
  - map to:

| Axis                      | RX                      | RY                      | RZ                      | TX                      | TY                      | TZ                      |
| ------------------------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- |
| use button                | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> |
| always on (ignore button) | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> |

- "Mode" function:
  - click behaviour: <select> <option>cycle through enabled axis</option> <option>toggle between axis lists</option> </select> // Add cycle through scales // use for position reset
  - (if toggle btw axis mode selected):

| Axis    | RX                      | RY                      | RZ                      | TX                      | TY                      | TZ                      |
| ------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- | ----------------------- |
| click 1 | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> |
| click 2 | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> | <input type="checkbox"> |

## Protocols:

- [ ] Enable HID (reboot if changed)
- [ ] Enable OSC (require wifi)
  - destination IP: 192.168.33.1
  - destination port: 5000
- [ ] Enable MIDI

## Connectivity:

- [ ] Disable wifi
- [ ] Disable BLE

## Midi modes

Axis RX:

- Mode: CC <input type="checkbox"> Note <input type="checkbox"> - (if cc): - CC nb: <select> <option>0</option> <option>1</option> <option>10</option><option>Modulation</option></select> 0 or center: <select> <option>0</option> <option>1</option> <option></option></select> HiResolution <input type="checkbox"> Sensitivity <select> <option>Linear</option> <option>Log</option> <option></option></select> Limit min value: 0% Limit max value: 100% - (if notes): - Scale quantums: <select> <option>Chromatic</option> <option>mixolidian</option></select> Base Note: <select> <option>C4</option> <option>D2</option> <option>E4</option></select> Nb of notes: 12
  Axis RY
- Mode ........

## HID modes:

- Mode: <select> <option>gamepad</option> <option>mouse</option> <option>keyboard</option></select> - (if gamepad): - [ ] Continuous or button press - analog X: <select> <option>RX</option> <option>RY</option> <option>RZ</option></select> - ....
  TO BE CONTINUED

# Event log:

.
