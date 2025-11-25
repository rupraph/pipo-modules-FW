Sensor update loop: (pipeline should be improved to be more generic)

store_previous_values
-> store "value_Ready" in "value_prev" (value prev is filtered...)
measure_sensor
-> raw measurement goes in "raw_value"
-> filtered measurement goes in "value"
apply offset
-> writes in "value_ready"
process_sensor_neutral_filter
-> overwrites "value_ready"
process_sensor_triggers
(uses value prev and value_ready)
