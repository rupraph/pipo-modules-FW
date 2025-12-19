import shutil

Import("env")
import subprocess


def getVersion():
    try:
        # Get the most recent tag reachable from HEAD
        tag = (
            subprocess.check_output(
                ["git", "describe", "--tags", "--abbrev=0"], stderr=subprocess.DEVNULL
            )
            .decode()
            .strip()
        )
        return tag
    except subprocess.CalledProcessError:
        # No tags found, fall back to commit hash
        head = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode().strip()
        return head[:10]


version = getVersion()
print("Pipo software version: ", version)
env.Append(CPPDEFINES=[f'PIPO_FW_VERSION=\\"{version}\\"'])
