#!/usr/bin/env python3
"""
burn-board-identity.py — Manufacturing: burn Pipo board identity into ESP32-S3 eFuse BLOCK3.

Burns board model, hardware revision, and reserved serial space into BLOCK3 (USER_DATA).
Designed to be called from batch flashing scripts — non-interactive with --yes.

Usage:
  # Interactive (prompts for confirmation):
  python3 scripts/burn-board-identity.py --port /dev/ttyUSB0 --model motion --revision 1.2

  # Batch/non-interactive:
  python3 scripts/burn-board-identity.py --port /dev/ttyUSB0 --model range --revision 2.1 --yes

  # With serial (future):
  python3 scripts/burn-board-identity.py --port /dev/ttyUSB0 --model analog --revision 2.0 --serial 072026-001

  # Preview without burning:
  python3 scripts/burn-board-identity.py --port /dev/ttyUSB0 --model motion --revision 1.2 --dry-run

  # JSON output for batch scripts:
  python3 scripts/burn-board-identity.py --port /dev/ttyUSB0 --model motion --revision 1.2 --yes --json

BLOCK3 Layout (32 bytes, 256 bits):
  Byte 0:     board_model (uint8)  — 256 possible board types
  Byte 1:     hw_rev_major (uint8) — major revision number
  Byte 2:     hw_rev_minor (uint8) — minor revision number
  Bytes 3-18: serial (16 bytes)    — ASCII, null-padded (reserved for future)
  Bytes 19-31: reserved (13 bytes) — zeros

⚠️  BLOCK3 uses RS coding — the ENTIRE block is burned in ONE shot.
   You cannot add or modify fields later. Plan accordingly.
"""

import sys
import argparse
import subprocess
import tempfile
import os
import json

# ── Model enum (expand as needed) ─────────────────────────────
MODEL_MAP = {
    "motion": 0x01,
    "range":  0x02,
    "analog": 0x03,
    # Add future board types here, up to 255 (0xFF)
    # "motion_v2": 0x04,
    # "range_v2":   0x05,
    # "analog_v2":  0x06,
}

MODEL_NAMES = {v: k for k, v in MODEL_MAP.items()}

BLOCK_SIZE = 32      # BLOCK3 = 256 bits = 32 bytes
SERIAL_MAX = 16      # Max serial characters

# ── Layout offsets (keep in sync with src/utils/board_identity.h) ──
OFFSET_MODEL      = 0
OFFSET_REV_MAJOR  = 1
OFFSET_REV_MINOR  = 2
OFFSET_SERIAL     = 3
OFFSET_SERIAL_END = 19   # exclusive
OFFSET_RESERVED   = 19


# ═══════════════════════════════════════════════════════════════
# Validation
# ═══════════════════════════════════════════════════════════════

def parse_revision(rev_str: str):
    """Parse 'major.minor' → (major: int, minor: int)."""
    try:
        major_str, minor_str = rev_str.split(".")
        major = int(major_str)
        minor = int(minor_str)
    except (ValueError, AttributeError):
        raise argparse.ArgumentTypeError(
            f"Invalid revision '{rev_str}'. Expected format: 'major.minor' (e.g., '1.2')"
        )
    if not (0 <= major <= 255):
        raise argparse.ArgumentTypeError(f"Major revision must be 0-255, got {major}")
    if not (0 <= minor <= 255):
        raise argparse.ArgumentTypeError(f"Minor revision must be 0-255, got {minor}")
    return (major, minor)


def validate_serial(serial: str):
    """Validate serial: printable ASCII, max 16 chars."""
    if len(serial) > SERIAL_MAX:
        raise argparse.ArgumentTypeError(
            f"Serial '{serial}' is {len(serial)} chars. Max {SERIAL_MAX} chars."
        )
    if not all(0x20 <= ord(c) <= 0x7E for c in serial):
        raise argparse.ArgumentTypeError(
            "Serial must contain only printable ASCII characters."
        )
    return serial


# ═══════════════════════════════════════════════════════════════
# Packing
# ═══════════════════════════════════════════════════════════════

def pack_block(model_id: int, rev_major: int, rev_minor: int, serial: str) -> bytes:
    """Pack identity into a 32-byte BLOCK3 payload."""
    data = bytearray(BLOCK_SIZE)

    # Header
    data[OFFSET_MODEL]     = model_id
    data[OFFSET_REV_MAJOR] = rev_major
    data[OFFSET_REV_MINOR] = rev_minor

    # Serial (ASCII, null-padded)
    serial_bytes = serial.encode("ascii")
    serial_len = min(len(serial_bytes), SERIAL_MAX)
    data[OFFSET_SERIAL:OFFSET_SERIAL + serial_len] = serial_bytes[:serial_len]
    # Remaining serial bytes + reserved are already zero from bytearray()

    return bytes(data)


# ═══════════════════════════════════════════════════════════════
# eFuse burning
# ═══════════════════════════════════════════════════════════════

def burn_block3(port: str, data: bytes, dry_run: bool = False) -> dict:
    """
    Burn data to BLOCK3 via espefuse.py burn_block_data.
    Returns a result dict with 'success', 'stdout', 'stderr'.
    """
    result = {"success": False, "stdout": "", "stderr": ""}

    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
        f.write(data)
        tmp_path = f.name

    try:
        cmd = [
            "espefuse.py",
            "--chip", "esp32s3",
            "--port", port,
            "--do-not-confirm",
            "burn-block-data",
            "BLOCK3",
            tmp_path,
        ]

        if dry_run:
            result["success"] = True
            result["stdout"] = f"DRY RUN: {' '.join(cmd)}"
            return result

        proc = subprocess.run(cmd, check=True, capture_output=True, text=True)
        result["success"] = True
        result["stdout"] = proc.stdout
        result["stderr"] = proc.stderr

    except subprocess.CalledProcessError as e:
        result["stdout"] = e.stdout or ""
        result["stderr"] = e.stderr or ""
    finally:
        os.unlink(tmp_path)

    return result


# ═══════════════════════════════════════════════════════════════
# Pre-burn / Post-burn verification
# ═══════════════════════════════════════════════════════════════

def read_block3_raw(port: str) -> bytes | None:
    """
    Read current raw BLOCK3 contents via 'espefuse.py dump'.
    Returns 32 bytes of raw data, or None on failure.
    """
    try:
        proc = subprocess.run(
            ["espefuse.py", "--chip", "esp32s3", "--port", port,
             "--do-not-confirm", "dump"],
            check=True, capture_output=True, text=True
        )
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        return None

    # Parse dump output: "BLOCK3          = 00 00 00 00 ... 00 00 00 00"
    for line in proc.stdout.splitlines():
        stripped = line.strip()
        if stripped.startswith("BLOCK3") and "=" in stripped:
            try:
                hex_part = stripped.split("=", 1)[1].strip()
                # Remove any trailing commentary after the hex bytes
                hex_tokens = hex_part.split()
                hex_bytes = [t for t in hex_tokens if len(t) == 2 and
                             all(c in "0123456789ABCDEFabcdef" for c in t)]
                raw = bytes(int(b, 16) for b in hex_bytes)
                if len(raw) == 32:
                    return raw
            except (ValueError, IndexError):
                pass
    return None


def is_block3_empty(raw: bytes) -> bool:
    """Check if BLOCK3 appears unprogrammed (all bytes 0x00)."""
    return all(b == 0x00 for b in raw)


# ═══════════════════════════════════════════════════════════════
# Main
# ═══════════════════════════════════════════════════════════════

def main():
    parser = argparse.ArgumentParser(
        description="Burn Pipo board identity into ESP32-S3 eFuse BLOCK3 (USER_DATA).",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --port /dev/ttyUSB0 --model motion --revision 1.2
  %(prog)s --port COM3 --model range --revision 2.1 --yes
  %(prog)s --port /dev/ttyUSB0 --model analog --revision 2.0 --serial 072026-001 --yes
  %(prog)s --port /dev/ttyUSB0 --model motion --revision 1.2 --dry-run
        """,
    )

    parser.add_argument(
        "--port", "-p", required=True,
        help="Serial port (e.g., /dev/ttyUSB0, COM3)"
    )
    parser.add_argument(
        "--model", "-m", required=True,
        choices=list(MODEL_MAP.keys()),
        help="Board model"
    )
    parser.add_argument(
        "--revision", "-r", required=True,
        type=parse_revision,
        help="Hardware revision as major.minor (e.g., '1.2')"
    )
    parser.add_argument(
        "--serial", "-s", type=validate_serial, default="",
        help=f"Serial number (max {SERIAL_MAX} ASCII chars, e.g., '072026-001'). "
             "Omit to leave serial area blank (zeros)."
    )
    parser.add_argument(
        "--yes", "-y", action="store_true",
        help="Skip confirmation prompt (for batch/non-interactive use)"
    )
    parser.add_argument(
        "--dry-run", "-n", action="store_true",
        help="Preview without burning"
    )
    parser.add_argument(
        "--force", "-f", action="store_true",
        help="Skip pre-burn check (allow re-burning an already-programmed board). "
             "DANGEROUS: RS-coded blocks cannot be meaningfully updated."
    )
    parser.add_argument(
        "--json", action="store_true",
        help="Output result as JSON (for batch script consumption)"
    )

    args = parser.parse_args()
    rev_major, rev_minor = args.revision
    model_id = MODEL_MAP[args.model]

    # ── Preview ───────────────────────────────────────────────
    serial_display = args.serial if args.serial else "(blank — reserved for future)"
    espefuse_port = args.port

    if not args.json:
        print("╔══════════════════════════════════════════╗")
        print("║    Pipo Board Identity Burner — BLOCK3   ║")
        print("╠══════════════════════════════════════════╣")
        print(f"║ Port:      {espefuse_port}")
        print(f"║ Model:     {args.model} (id=0x{model_id:02X})")
        print(f"║ Revision:  {rev_major}.{rev_minor}")
        print(f"║ Serial:    {serial_display}")
        print(f"║ Target:    BLOCK3 (USER_DATA)")
        print(f"║ ⚠️  RS coding: one-shot, cannot re-burn")
        print("╚══════════════════════════════════════════╝")

    # ── Pack data ─────────────────────────────────────────────
    data = pack_block(model_id, rev_major, rev_minor, args.serial)

    if args.dry_run:
        if args.json:
            print(json.dumps({
                "status": "dry_run",
                "model": args.model,
                "model_id": model_id,
                "revision": f"{rev_major}.{rev_minor}",
                "serial": args.serial or None,
                "block": "BLOCK3",
                "data_hex": data.hex(),
                "data_bytes": list(data),
            }))
        else:
            print(f"\n🔍 DRY RUN — No eFuse burned.")
            print(f"   Data: {data.hex(' ')}")
            print(f"   Layout:")
            print(f"     [{OFFSET_MODEL}] model     = 0x{data[OFFSET_MODEL]:02X} ({args.model})")
            print(f"     [{OFFSET_REV_MAJOR}] rev_major = 0x{data[OFFSET_REV_MAJOR]:02X} ({rev_major})")
            print(f"     [{OFFSET_REV_MINOR}] rev_minor = 0x{data[OFFSET_REV_MINOR]:02X} ({rev_minor})")
            serial_bytes = data[OFFSET_SERIAL:OFFSET_SERIAL_END]
            print(f"     [{OFFSET_SERIAL}-{OFFSET_SERIAL_END-1}] serial    = {serial_bytes.hex(' ')} ({serial_bytes.decode('ascii').rstrip(chr(0)) or '(empty)'})")
            print(f"     [{OFFSET_RESERVED}-31] reserved  = zeros")
        return 0

    # ── Confirmation ──────────────────────────────────────────
    if not args.yes:
        print("\n⚠️  WARNING: This will PERMANENTLY burn BLOCK3.")
        print("   This operation CANNOT be undone.")
        print("   The block can never be written to again (RS coding).")
        print()
        try:
            response = input("   Type 'BURN' to confirm: ")
        except (EOFError, KeyboardInterrupt):
            print("\n   Aborted.")
            return 1
        if response.strip() != "BURN":
            print("   Aborted.")
            return 1

    # ── Pre-burn safety check ──────────────────────────────────
    if not args.dry_run and not args.force:
        if not args.json:
            print("\n🔍 Reading current BLOCK3 state...")
        existing = read_block3_raw(espefuse_port)
        if existing is None:
            msg = "Failed to read BLOCK3. Check connection and that espefuse.py is in PATH."
            if args.json:
                print(json.dumps({"status": "error", "error": msg}))
            else:
                print(f"❌ {msg}", file=sys.stderr)
            return 1
        if not is_block3_empty(existing):
            if args.json:
                print(json.dumps({
                    "status": "error",
                    "error": "BLOCK3 already programmed",
                    "existing_hex": existing.hex(),
                    "hint": "Use --force to override (not recommended for RS-coded blocks)",
                }))
            else:
                print("❌ BLOCK3 already programmed! Refusing to overwrite.")
                print(f"   Existing data: {existing.hex(' ')}")
                print("   Use --force to override (not recommended for RS-coded blocks).")
            return 1
        if not args.json:
            print("   ✅ BLOCK3 is empty — safe to proceed.")

    # ── Burn ──────────────────────────────────────────────────
    print(f"\n🔥 Burning BLOCK3...")
    result = burn_block3(espefuse_port, data)

    if args.json:
        output = {
            "status": "ok" if result["success"] else "error",
            "model": args.model,
            "model_id": model_id,
            "revision": f"{rev_major}.{rev_minor}",
            "serial": args.serial or None,
            "block": "BLOCK3",
            "data_hex": data.hex(),
        }
        if not result["success"]:
            output["error_stdout"] = result["stdout"]
            output["error_stderr"] = result["stderr"]
        print(json.dumps(output))
    else:
        if result["stdout"]:
            print(result["stdout"])
        if result["stderr"]:
            print(result["stderr"], file=sys.stderr)

    if result["success"]:
        # ── Post-burn verification ────────────────────────────
        if not args.dry_run:
            if not args.json:
                print("\n🔍 Verifying burned data...")
            burned = read_block3_raw(espefuse_port)
            if burned is None:
                if not args.json:
                    print("⚠️  Could not read back BLOCK3 for verification.", file=sys.stderr)
            elif burned == data:
                if not args.json:
                    print("   ✅ Verification passed — data matches.")
            else:
                if args.json:
                    print(json.dumps({
                        "status": "error",
                        "error": "Post-burn verification failed — data mismatch",
                        "expected_hex": data.hex(),
                        "readback_hex": burned.hex(),
                    }))
                else:
                    print("❌ Verification FAILED — data mismatch!", file=sys.stderr)
                    print(f"   Expected: {data.hex(' ')}")
                    print(f"   Read:     {burned.hex(' ')}")
                return 1

        if not args.json:
            print("✅ Board identity burned successfully.")
            print(f"   Verify: espefuse.py --port {espefuse_port} summary")
        return 0
    else:
        if not args.json:
            print("❌ eFuse burn FAILED.", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
