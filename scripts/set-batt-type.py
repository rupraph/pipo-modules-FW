"""
PlatformIO extra script: reads PIPO_BATT_TYPE env var and adds -DBATT_TYPE=X to build flags.

Usage:
  PIPO_BATT_TYPE=2 pio run -e analog_rev2

If PIPO_BATT_TYPE is not set, no flag is added and the default in HW_CONFIG.h is used.
"""

import os


def ApplyBuildFlags(env):
    batt_type = os.environ.get("PIPO_BATT_TYPE")
    if batt_type:
        env.Append(BUILD_FLAGS=[f"-DBATT_TYPE={batt_type}"])
        print(f"[set-batt-type] PIPO_BATT_TYPE={batt_type} → -DBATT_TYPE={batt_type}")


# Required PlatformIO entry point
Import("env")
ApplyBuildFlags(env)
