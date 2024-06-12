#include "input_sensor.h"
#include "acc_sensor.h"
#include "range_sensor.h"
//#include "analog_sensor.h"

//Todo: replace throw with Serial

void Sensor::teleplot_data(string axis)
{   
    if (sensor_dat.find(axis) == sensor_dat.end())
    {
        Serial.println("error: Axis not found");
        return;
    }
    else
    {
        Serial.print(">");
        Serial.print(axis.c_str());
        Serial.print(": ");
        Serial.println(sensor_dat[axis].value);
    }
}

unsigned long Sensor::measured_loop_duration()
{
    measured_loop = millis() - last_update;
    last_update = millis();
    return measured_loop;
}

void Sensor::start_duration()
{
    start_time = millis();
}

unsigned long Sensor::end_duration()
{
    measured_interval_duration = millis() - start_time;
    return measured_interval_duration;
}

//config

json Sensor::get_config(bool debug)
{
    json config;
    for (auto const& pair : sensor_dat)
    {
        string axis_name = pair.first;
        config[axis_name]["enabled"] = sensor_dat[axis_name].enabled;
        config[axis_name]["inverted"] = sensor_dat[axis_name].inverted;
        config[axis_name]["deadZone"] = sensor_dat[axis_name].deadZone;
        config[axis_name]["value"] = sensor_dat[axis_name].value;
        config[axis_name]["offset"] = sensor_dat[axis_name].offset;
        config[axis_name]["limit_max"] = sensor_dat[axis_name].limit_max;
        config[axis_name]["limit_min"] = sensor_dat[axis_name].limit_min;
    }
    if (debug)
    {
        Serial.println("returned_sensor_get_config");
        Serial.println(config.dump(4).c_str());
        Serial.println("returned_sensor_get_config_end");
    }
    return config;
}

void Sensor::set_config(json& config, bool debug)
{
    for (auto const& pair : config.items())
    {
        string axis_name = pair.key();
        sensor_dat[axis_name].enabled = config[axis_name]["enabled"];
        sensor_dat[axis_name].inverted = config[axis_name]["inverted"];
        sensor_dat[axis_name].deadZone = config[axis_name]["deadZone"];
        sensor_dat[axis_name].value = config[axis_name]["value"];
        sensor_dat[axis_name].offset = config[axis_name]["offset"];
        sensor_dat[axis_name].limit_max = config[axis_name]["limit_max"];
        sensor_dat[axis_name].limit_min = config[axis_name]["limit_min"];
    }
    if (debug)
    {
        Serial.println("set_sensor_config");
        Serial.println(config.dump(4).c_str());
        Serial.println("set_sensor_config_end");
    }
}

//gett setters

unordered_map<string, Sensor::SensorDat> Sensor::get_sensor_dat_map() {
    return sensor_dat;
}

bool Sensor::get_enabled(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].enabled;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

bool Sensor::get_inverted(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].inverted;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

int Sensor::get_deadZone(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].deadZone;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_value(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_value_prev(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].value_prev;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_limit_max(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].limit_max;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_limit_min(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].limit_min;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_offset(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].offset;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_triggered(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].triggered;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_untriggered(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].untriggered;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}




//Setters

void Sensor::set_enabled(const std::string& axis, bool value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].enabled = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_inverted(const std::string& axis, bool value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].inverted = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_deadZone(const std::string& axis, int value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].deadZone = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_value(const std::string& axis, float value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].value = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_value_prev(const std::string& axis, float value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].value_prev = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_offset(const std::string& axis, float value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].offset = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_limit_max(const std::string& axis, float value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].limit_max = value;

    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_limit_min(const std::string& axis, float value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].limit_min = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_triggered(const std::string& axis, bool value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].triggered = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_untriggered(const std::string& axis, bool value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].untriggered = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}


//Todo: deadzone should be in percentage or max or in value ?
bool Sensor::test_outside_deadzone(const std::string& axis)
{   
    if(sensor_dat.find(axis) != sensor_dat.end())
    {

        if (abs(sensor_dat[axis].value) > sensor_dat[axis].deadZone)
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    else
    {
        Serial.println("error: Axis not found");
        return false;
    }
}