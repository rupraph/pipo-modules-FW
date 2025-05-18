import subprocess
import os
import json
import shutil

# This should be run from platformio terminal

ENVS = ["motion", "motion_rev1_1", "range", "range_rev1_1", "analog", "analog_rev2"]


def get_latest_tag():
    try:
        tag = subprocess.check_output(
            ["git", "describe", "--tags"], encoding="utf-8"
        ).strip()
        return tag
    except subprocess.CalledProcessError:
        return "untagged"


def write_manifest(tag, env):
    tag_only = tag.split("-")[0]
    manifest = {"name": f"Pipo {env}", "version": tag_only, "builds": []}
    manifest["builds"].append(
        {
            "chipFamily": "ESP32-S3",
            "parts": [{"path": f"merged_{env}_{tag}.bin", "offset": 0}],
        }
    )
    with open(f".pio/releases/{tag_only}/{env}/manifest.json", "w") as f:
        json.dump(manifest, f, indent=2)


def run(cmd):
    print(f"Running: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)


def build_env(env):
    run(["pio", "run", "-e", env])
    run(["pio", "run", "-t", "buildfs", "-e", env])


def merge_env(env):
    build_dir = f".pio/build/{env}"
    bootloader = f"{build_dir}/bootloader.bin"
    partitions = f"{build_dir}/partitions.bin"
    boot_app0 = os.path.expanduser(
        "~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
    )
    firmware = f"{build_dir}/firmware.bin"
    littlefs = f"{build_dir}/littlefs.bin"
    # get tag only from the full version
    tag_only = tag.split("-")[0]
    merged_dir = f".pio/releases/{tag_only}/{env}"
    os.makedirs(merged_dir, exist_ok=True)
    merged = f".pio/releases/{tag_only}/{env}/merged_{env}_{tag}.bin"
    run(
        [
            "esptool.py",
            "--chip",
            "esp32",
            "merge_bin",
            "-o",
            merged,
            "--flash_mode",
            "dio",
            "--flash_freq",
            "40m",
            "--flash_size",
            "8MB",
            "0x0000",
            bootloader,
            "0x8000",
            partitions,
            "0xe000",
            boot_app0,
            "0x10000",
            firmware,
            "0x410000",
            littlefs,
        ]
    )


if __name__ == "__main__":
    try:
        tag = get_latest_tag()
        tag_only = tag.split("-")[0]
        run(["npm", "run", f"build:web"])
        results = {}
        for env in ENVS:
            build_env(env)
            merge_env(env)
            write_manifest(tag, env)
        src_dir = f".pio/releases/{tag_only}"
        dest_dir = os.path.abspath(
            f"/Users/rup/Documents/GitHub/pipo-site-vuepress/src/.vuepress/public/assets/releases/{tag_only}"
        )
        if os.path.exists(dest_dir):
            shutil.rmtree(dest_dir)
        shutil.copytree(src_dir, dest_dir)
        print(f"Copied {src_dir} to {dest_dir}")
    except subprocess.CalledProcessError as e:
        print(f"ERROR: Command failed {e.cmd} Return code: {e.returncode}")
        exit(1)
    except Exception as e:
        print(f"ERROR: {e}")
        exit(1)

    print("Release process completed successfully.")
