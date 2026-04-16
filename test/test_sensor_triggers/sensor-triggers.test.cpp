#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <ArduinoFake.h>
#include <doctest.h>
#include "esp_log_stub.h"
#include "sensors/input_sensor.h"
// Include implementation directly — avoids build_src_filter issues with PIO test mode
#include "sensors/input_sensor.cpp"

// Minimal concrete Sensor subclass for testing the base class pipeline.
// measure_sensor() applies pending state (next_reading_valid, next_value) so that
// store_previous_values() correctly captures the PREVIOUS state.
class TestSensor : public Sensor {
 public:
  bool next_measure_result =
      true;  // what measure_sensor() returns (new sample available?)

  // Pending values applied during measure_sensor()
  bool has_pending_reading_valid = false;
  bool pending_reading_valid = true;
  bool has_pending_value = false;
  float pending_value = 0.0;

  TestSensor() {}

  void add_axis(const std::string& name) { sensor_dat[name] = SensorDat(); }

  void init() override {}
  void setup() override {}

  bool measure_sensor() override {
    // Apply pending state changes (simulates what real sensors do)
    if (has_pending_reading_valid) {
      for (auto& pair : sensor_dat) {
        pair.second.reading_valid = pending_reading_valid;
      }
      has_pending_reading_valid = false;
    }
    if (has_pending_value) {
      for (auto& pair : sensor_dat) {
        pair.second.value = pending_value;
      }
      has_pending_value = false;
    }
    return next_measure_result;
  }

  // Schedule reading_valid change for next measure_sensor() call
  void set_next_reading_valid(bool val) {
    has_pending_reading_valid = true;
    pending_reading_valid = val;
  }

  // Schedule value change for next measure_sensor() call
  void set_next_value(float val) {
    has_pending_value = true;
    pending_value = val;
  }

  void set_sensor_config(JsonObject config, bool debug = false) override {}
  JsonDocument get_sensor_config(bool debug = false) override {
    return JsonDocument();
  }

  // Expose protected sensor_dat for test manipulation
  SensorDat& dat(const std::string& axis) { return sensor_dat[axis]; }
};

// ============================================================================
// store_previous_values: saves value_ready, reading_valid, within_bounds, in_range
// ============================================================================

TEST_CASE("store_previous_values saves all _prev fields") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").value_ready = 50.0;
  s.dat("x").reading_valid = true;
  s.dat("x").within_bounds = true;
  s.dat("x").in_range = true;

  s.dat("x").value_prev = 0.0;
  s.dat("x").reading_valid_prev = false;
  s.dat("x").within_bounds_prev = false;
  s.dat("x").in_range_prev = false;

  s.dat("x").lmin = 0.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 50.0;
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("x").value_prev == 50.0);
  CHECK(s.dat("x").reading_valid_prev == true);
  CHECK(s.dat("x").within_bounds_prev == true);
  CHECK(s.dat("x").in_range_prev == true);
}

// ============================================================================
// is_within_range: returns the in_range flag (composite)
// ============================================================================

TEST_CASE("is_within_range returns in_range flag") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").in_range = true;
  CHECK(s.is_within_range("x") == true);

  s.dat("x").in_range = false;
  CHECK(s.is_within_range("x") == false);
}

TEST_CASE("is_prev_within_range returns in_range_prev flag") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").in_range_prev = true;
  CHECK(s.is_prev_within_range("x") == true);

  s.dat("x").in_range_prev = false;
  CHECK(s.is_prev_within_range("x") == false);
}

// ============================================================================
// within_bounds: base class computes for ALL axes from value_ready vs [lmin, lmax]
// ============================================================================

TEST_CASE("within_bounds computed true when value inside [lmin, lmax]") {
  TestSensor s;
  s.add_axis("pitch");
  s.dat("pitch").lmin = 0.0;
  s.dat("pitch").lmax = 100.0;
  s.dat("pitch").deadband = 0;
  s.dat("pitch").NeutralFilter.setDeadband(0);
  s.dat("pitch").value = 50.0;
  s.dat("pitch").within_bounds = false;  // will be computed to true
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("pitch").within_bounds == true);
  CHECK(s.dat("pitch").in_range ==
        true);  // reading_valid=true (default) && within_bounds=true
}

TEST_CASE("within_bounds computed false when value outside [lmin, lmax]") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").lmin = 10.0;
  s.dat("x").lmax = 90.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 95.0;          // above lmax
  s.dat("x").within_bounds = true;  // will be computed to false
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("x").within_bounds == false);
  CHECK(s.dat("x").in_range == false);
}

TEST_CASE(
    "in_range is false when reading_valid=false even if within_bounds=true") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);
  s.dat("dist").value = 30.0;  // within bounds
  s.dat("dist").reading_valid = false;
  s.dat("dist").invalid_count = 10;  // past debounce
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("dist").within_bounds == true);
  CHECK(s.dat("dist").in_range == false);  // reading_valid debounced to false
}

// ============================================================================
// Truth table: trigger/untrigger flags in continuous mode
// ============================================================================

TEST_CASE("Truth table row 1: in_range stays true — no trigger flags") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 0;  // continuous
  s.dat("x").lmin = 0.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 30.0;
  s.dat("x").value_ready = 30.0;
  s.dat("x").reading_valid = true;
  s.dat("x").in_range = true;
  s.dat("x").in_range_prev = true;
  s.next_measure_result = true;

  // First update: in_range stays true → no transition
  s.update();

  CHECK(s.dat("x").trigger_flags.midi_trig == false);
  CHECK(s.dat("x").untrigger_flags.midi_trig == false);
}

TEST_CASE("Truth table row 2: enter range — TRIGGER fires") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 0;
  s.dat("x").lmin = 0.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 0.0;
  s.dat("x").value_ready = 0.0;
  s.dat("x").value_prev = 0.0;

  // Start out of bounds
  s.dat("x").reading_valid = true;
  s.dat("x").within_bounds = false;
  s.dat("x").in_range = false;
  s.dat("x").in_range_prev = false;
  s.next_measure_result = true;

  // First update to establish in_range_prev = false
  s.update();

  // Schedule transition to within bounds
  s.set_next_value(30.0);
  s.update();

  CHECK(s.dat("x").trigger_flags.midi_trig == true);
  CHECK(s.dat("x").trigger_flags.osc_trig == true);
  CHECK(s.dat("x").untrigger_flags.midi_trig == false);
}

TEST_CASE("Truth table row 3: exit range (value > lmax) — UNTRIGGER fires") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range
  s.dat("dist").reading_valid = true;
  s.dat("dist").in_range = true;
  s.dat("dist").in_range_prev = true;
  s.dat("dist").within_bounds = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.dat("dist").value_prev = 30.0;
  s.next_measure_result = true;

  // First update: establish in_range_prev = true
  s.update();

  // Now exit: value goes above lmax (base class will compute within_bounds=false)
  s.set_next_value(100.0);
  s.update();

  CHECK(s.dat("dist").within_bounds == false);
  CHECK(s.dat("dist").in_range == false);
  CHECK(s.dat("dist").untrigger_flags.midi_trig == true);
  CHECK(s.dat("dist").untrigger_flags.osc_trig == true);
  CHECK(s.dat("dist").trigger_flags.midi_trig == false);
}

TEST_CASE("Truth table row 4: stays out of range — no flags") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start out of bounds
  s.dat("dist").reading_valid = true;
  s.dat("dist").value = 100.0;  // above lmax
  s.dat("dist").value_ready = 100.0;
  s.dat("dist").value_prev = 100.0;
  s.dat("dist").within_bounds = false;
  s.dat("dist").in_range = false;
  s.dat("dist").in_range_prev = false;
  s.next_measure_result = true;

  // First update: in_range_prev = false
  s.update();

  // Still out of bounds
  s.set_next_value(100.0);
  s.update();

  CHECK(s.dat("dist").trigger_flags.midi_trig == false);
  CHECK(s.dat("dist").untrigger_flags.midi_trig == false);
}

TEST_CASE(
    "Truth table row 6: HW invalid (debounced), was in range — UNTRIGGER") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // In range
  s.dat("dist").reading_valid = true;
  s.dat("dist").in_range = true;
  s.dat("dist").in_range_prev = true;
  s.dat("dist").within_bounds = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.dat("dist").value_prev = 30.0;
  s.next_measure_result = true;

  s.update();

  // HW goes invalid — need 3 consecutive invalid frames to debounce
  s.set_next_reading_valid(false);
  s.set_next_value(400.0);  // abs_max
  s.next_measure_result = true;
  s.update();                             // frame 1: still debounced as valid
  CHECK(s.dat("dist").in_range == true);  // debounce holds

  s.set_next_reading_valid(false);
  s.set_next_value(400.0);
  s.update();  // frame 2: still debounced
  CHECK(s.dat("dist").in_range == true);

  s.set_next_reading_valid(false);
  s.set_next_value(400.0);
  s.update();  // frame 3: debounce expires → in_range goes false

  CHECK(s.dat("dist").in_range == false);
  CHECK(s.dat("dist").untrigger_flags.midi_trig == true);
  CHECK(s.dat("dist").untrigger_flags.osc_trig == true);
}

TEST_CASE("Truth table row 8: re-enter range — TRIGGER fires") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Out of range (reading invalid, past debounce)
  s.dat("dist").reading_valid = false;
  s.dat("dist").invalid_count = 10;
  s.dat("dist").within_bounds = false;
  s.dat("dist").in_range = false;
  s.dat("dist").in_range_prev = false;
  s.dat("dist").value = 400.0;
  s.dat("dist").value_ready = 400.0;
  s.dat("dist").value_prev = 400.0;
  s.next_measure_result = true;

  s.update();

  // Re-enter: sensor gets valid reading within bounds
  s.set_next_reading_valid(true);
  s.set_next_value(25.0);
  s.update();

  CHECK(s.dat("dist").reading_valid == true);
  CHECK(s.dat("dist").within_bounds == true);
  CHECK(s.dat("dist").in_range == true);
  CHECK(s.dat("dist").trigger_flags.midi_trig == true);
  CHECK(s.dat("dist").trigger_flags.osc_trig == true);
}

// ============================================================================
// Hold mode scenario: value unchanged but in_range transitions
// This is THE core bug fix test — update() must not early-exit
// ============================================================================

TEST_CASE("Hold mode: update() processes triggers even when value unchanged") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").hold_mode = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range
  s.dat("dist").reading_valid = true;
  s.dat("dist").in_range = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.next_measure_result = true;

  s.update();

  // Sensor loses signal — value goes to abs_max but hold_mode reverts it.
  // reading_valid goes false. After debounce, in_range should transition.
  // Run enough frames to pass debounce (3 frames)
  for (int i = 0; i < 3; i++) {
    s.set_next_reading_valid(false);
    s.set_next_value(999.0);
    s.next_measure_result = true;
    s.update();
  }

  CHECK(s.dat("dist").in_range == false);
  CHECK(s.dat("dist").untrigger_flags.midi_trig == true);
  CHECK(s.dat("dist").untrigger_flags.osc_trig == true);
  // Value should be held at 30 due to hold_mode
  CHECK(s.dat("dist").value == 30.0);
}

TEST_CASE("Hold mode: value preserved when reading_valid=false") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").hold_mode = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range with value 30
  s.dat("dist").reading_valid = true;
  s.dat("dist").in_range = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.next_measure_result = true;

  s.update();

  // Sensor loses signal — measure_sensor would write 999 (abs_max),
  // but hold_mode should revert it because reading_valid=false
  s.set_next_reading_valid(false);
  s.set_next_value(999.0);
  s.next_measure_result = true;

  s.update();

  // .value should be held at 30 (reverted by base class)
  CHECK(s.dat("dist").value == 30.0);
}

TEST_CASE("No hold mode: value updates after debounce expires") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").hold_mode = false;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range with value 30
  s.dat("dist").reading_valid = true;
  s.dat("dist").in_range = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.next_measure_result = true;

  s.update();

  // Without hold_mode, value is still held during debounce window,
  // then flows through once debounce expires
  for (int i = 0; i < 3; i++) {
    s.set_next_reading_valid(false);
    s.set_next_value(999.0);
    s.next_measure_result = true;
    s.update();
  }

  // After debounce, value should have passed through on the 3rd frame
  CHECK(s.dat("dist").value == 999.0);
}

TEST_CASE("Normal mode: update() returns false when no new sample") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 0;
  s.dat("x").in_range = true;
  s.dat("x").in_range_prev = true;
  s.dat("x").value_ready = 50.0;
  s.dat("x").value_prev = 50.0;
  s.next_measure_result = false;  // HW not ready

  bool changed = s.update();

  CHECK(changed == false);
}

// ============================================================================
// Threshold mode: regression test — triggers on bool_value transitions
// ============================================================================

TEST_CASE("Threshold mode: trigger on rising edge") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 1;     // trigger mode
  s.dat("x").th_mode = 0;  // basic threshold
  s.dat("x").lmin = 10.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);

  // Below threshold
  s.dat("x").value = 5.0;
  s.dat("x").value_ready = 5.0;
  s.dat("x").bool_value = false;
  s.dat("x").bool_value_prev = false;
  s.next_measure_result = true;

  s.update();

  // Now above threshold
  s.dat("x").value = 15.0;
  s.update();

  CHECK(s.dat("x").bool_value == true);
  CHECK(s.dat("x").trigger_flags.midi_trig == true);
}

TEST_CASE("Threshold mode: untrigger on falling edge") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 1;
  s.dat("x").th_mode = 0;
  s.dat("x").lmin = 10.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);

  // Above threshold
  s.dat("x").value = 15.0;
  s.dat("x").value_ready = 15.0;
  s.dat("x").bool_value = true;
  s.dat("x").bool_value_prev = true;
  s.next_measure_result = true;

  s.update();

  // Drop below
  s.dat("x").value = 5.0;
  s.update();

  CHECK(s.dat("x").bool_value == false);
  CHECK(s.dat("x").untrigger_flags.midi_trig == true);
}

// ============================================================================
// Debounce: single-frame glitch should NOT trigger untrigger
// ============================================================================

TEST_CASE("Debounce: single invalid frame does not cause untrigger") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range
  s.dat("dist").reading_valid = true;
  s.dat("dist").in_range = true;
  s.dat("dist").in_range_prev = true;
  s.dat("dist").within_bounds = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.dat("dist").value_prev = 30.0;
  s.next_measure_result = true;

  s.update();

  // Single invalid frame (glitch)
  s.set_next_reading_valid(false);
  s.set_next_value(400.0);
  s.update();

  // Should still be in_range due to debounce
  CHECK(s.dat("dist").in_range == true);
  CHECK(s.dat("dist").untrigger_flags.midi_trig == false);

  // Recovery: valid reading returns
  s.set_next_reading_valid(true);
  s.set_next_value(30.0);
  s.update();

  CHECK(s.dat("dist").in_range == true);
  CHECK(s.dat("dist").invalid_count == 0);
}

TEST_CASE("Debounce: recovery resets counter immediately") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  s.dat("dist").reading_valid = true;
  s.dat("dist").in_range = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.next_measure_result = true;

  s.update();

  // 2 invalid frames (not enough)
  for (int i = 0; i < 2; i++) {
    s.set_next_reading_valid(false);
    s.set_next_value(400.0);
    s.update();
  }
  CHECK(s.dat("dist").in_range == true);
  CHECK(s.dat("dist").invalid_count == 2);

  // Recovery
  s.set_next_reading_valid(true);
  s.set_next_value(30.0);
  s.update();

  CHECK(s.dat("dist").invalid_count == 0);
  CHECK(s.dat("dist").in_range == true);
}
