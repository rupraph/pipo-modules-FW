import shutil
import os

Import("env")
pipotype = env["PIOENV"]
build_flags = env.get("BUILD_FLAGS", [])
print(f"pipotype: {pipotype}, build_flags: {build_flags}")

toCopy = ["analog", "analog_rev2", "motion", "range"]


def copy():
    if not "buildfs" in COMMAND_LINE_TARGETS:
        print("Do not copy default config file")
        return
    if "-DPIPO_ANALOG=1" in build_flags:
        if "-DHW_REV=20" in build_flags:
            path = f"configs/analog2_config.json"
            preset_type = "analog2"
        else:
            path = f"configs/analog_config.json"
            preset_type = "analog"
    elif "-DPIPO_MOTION=1" in build_flags:
        path = f"configs/motion_config.json"
        preset_type = "motion"
    elif "-DPIPO_RANGE=1" in build_flags:
        path = f"configs/range_config.json"
        preset_type = "range"
    elif "-DPIPO_RANGE_L5CX=1" in build_flags:
        path = f"configs/range_l5cx_config.json"
        preset_type = "range"
    else:
        raise Exception("No config file for build type")

    if not os.path.exists("data"):
        os.makedirs("data")
    shutil.copyfile(path, "data/default.json")

    if os.path.exists("data/configs"):
        shutil.rmtree("data/configs")
    os.makedirs("data/configs")
    shutil.copyfile(path, "data/configs/Config-1.json")

    print(f"Copy default config file {path}")

    # Copy presets for the current build type
    preset_src = f"presets/{preset_type}"
    preset_dst = "data/presets"

    if os.path.exists(preset_src):
        if os.path.exists(preset_dst):
            shutil.rmtree(preset_dst)
        shutil.copytree(preset_src, preset_dst)
        print(f"Copy presets from {preset_src} to {preset_dst}")
    else:
        print(f"Warning: No presets folder found at {preset_src}")


copy()
