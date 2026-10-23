#ifndef BLE_MIDI_CODEC_H
#define BLE_MIDI_CODEC_H

// BLE-MIDI 1.0 packet encoding/decoding and the outbound message queue.
// Pure C++11, no Arduino/NimBLE dependency so it can be unit tested natively
// (test/test_ble_midi). The NimBLE glue lives in midiBLE.cpp.
// See docs/ble-midi.md for how the whole layer works.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Largest packet we build: preferred ATT MTU (128) minus the 3-byte ATT header.
static const size_t kBleMidiMaxPacket = 125;

struct BleMidiMsg {
  uint8_t status;  // channel voice status byte (0x80-0xEF)
  uint8_t d1;
  uint8_t d2;  // unused for 2-byte messages
};

// Length in bytes of a channel voice message including its status byte.
inline uint8_t bleMidiMsgLen(uint8_t status) {
  uint8_t type = status & 0xF0;
  return (type == 0xC0 || type == 0xD0) ? 2 : 3;
}

// Encodes msgs into one BLE-MIDI packet: header byte, then for each message a
// timestamp byte followed by the full message (no running status, the most
// compatible form). All messages share the same 13-bit millisecond timestamp.
// Returns the number of bytes written, or 0 if n == 0 or it does not fit.
inline size_t bleMidiEncode(const BleMidiMsg* msgs, size_t n, uint16_t timeMs,
                            uint8_t* out, size_t cap) {
  size_t need = 1;
  for (size_t i = 0; i < n; i++) need += 1 + bleMidiMsgLen(msgs[i].status);
  if (n == 0 || need > cap) return 0;

  const uint8_t ts = 0x80 | (timeMs & 0x7F);
  size_t w = 0;
  out[w++] = 0x80 | ((timeMs >> 7) & 0x3F);
  for (size_t i = 0; i < n; i++) {
    out[w++] = ts;
    out[w++] = msgs[i].status;
    out[w++] = msgs[i].d1;
    if (bleMidiMsgLen(msgs[i].status) == 3) out[w++] = msgs[i].d2;
  }
  return w;
}

// Parses one BLE-MIDI packet and calls onMessage(status, d1, d2) for every
// complete channel voice message (d2 is 0 for 2-byte messages).
// Never reads past len. Handles running status (with or without timestamp),
// skips real-time bytes without losing running status, skips SysEx (including
// SysEx continuation packets) and system common messages. Malformed or
// truncated messages are dropped; parsing resumes at the next timestamp.
template <class F>
inline void bleMidiParse(const uint8_t* data, size_t len, F onMessage) {
  if (data == nullptr || len < 2 || !(data[0] & 0x80)) return;

  size_t i = 1;
  uint8_t running = 0;
  bool sysex = false;

  while (i < len) {
    uint8_t b = data[i];
    if (b & 0x80) {  // timestamp byte, followed by a status or a data byte
      if (++i >= len) return;
      b = data[i];
      if (b & 0x80) {
        i++;
        if (b >= 0xF8) continue;  // real-time: no effect on running status
        if (b == 0xF0) {
          sysex = true;
          running = 0;
          continue;
        }
        sysex = false;
        if (b >= 0xF0) {  // system common (incl. end of SysEx): skip its data
          running = 0;
          int skip = (b == 0xF2) ? 2 : (b == 0xF1 || b == 0xF3) ? 1 : 0;
          while (skip-- > 0 && i < len && !(data[i] & 0x80)) i++;
          continue;
        }
        running = b;
        continue;
      }
    }

    // data byte at i
    if (sysex || running == 0) {
      i++;
      continue;
    }
    const size_t need = bleMidiMsgLen(running) - 1;
    if (i + need > len) return;  // truncated
    if (need == 2 && (data[i + 1] & 0x80)) {  // cut short by a timestamp
      i++;
      continue;
    }
    onMessage(running, data[i], need == 2 ? data[i + 1] : 0);
    i += need;
  }
}

// Outbound queue between the producers (engine) and the BLE sender task.
// - Continuous messages (CC, pitch bend, aftertouch) are coalesced: only the
//   latest value per (status, controller) is kept, so a slow link never
//   builds a backlog of stale values.
// - Other messages (notes, program change) are kept in order in a FIFO.
// - When the FIFO is full: a Note Off evicts the oldest queued Note On (so
//   notes can't hang), anything else new is dropped.
// Not thread safe: the caller provides the lock.
class BleMidiOutQueue {
 public:
  enum { kOrderedCap = 32, kLatestCap = 32 };

  struct Stats {
    uint32_t droppedOrdered;  // notes/PC dropped because the FIFO was full
    uint32_t droppedLatest;   // CC/PB keys dropped because all slots were busy
    uint32_t evictedNoteOn;   // Note Ons evicted to make room for a Note Off
  };

  BleMidiOutQueue() {
    clear();
    resetStats();
  }

  // Returns false if the message was dropped.
  bool push(uint8_t status, uint8_t d1, uint8_t d2) {
    return isLatestValue(status) ? pushLatest(status, d1, d2)
                                 : pushOrdered(status, d1, d2);
  }

  // Moves messages out of the queue into out[], ordered messages first, then
  // latest values (round-robin so a busy controller can't starve others).
  // maxBytes is the packet budget including the header byte; each message
  // costs its length plus one timestamp byte.
  size_t takeBatch(BleMidiMsg* out, size_t maxMsgs, size_t maxBytes) {
    size_t n = 0;
    size_t bytes = 1;

    size_t taken = 0;
    while (taken < orderedCount_ && n < maxMsgs) {
      size_t cost = 1 + bleMidiMsgLen(ordered_[taken].status);
      if (bytes + cost > maxBytes) break;
      out[n++] = ordered_[taken++];
      bytes += cost;
    }
    removeOrdered(0, taken);
    if (orderedCount_ > 0) return n;

    const size_t start = rr_;
    for (size_t k = 0; k < kLatestCap && usedSlots_ > 0 && n < maxMsgs; k++) {
      size_t idx = (start + k) % kLatestCap;
      Slot& s = slots_[idx];
      if (!s.used) continue;
      size_t cost = 1 + bleMidiMsgLen(s.status);
      if (bytes + cost > maxBytes) break;
      BleMidiMsg m = {s.status, s.d1, s.d2};
      out[n++] = m;
      bytes += cost;
      s.used = false;
      usedSlots_--;
      rr_ = (idx + 1) % kLatestCap;
    }
    return n;
  }

  void clear() {
    orderedCount_ = 0;
    usedSlots_ = 0;
    rr_ = 0;
    for (size_t i = 0; i < kLatestCap; i++) slots_[i].used = false;
  }

  bool empty() const { return orderedCount_ == 0 && usedSlots_ == 0; }
  const Stats& stats() const { return stats_; }
  void resetStats() { memset(&stats_, 0, sizeof(stats_)); }

 private:
  struct Slot {
    uint8_t status, key, d1, d2;
    bool used;
  };

  static bool isLatestValue(uint8_t status) {
    uint8_t type = status & 0xF0;
    return type == 0xA0 || type == 0xB0 || type == 0xD0 || type == 0xE0;
  }
  static bool isNoteOff(const BleMidiMsg& m) {
    uint8_t type = m.status & 0xF0;
    return type == 0x80 || (type == 0x90 && m.d2 == 0);
  }
  static bool isNoteOn(const BleMidiMsg& m) {
    return (m.status & 0xF0) == 0x90 && m.d2 > 0;
  }
  static bool sameNote(const BleMidiMsg& a, const BleMidiMsg& b) {
    uint8_t ta = a.status & 0xF0, tb = b.status & 0xF0;
    return (ta == 0x80 || ta == 0x90) && (tb == 0x80 || tb == 0x90) &&
           (a.status & 0x0F) == (b.status & 0x0F) && a.d1 == b.d1;
  }

  bool pushLatest(uint8_t status, uint8_t d1, uint8_t d2) {
    uint8_t type = status & 0xF0;
    uint8_t key = (type == 0xA0 || type == 0xB0) ? d1 : 0;
    Slot* free = nullptr;
    for (size_t i = 0; i < kLatestCap; i++) {
      Slot& s = slots_[i];
      if (s.used && s.status == status && s.key == key) {
        s.d1 = d1;
        s.d2 = d2;
        return true;
      }
      if (!s.used && free == nullptr) free = &s;
    }
    if (free == nullptr) {
      stats_.droppedLatest++;
      return false;
    }
    free->status = status;
    free->key = key;
    free->d1 = d1;
    free->d2 = d2;
    free->used = true;
    usedSlots_++;
    return true;
  }

  bool pushOrdered(uint8_t status, uint8_t d1, uint8_t d2) {
    BleMidiMsg m = {status, d1, d2};
    const bool off = isNoteOff(m);

    if (off) {  // skip if the last queued event for this note is already an off
      for (size_t i = orderedCount_; i-- > 0;) {
        if (sameNote(ordered_[i], m)) {
          if (isNoteOff(ordered_[i])) return true;
          break;
        }
      }
    }

    if (orderedCount_ < kOrderedCap) {
      ordered_[orderedCount_++] = m;
      return true;
    }

    if (off) {
      for (size_t i = 0; i < orderedCount_; i++) {
        if (isNoteOn(ordered_[i])) {
          removeOrdered(i, 1);
          stats_.evictedNoteOn++;
          ordered_[orderedCount_++] = m;
          return true;
        }
      }
    }

    stats_.droppedOrdered++;
    return false;
  }

  void removeOrdered(size_t at, size_t count) {
    if (count == 0) return;
    memmove(&ordered_[at], &ordered_[at + count],
            (orderedCount_ - at - count) * sizeof(BleMidiMsg));
    orderedCount_ -= count;
  }

  BleMidiMsg ordered_[kOrderedCap];
  size_t orderedCount_;
  Slot slots_[kLatestCap];
  size_t usedSlots_;
  size_t rr_;
  Stats stats_;
};

#endif  // BLE_MIDI_CODEC_H
