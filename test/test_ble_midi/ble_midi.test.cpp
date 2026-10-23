#define DOCTEST_CONFIG_IMPLEMENT  // REQUIRED: Enable custom main()
#include <doctest.h>
#include <vector>
#include "midi/ble_midi_codec.h"

struct Got {
  uint8_t status, d1, d2;
  bool operator==(const Got& o) const {
    return status == o.status && d1 == o.d1 && d2 == o.d2;
  }
};

static std::vector<Got> parse(const uint8_t* data, size_t len) {
  std::vector<Got> out;
  bleMidiParse(data, len, [&out](uint8_t s, uint8_t a, uint8_t b) {
    Got g = {s, a, b};
    out.push_back(g);
  });
  return out;
}

static Got G(uint8_t s, uint8_t a, uint8_t b) {
  Got g = {s, a, b};
  return g;
}

static BleMidiMsg M(uint8_t s, uint8_t a, uint8_t b) {
  BleMidiMsg m = {s, a, b};
  return m;
}

// ---------------------------------------------------------------- encoder

TEST_CASE("Message length depends on status") {
  CHECK(bleMidiMsgLen(0x90) == 3);
  CHECK(bleMidiMsgLen(0xB5) == 3);
  CHECK(bleMidiMsgLen(0xE0) == 3);
  CHECK(bleMidiMsgLen(0xC2) == 2);
  CHECK(bleMidiMsgLen(0xD0) == 2);
}

TEST_CASE("Encode puts one header and a timestamp before each message") {
  BleMidiMsg msgs[] = {M(0xB0, 0x07, 0x64), M(0xC0, 0x03, 0)};
  uint8_t out[16];
  size_t n = bleMidiEncode(msgs, 2, 0x1ABC, out, sizeof(out));
  const uint8_t expected[] = {0xB5, 0xBC, 0xB0, 0x07, 0x64, 0xBC, 0xC0, 0x03};
  REQUIRE(n == sizeof(expected));
  for (size_t i = 0; i < n; i++) CHECK(out[i] == expected[i]);
}

TEST_CASE("Encode returns 0 when the buffer is too small") {
  BleMidiMsg msgs[] = {M(0xB0, 0x07, 0x64), M(0xC0, 0x03, 0)};
  uint8_t out[7];
  CHECK(bleMidiEncode(msgs, 2, 0, out, sizeof(out)) == 0);
}

TEST_CASE("Encode then parse round-trips") {
  BleMidiMsg msgs[] = {M(0x90, 60, 127), M(0xE3, 0x00, 0x40), M(0xC1, 9, 0)};
  uint8_t out[32];
  size_t n = bleMidiEncode(msgs, 3, 1234, out, sizeof(out));
  std::vector<Got> got = parse(out, n);
  REQUIRE(got.size() == 3);
  CHECK(got[0] == G(0x90, 60, 127));
  CHECK(got[1] == G(0xE3, 0x00, 0x40));
  CHECK(got[2] == G(0xC1, 9, 0));
}

// ---------------------------------------------------------------- parser

TEST_CASE("Parse a single program change") {
  const uint8_t p[] = {0x80, 0x80, 0xC2, 0x05};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0xC2, 5, 0));
}

TEST_CASE("Parse two timestamped messages") {
  const uint8_t p[] = {0x80, 0x81, 0xB0, 0x07, 0x64, 0x82, 0xC0, 0x03};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 2);
  CHECK(got[0] == G(0xB0, 0x07, 0x64));
  CHECK(got[1] == G(0xC0, 0x03, 0));
}

TEST_CASE("Parse running status without timestamp") {
  const uint8_t p[] = {0x80, 0x80, 0xB0, 0x07, 0x10, 0x08, 0x20};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 2);
  CHECK(got[0] == G(0xB0, 0x07, 0x10));
  CHECK(got[1] == G(0xB0, 0x08, 0x20));
}

TEST_CASE("Parse running status with timestamp") {
  const uint8_t p[] = {0x80, 0x80, 0xB0, 0x07, 0x10, 0x81, 0x08, 0x20};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 2);
  CHECK(got[1] == G(0xB0, 0x08, 0x20));
}

TEST_CASE("MIDI clock between messages does not stop parsing") {
  const uint8_t p[] = {0x80, 0x80, 0xF8, 0x81, 0xC1, 0x09};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0xC1, 9, 0));
}

TEST_CASE("Clock keeps running status") {
  const uint8_t p[] = {0x80, 0x80, 0xB0, 0x07, 0x10, 0x81, 0xF8, 0x08, 0x20};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 2);
  CHECK(got[1] == G(0xB0, 0x08, 0x20));
}

TEST_CASE("SysEx is skipped and the following message is parsed") {
  const uint8_t p[] = {0x80, 0x80, 0xF0, 0x7E, 0x01, 0x02,
                       0x81, 0xF7, 0x82, 0xC0, 0x04};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0xC0, 4, 0));
}

TEST_CASE("SysEx continuation packet is skipped") {
  const uint8_t p[] = {0x80, 0x01, 0x02, 0x03, 0x81, 0xF7, 0x82, 0xC0, 0x05};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0xC0, 5, 0));
}

TEST_CASE("System common message data is skipped and clears running status") {
  // song position (2 data bytes), then orphan data, then a PC
  const uint8_t p[] = {0x80, 0x80, 0xB0, 0x07, 0x10, 0x81, 0xF2,
                       0x01, 0x02, 0x08, 0x20, 0x82, 0xC0, 0x06};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 2);
  CHECK(got[0] == G(0xB0, 0x07, 0x10));
  CHECK(got[1] == G(0xC0, 6, 0));
}

TEST_CASE("Truncated message is not emitted and nothing past len is read") {
  // The byte after len would complete the CC if it were read.
  const uint8_t p[] = {0x80, 0x80, 0xB0, 0x07, 0x64};
  CHECK(parse(p, 4).empty());
}

TEST_CASE("Message cut by a timestamp is dropped, the next one survives") {
  const uint8_t p[] = {0x80, 0x80, 0xB0, 0x07, 0x81, 0xC0, 0x02};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0xC0, 2, 0));
}

TEST_CASE("Invalid or too short packets are ignored") {
  const uint8_t one[] = {0x80};
  const uint8_t noHeaderBit[] = {0x00, 0x80, 0xC0, 0x01};
  CHECK(parse(nullptr, 0).empty());
  CHECK(parse(one, 1).empty());
  CHECK(parse(noHeaderBit, sizeof(noHeaderBit)).empty());
}

TEST_CASE("Data bytes without running status are ignored") {
  const uint8_t p[] = {0x80, 0x05, 0x06};
  CHECK(parse(p, sizeof(p)).empty());
}

TEST_CASE("Trailing timestamp is ignored") {
  const uint8_t p[] = {0x80, 0x80, 0xC0, 0x01, 0x81};
  std::vector<Got> got = parse(p, sizeof(p));
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0xC0, 1, 0));
}

// ---------------------------------------------------------------- queue

// Default budget fits a full queue (32 x 4 bytes + header).
static std::vector<Got> take(BleMidiOutQueue& q, size_t maxMsgs = 64,
                             size_t maxBytes = 256) {
  BleMidiMsg buf[64];
  size_t n = q.takeBatch(buf, maxMsgs, maxBytes);
  std::vector<Got> out;
  for (size_t i = 0; i < n; i++) out.push_back(G(buf[i].status, buf[i].d1, buf[i].d2));
  return out;
}

TEST_CASE("Same CC coalesces to the latest value") {
  BleMidiOutQueue q;
  q.push(0xB0, 7, 0x10);
  q.push(0xB0, 7, 0x20);
  std::vector<Got> got = take(q);
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0xB0, 7, 0x20));
  CHECK(q.empty());
}

TEST_CASE("Different CC numbers and channels stay separate") {
  BleMidiOutQueue q;
  q.push(0xB0, 7, 1);
  q.push(0xB0, 8, 2);
  q.push(0xB1, 7, 3);
  CHECK(take(q).size() == 3);
}

TEST_CASE("Pitch bend coalesces per channel") {
  BleMidiOutQueue q;
  q.push(0xE0, 0x00, 0x40);
  q.push(0xE0, 0x7F, 0x7F);
  q.push(0xE1, 0x00, 0x00);
  std::vector<Got> got = take(q);
  REQUIRE(got.size() == 2);
  CHECK(got[0] == G(0xE0, 0x7F, 0x7F));
}

TEST_CASE("Notes keep their order and never coalesce") {
  BleMidiOutQueue q;
  q.push(0x90, 60, 127);
  q.push(0x80, 60, 0);
  q.push(0x90, 60, 100);
  std::vector<Got> got = take(q);
  REQUIRE(got.size() == 3);
  CHECK(got[0] == G(0x90, 60, 127));
  CHECK(got[1] == G(0x80, 60, 0));
  CHECK(got[2] == G(0x90, 60, 100));
}

TEST_CASE("Ordered messages come before latest-value messages") {
  BleMidiOutQueue q;
  q.push(0xB0, 7, 1);
  q.push(0x90, 60, 127);
  std::vector<Got> got = take(q);
  REQUIRE(got.size() == 2);
  CHECK(got[0] == G(0x90, 60, 127));
  CHECK(got[1] == G(0xB0, 7, 1));
}

TEST_CASE("Batch respects the byte budget and leaves the rest queued") {
  BleMidiOutQueue q;
  q.push(0x90, 1, 1);
  q.push(0x90, 2, 1);
  q.push(0x90, 3, 1);
  // header (1) + 2 x (timestamp + 3 bytes) = 9
  std::vector<Got> got = take(q, 64, 9);
  REQUIRE(got.size() == 2);
  got = take(q);
  REQUIRE(got.size() == 1);
  CHECK(got[0] == G(0x90, 3, 1));
}

TEST_CASE("Batch respects the message budget") {
  BleMidiOutQueue q;
  q.push(0x90, 1, 1);
  q.push(0x90, 2, 1);
  CHECK(take(q, 1).size() == 1);
  CHECK(take(q, 1).size() == 1);
  CHECK(q.empty());
}

TEST_CASE("Full ordered queue drops a new Note On") {
  BleMidiOutQueue q;
  for (int i = 0; i < (int)BleMidiOutQueue::kOrderedCap; i++) CHECK(q.push(0x90, i, 100));
  CHECK_FALSE(q.push(0x90, 100, 100));
  CHECK(q.stats().droppedOrdered == 1);
  CHECK(take(q).size() == BleMidiOutQueue::kOrderedCap);
}

TEST_CASE("Full ordered queue evicts the oldest Note On for a Note Off") {
  BleMidiOutQueue q;
  for (int i = 0; i < (int)BleMidiOutQueue::kOrderedCap; i++) q.push(0x90, i, 100);
  CHECK(q.push(0x80, 40, 0));
  CHECK(q.stats().evictedNoteOn == 1);
  std::vector<Got> got = take(q);
  REQUIRE(got.size() == BleMidiOutQueue::kOrderedCap);
  CHECK(got.front() == G(0x90, 1, 100));
  CHECK(got.back() == G(0x80, 40, 0));
}

TEST_CASE("Note On with velocity 0 counts as a Note Off") {
  BleMidiOutQueue q;
  for (int i = 0; i < (int)BleMidiOutQueue::kOrderedCap; i++) q.push(0x90, i, 100);
  CHECK(q.push(0x90, 40, 0));
  CHECK(q.stats().evictedNoteOn == 1);
}

TEST_CASE("Full queue of Note Offs drops a new Note Off") {
  BleMidiOutQueue q;
  for (int i = 0; i < (int)BleMidiOutQueue::kOrderedCap; i++) q.push(0x80, i, 0);
  CHECK_FALSE(q.push(0x80, 100, 0));
  CHECK(q.stats().droppedOrdered == 1);
}

TEST_CASE("A Note Off already queued is not queued twice") {
  BleMidiOutQueue q;
  q.push(0x80, 60, 0);
  CHECK(q.push(0x80, 60, 0));
  CHECK(take(q).size() == 1);
}

TEST_CASE("Full latest-value slots drop a new key but still update existing ones") {
  BleMidiOutQueue q;
  for (int i = 0; i < (int)BleMidiOutQueue::kLatestCap; i++) CHECK(q.push(0xB0, i, 1));
  CHECK_FALSE(q.push(0xB0, 100, 1));
  CHECK(q.stats().droppedLatest == 1);
  CHECK(q.push(0xB0, 0, 99));
  std::vector<Got> got = take(q);
  REQUIRE(got.size() == BleMidiOutQueue::kLatestCap);
  CHECK(got[0] == G(0xB0, 0, 99));
}

TEST_CASE("Sent slots are reused once a new key needs room") {
  BleMidiOutQueue q;
  for (int i = 0; i < (int)BleMidiOutQueue::kLatestCap; i++) q.push(0xB0, i, 1);
  take(q);
  CHECK(q.push(0xB0, 100, 1));
  CHECK(q.stats().droppedLatest == 0);
}

TEST_CASE("Round-robin: a busy CC does not starve the others") {
  BleMidiOutQueue q;
  q.push(0xB0, 1, 1);
  q.push(0xB0, 2, 1);
  CHECK(take(q, 1)[0] == G(0xB0, 1, 1));
  q.push(0xB0, 1, 2);
  CHECK(take(q, 1)[0] == G(0xB0, 2, 1));
  CHECK(take(q, 1)[0] == G(0xB0, 1, 2));
}

TEST_CASE("Clear empties both queues") {
  BleMidiOutQueue q;
  q.push(0x90, 60, 127);
  q.push(0xB0, 7, 1);
  q.clear();
  CHECK(q.empty());
  CHECK(take(q).empty());
}

int main(int argc, char** argv) {
  doctest::Context context;

  // BEGIN:: PLATFORMIO REQUIRED OPTIONS
  context.setOption("success", true);  // Report successful tests
  context.setOption("no-exitcode",
                    true);  // Do not return non-zero code on failed test case
  // END:: PLATFORMIO REQUIRED OPTIONS

  context.applyCommandLine(argc, argv);
  return context.run();
}
