This repo contains the code for Pipo _motion_, _analog_, and _range_ with the UI.

> [!IMPORTANT]
> The last stable and tested version of the code is in branch `main`. Ongoing developements in `develop`
>
Discord Server: ZMVstP7zh9

# Minimal Quick Start guide (linux/mac)

## Embedded software

To setup the environment, you need:

- VSCode with PlatformIO extension
- Install the `espressif32` platform in PIO
- install `node.js` (and `npm`)

The project is configured to deal with separate environments for each module so don't use the usual PIO build/upload buttons at the bottom of vscode, these will likely not work.

To compile/upload, use the `npm` commands from vs code terminal (which are just shortcuts for pio CLI commands, see `package.json`)
For eg, you can run: `npm run full:motion_rev1_1` to build and upload the Firware to a Pipo motion.

Use module build flags as necessary: "motion_rev1_1", "analog_rev1_1", "range_rev1_1"

## UI

The web UI is built with svelte.

- From the `web-config` directory, install the dependencies: `npm install`
- Run development or build commands (see `package.json`)

## Example

Compilation/upload Steps for "motion":

- [optional] Rebuild UI web page files: `npm run build:web`
- Build module filesystem: `npm run build:fs:motion_rev1_1`
- Build firmware: `npm run build:motion_rev1_1`
- Put the module in receive mode: maintain boot clicked while clicking reset once, then release boot.
- Upload firmware: `npm run upload:motion_rev1_1`
- Upload filesystem: `npm run upload:fs_rev1_1`
- Hit reset on the module.

You can also rebuild/upload everything at once: 'npm run full:motion_rev1_1'.

## Roadmap

There are plans to make this project more flexible to be easily portable and usable on any common esp32-s3 dev-board to build various types of sensing devices. If you are interested to contribute to this effort, let me know !
