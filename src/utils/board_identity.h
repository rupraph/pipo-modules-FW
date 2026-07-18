/**
 * @file board_identity.h
 * @brief Read Pipo board identity from ESP32-S3 eFuse BLOCK3 (USER_DATA).
 *
 * Layout (32 bytes, must match scripts/burn-board-identity.py):
 *   Byte 0:     board_model   (uint8) — 256 possible board types
 *   Byte 1:     hw_rev_major  (uint8) — major revision
 *   Byte 2:     hw_rev_minor  (uint8) — minor revision
 *   Bytes 3-18: serial        (16 bytes, ASCII, null-padded)
 *   Bytes 19-31: reserved     (13 bytes, zeros)
 */

#pragma once

#include "esp_efuse.h"
#include <cstring>

// ── Model enum ──────────────────────────────────────────────
// Keep in sync with MODEL_MAP in scripts/burn-board-identity.py

enum class BoardModel : uint8_t {
  UNKNOWN = 0x00,
  MOTION = 0x01,
  RANGE = 0x02,
  ANALOG = 0x03,
  // Future: MOTION_V2 = 0x04, RANGE_V2 = 0x05, ANALOG_V2 = 0x06, ...
};

// ── Identity struct ─────────────────────────────────────────

struct BoardIdentity {
  BoardModel model;
  uint8_t rev_major;  // e.g., 1 for v1.x
  uint8_t rev_minor;  // e.g., 2 for v1.2
  char serial[17];    // null-terminated, max 16 chars
  bool programmed;    // false if BLOCK3 is unprogrammed (all 0xFF or all 0x00)

  /// Human-readable model name
  const char* model_str() const {
    switch (model) {
      case BoardModel::MOTION:
        return "motion";
      case BoardModel::RANGE:
        return "range";
      case BoardModel::ANALOG:
        return "analog";
      default:
        return "unknown";
    }
  }

  /// Revision as "major.minor" string (uses internal static buffer)
  const char* rev_str() const {
    static char buf[16];
    snprintf(buf, sizeof(buf), "%u.%u", rev_major, rev_minor);
    return buf;
  }

  /// Full identity string for logging: "motion rev1.2 s/n:072026-001"
  const char* to_string() const {
    static char buf[64];
    if (!programmed) {
      return "unprogrammed";
    }
    if (serial[0] != '\0' && serial[0] != 0xFF) {
      snprintf(buf, sizeof(buf), "%s rev%s s/n:%s", model_str(), rev_str(),
               serial);
    } else {
      snprintf(buf, sizeof(buf), "%s rev%s", model_str(), rev_str());
    }
    return buf;
  }

  /// Check if this is a specific compile-time type (for safety validation)
  bool is_model(BoardModel expected) const {
    return programmed && model == expected;
  }
};

// ── Reader ──────────────────────────────────────────────────

/**
 * Read board identity from eFuse BLOCK3.
 *
 * Returns BoardIdentity with programmed=false if BLOCK3 is unprogrammed
 * (all bytes 0xFF or all 0x00).
 *
 * Safe to call early in setup() — uses esp_efuse_read_reg() which has no
 * dependencies on WiFi, filesystem, or RTOS.
 */
inline BoardIdentity read_board_identity() {
  BoardIdentity id = {};
  id.programmed = false;

  // Read all 8 x 32-bit registers of BLOCK3
  uint8_t raw[32] = {};
  for (int i = 0; i < 8; i++) {
    uint32_t reg = esp_efuse_read_reg(EFUSE_BLK3, i);
    // ESP32-S3 eFuse registers are little-endian
    raw[i * 4 + 0] = (reg >> 0) & 0xFF;
    raw[i * 4 + 1] = (reg >> 8) & 0xFF;
    raw[i * 4 + 2] = (reg >> 16) & 0xFF;
    raw[i * 4 + 3] = (reg >> 24) & 0xFF;
  }

  // Detect unprogrammed state:
  // - All 0xFF = never burned (eFuse default)
  // - All 0x00 = explicitly cleared
  bool all_ff = true;
  bool all_00 = true;
  for (int i = 0; i < 4; i++) {  // Check header bytes only
    if (raw[i] != 0xFF)
      all_ff = false;
    if (raw[i] != 0x00)
      all_00 = false;
  }
  if (all_ff || all_00) {
    return id;  // programmed stays false
  }

  // Parse fields
  id.model = static_cast<BoardModel>(raw[0]);
  id.rev_major = raw[1];
  id.rev_minor = raw[2];
  memcpy(id.serial, &raw[3], 16);
  id.serial[16] = '\0';
  id.programmed = true;

  return id;
}
