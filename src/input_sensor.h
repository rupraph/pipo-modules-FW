#ifndef INPUT_SENSOR_H
#define INPUT_SENSOR_H

#include <vector>
#include <unordered_map>
#include "json.hpp"
#include "filters.h"

using namespace std;
using json = nlohmann::json;

class Sensor {
public:
    // static sensor& getInstance();
    
    // sensor(const sensor&) = delete;
    // void operator=(const sensor&) = delete;

    //Sensor() {}  // Constructor is protected to prevent direct instantiation
    
    virtual void init()=0;
    virtual void setup()=0;
    virtual void update()=0;

     // might want to move this structure outside of the class
    struct SensorDat {
        bool enabled;
        bool inverted; 
        float deadZone; // supposed to be % of the total range. value for now
        float value;
        float offset;
    };

    

    //config 
    json get_config(bool debug=false);
    void set_config(json& config,bool debug=false);

    //Getter setters

    unordered_map<string, SensorDat> get_sensor_dat_map();

    bool get_enabled(const std::string& axis);
    void set_enabled(const std::string& axis, bool value);

    bool get_inverted(const std::string& axis);
    void set_inverted(const std::string& axis, bool value);

    int get_deadZone(const std::string& axis);
    void set_deadZone(const std::string& axis, int value);

    float get_value(const std::string& axis);
    void set_value(const std::string& axis, float value);

    float get_offset(const std::string& axis);
    void set_offset(const std::string& axis, float value);

    void teleplot_data(string axis);

    bool test_outside_deadzone(const std::string& axis);

protected:
    unordered_map<string, SensorDat> sensor_dat;
    

// private:
//     static sensor* instance;
};

#endif // INPUT_SENSOR_H