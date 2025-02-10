This repo contains the code for pipo motion, analog, and range with the UI.

# The last stable and tested version of the code is in branch "main"

# Minimal Quick Start guide (linux/mac)

To run the project, you need:

- VScode with PlatformIO extension
- install the espressif32 platform in PIO
- optionally, install npm (to compile the web UI, and be able to use the compile/upload scripts)
- optionally install vite in the web-config folder(for UI):`npm install vite --save-dev`

The project is configured to deal with seprate environments for each module so don't use the usual PIO build/upload buttons at the bootom of vscode, these will likely not work.

To compile/upload, there are 2 options:

- using npm scripts from vs code terminal (which are just shortcuts for pio cli commands, see package.json)
- directly use pio commands from the pio cli

Use module build flags when necessary: "motion", "analog", "range"

Compilation/upload Steps for motion as an e.g:
(optional) 
- rebuild UI web page files: `npm run build:web`

- build module filesystem: `npm run build:fs:motion`
- build firmware: `npm run build:motion`
- put the module in receive mode: maintain boot clicked while clicking reset, then release boot.
- upload firmware: `npm run upload:motion`
- upload filsystem: `npm run upload:fs`
- hit reset on the module.
  (Other combinations of scripts command are possible, this is the most detailed one)
