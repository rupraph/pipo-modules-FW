#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <ArduinoFake.h>
#include <doctest.h>
#include "esp_log_stub.h"
#include "sensors/input_sensor.h"
// Include implementation directly — avoids build_src_filter issues with PIO test mode
#include "sensors/input_sensor.cpp"

// Minimal concrete Sensor subclass for testing the base class pipeline.
// measure_sensor() applies pending state (next_in_range, next_value) so that
// store_previous_values() correctly captures the PREVIOUS state.
class TestSensor : public Sensor {
 public:
  bool next_measure_result = true;  // what measure_sensor() returns

  // Pending values applied during measure_sensor()
  bool has_pending_in_range = false;
  bool pending_in_range = true;
  bool has_pending_value = false;
  float pending_value = 0.0;

  TestSensor() {}

  void add_axis(const std::string& name) { sensor_dat[name] = SensorDat(); }

  void init() override {}
  void setup() override {}

  bool measure_sensor() override {
    // Apply pending state changes (simulates what real sensors do)
    if (has_pending_in_range) {
      for (auto& pair : sensor_dat) {
        pair.second.in_range = pending_in_range;
      }
      has_pending_in_range = false;
    }
    if (has_pending_value) {
      for (auto& pair : sensor_dat) {
        pair.second.value = pending_value;
      }
      has_pending_value = false;
    }
    return next_measure_result;
  }

  // Schedule in_range change for next measure_sensor() call
  void set_next_in_range(bool val) {
    has_pending_in_range = true;
    pending_in_range = val;
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
// store_previous_values: saves both value_ready and in_range
// ============================================================================

TEST_CASE("store_previous_values saves value_prev and in_range_prev") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").value_ready = 50.0;
  s.dat("x").in_range = true;

  // Simulate one update cycle: store_previous, then change current
  s.dat("x").value_prev = 0.0;
  s.dat("x").in_range_prev = false;

  // Call update which calls store_previous_values internally
  // But we need direct access — use the pipeline:
  // Set measure to return true so update() proceeds
  s.dat("x").lmin = 0.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;  // disable neutral filter deadband
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 50.0;
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("x").value_prev == 50.0);
  CHECK(s.dat("x").in_range_prev == true);
}

// ============================================================================
// is_within_range: returns the in_range flag
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
// Auto-compute in_range for axes without in_range_set_by_sensor
// ============================================================================

TEST_CASE("Auto-compute in_range from value_ready for non-hw-set axes") {
  TestSensor s;
  s.add_axis("pitch");
  s.dat("pitch").in_range_set_by_sensor = false;  // default
  s.dat("pitch").lmin = 0.0;
  s.dat("pitch").lmax = 100.0;
  s.dat("pitch").deadband = 0;
  s.dat("pitch").NeutralFilter.setDeadband(0);
  s.dat("pitch").value = 50.0;
  s.dat("pitch").in_range = false;  // will be auto-computed to true
  s.dat("pitch").in_range_prev = false;
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("pitch").in_range == true);
}

TEST_CASE("Auto-compute in_range false when value outside bounds") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").in_range_set_by_sensor = false;
  s.dat("x").lmin = 10.0;
  s.dat("x").lmax = 90.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 95.0;  // above lmax
  s.dat("x").in_range = true;
  s.dat("x").in_range_prev = true;
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("x").in_range == false);
}

TEST_CASE("Sensor-set in_range is NOT overwritten by auto-compute") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").in_range_set_by_sensor = true;  // like range sensor
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);
  s.dat("dist").value = 30.0;        // value within bounds...
  s.dat("dist").in_range = false;    // ...but sensor says NOT in range (e.g. hold mode)
  s.dat("dist").in_range_prev = true;
  s.next_measure_result = true;

  s.update();

  // Should remain false because in_range_set_by_sensor is true
  CHECK(s.dat("dist").in_range == false);
}

// ============================================================================
// Truth table: trigger/untrigger flags in continuous mode
// ============================================================================

TEST_CASE("Truth table row 1: in_range stays true — no trigger flags") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 0;  // continuous
  s.dat("x").in_range_set_by_sensor = true;
  s.dat("x").in_range = true;
  s.dat("x").in_range_prev = true;  // will be overwritten by store_previous
  s.dat("x").lmin = 0.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 30.0;
  s.dat("x").value_ready = 30.0;
  s.next_measure_result = true;

  // First update: store_previous saves in_range=true → in_range_prev=true
  // measure_sensor keeps in_range=true → no transition
  s.update();

  CHECK(s.dat("x").trigger_flags.midi_trig == false);
  CHECK(s.dat("x").untrigger_flags.midi_trig == false);
}

TEST_CASE("Truth table row 2: enter range — TRIGGER fires") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 0;
  s.dat("x").in_range_set_by_sensor = true;
  s.dat("x").lmin = 0.0;
  s.dat("x").lmax = 100.0;
  s.dat("x").deadband = 0;
  s.dat("x").NeutralFilter.setDeadband(0);
  s.dat("x").value = 0.0;
  s.dat("x").value_ready = 0.0;
  s.dat("x").value_prev = 0.0;

  // Start out of range
  s.dat("x").in_range = false;
  s.dat("x").in_range_prev = false;
  s.next_measure_result = true;

  // First update to establish in_range_prev = false
  s.update();

  // Schedule transition to in-range (applied inside measure_sensor)
  s.set_next_in_range(true);
  s.set_next_value(30.0);
  s.update();

  CHECK(s.dat("x").trigger_flags.midi_trig == true);
  CHECK(s.dat("x").trigger_flags.osc_trig == true);
  CHECK(s.dat("x").untrigger_flags.midi_trig == false);
}

TEST_CASE("Truth table row 3: exit range (obstacle > lmax) — UNTRIGGER fires") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").in_range_set_by_sensor = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range
  s.dat("dist").in_range = true;
  s.dat("dist").in_range_prev = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.dat("dist").value_prev = 30.0;
  s.next_measure_result = true;

  // First update: establish in_range_prev = true
  s.update();

  // Now exit: sensor detects obstacle at 100cm > lmax(60)
  s.set_next_in_range(false);
  // hold_mode: value unchanged, so no set_next_value
  s.update();

  CHECK(s.dat("dist").untrigger_flags.midi_trig == true);
  CHECK(s.dat("dist").untrigger_flags.osc_trig == true);
  CHECK(s.dat("dist").trigger_flags.midi_trig == false);
}

TEST_CASE("Truth table row 4: stays out of range — no flags") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").in_range_set_by_sensor = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start out of range
  s.dat("dist").in_range = false;
  s.dat("dist").in_range_prev = false;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.dat("dist").value_prev = 30.0;
  s.next_measure_result = true;

  // First update: in_range_prev = false
  s.update();

  // Still out of range (no pending change — measure_sensor keeps in_range = false)
  s.update();

  CHECK(s.dat("dist").trigger_flags.midi_trig == false);
  CHECK(s.dat("dist").untrigger_flags.midi_trig == false);
}

TEST_CASE("Truth table row 6: HW invalid, was in range — UNTRIGGER") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").in_range_set_by_sensor = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // In range
  s.dat("dist").in_range = true;
  s.dat("dist").in_range_prev = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.dat("dist").value_prev = 30.0;
  s.next_measure_result = true;

  s.update();

  // HW goes invalid → sensor sets in_range = false during measure_sensor
  s.set_next_in_range(false);
  s.next_measure_result = true;  // data_ready = true even in hold mode
  s.update();

  CHECK(s.dat("dist").untrigger_flags.midi_trig == true);
  CHECK(s.dat("dist").untrigger_flags.osc_trig == true);
}

TEST_CASE("Truth table row 8: re-enter range — TRIGGER fires") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").in_range_set_by_sensor = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Out of range
  s.dat("dist").in_range = false;
  s.dat("dist").in_range_prev = false;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.dat("dist").value_prev = 30.0;
  s.next_measure_result = true;

  s.update();

  // Re-enter: sensor sets in_range = true during measure_sensor
  s.set_next_in_range(true);
  s.set_next_value(25.0);
  s.update();

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
  s.dat("dist").in_range_set_by_sensor = true;
  s.dat("dist").hold_mode = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range
  s.dat("dist").in_range = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.next_measure_result = true;

  s.update();

  // Simulate hold mode: measure_sensor returns false (no new value data),
  // but in_range changes to false during measure_sensor
  s.set_next_in_range(false);
  s.next_measure_result = false;  // no new value — hold mode holds value

  bool changed = s.update();

  CHECK(changed == true);  // update() should NOT early-exit
  CHECK(s.dat("dist").untrigger_flags.midi_trig == true);
  CHECK(s.dat("dist").untrigger_flags.osc_trig == true);
}

TEST_CASE("Hold mode: .value preserved when out of range") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").in_range_set_by_sensor = true;
  s.dat("dist").hold_mode = true;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range with value 30
  s.dat("dist").in_range = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.next_measure_result = true;

  s.update();

  // Go out of range — measure_sensor would write a new value (e.g. 999),
  // but hold_mode should revert it to 30
  s.set_next_in_range(false);
  s.set_next_value(999.0);  // sensor writes abs_max
  s.next_measure_result = true;

  s.update();

  // .value should be held at 30 (reverted by base class)
  CHECK(s.dat("dist").value == 30.0);
}

TEST_CASE("No hold mode: .value updates when out of range") {
  TestSensor s;
  s.add_axis("dist");
  s.dat("dist").mode = 0;
  s.dat("dist").in_range_set_by_sensor = true;
  s.dat("dist").hold_mode = false;
  s.dat("dist").lmin = 2.0;
  s.dat("dist").lmax = 60.0;
  s.dat("dist").deadband = 0;
  s.dat("dist").NeutralFilter.setDeadband(0);

  // Start in range with value 30
  s.dat("dist").in_range = true;
  s.dat("dist").value = 30.0;
  s.dat("dist").value_ready = 30.0;
  s.next_measure_result = true;

  s.update();

  // Go out of range — without hold_mode, value should update normally
  s.set_next_in_range(false);
  s.set_next_value(999.0);
  s.next_measure_result = true;

  s.update();

  CHECK(s.dat("dist").value == 999.0);
}

TEST_CASE("Normal mode: update() returns false when no data and no range change") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 0;
  s.dat("x").in_range_set_by_sensor = true;
  s.dat("x").in_range = true;
  s.dat("x").in_range_prev = true;
  s.dat("x").value_ready = 50.0;
  s.dat("x").value_prev = 50.0;
  s.next_measure_result = false;

  bool changed = s.update();

  CHECK(changed == false);
}

// ============================================================================
// Threshold mode: regression test — triggers on bool_value transitions
// ============================================================================

TEST_CASE("Threshold mode: trigger on rising edge") {
  TestSensor s;
  s.add_axis("x");
  s.dat("x").mode = 1;  // trigger mode
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
