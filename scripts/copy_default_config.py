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
        else:
            path = f"configs/analog_config.json"
    elif "-DPIPO_MOTION=1" in build_flags:
        path = f"configs/motion_config.json"
    elif "-DPIPO_RANGE=1" in build_flags:
        path = f"configs/range_config.json"
    else:
        raise Exception("No config file for build type")
    if not os.path.exists("data"):
        os.makedirs("data")
    if not os.path.exists("data/configs"):
        os.makedirs("data/configs")
    shutil.copyfile(path, "data/default.json")
    shutil.copyfile(path, "data/configs/Config-0.json")
    print(f"Copy default config file {path}")


copy()
