This repo contains the code for Pipo _motion_, _analog_, and _range_ with the UI.

# Minimal Quick Start guide

## Embedded software

To run the project, you need:

- VSCode with PlatformIO extension
- Install the `espressif32` platform in PIO

The project is configured to deal with separate environments for each module so don't use the usual PIO build/upload buttons at the bottom of vscode, these will likely not work.

To compile/upload, there are 2 options:

- [optional] Using `npm` commands from vs code terminal (which are just shortcuts for pio CLI commands, see `package.json`)
- Directly use pio commands from the pio CLI

Use module build flags when necessary: "motion", "analog", "range"

## UI

The UI is built with web technologies so that it can run in any browser.
To develop or compile the UI, install `node` and `npm`.

- From the `web-config` directory, install the dependencies: `npm install`
- Run development or build commands (see `package.json`)

## Example

Compilation/upload Steps for "motion":

- [optional] Rebuild UI web page files: `npm run build:web`
- Build module filesystem: `npm run build:fs:motion`
- Build firmware: `npm run build:motion`
- Put the module in receive mode: maintain boot clicked while clicking reset, then release boot.
- Upload firmware: `npm run upload:motion`
- Upload filesystem: `npm run upload:fs`
- Hit reset on the module.

(Other combinations of scripts command are possible, this is the most detailed one)
