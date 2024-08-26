#ifndef HID_TRANSLATOR_H
#define HID_TRANSLATOR_H

#include <Arduino.h>
#include "utils/json.hpp"

using namespace std;

class HidTranslator
{
private:
    /* data */
public:

    //Todo: link to usb hid mode or use only one of the 2
    int out_type=0; //0=axis, 1=button
    string mapto="x"; //Gamepad: x,y,z,rz,rx,ry,buttons, hat //Mouse: x,y,wheel,buttons,pan //Keyboard: keycodes

    // axis are int8_t

    bool quantize=0;
    int quantize_steps=5;


    int use_threshold=0;
    int threshold=0;

    int output_max=255;
    int output_min=0;

    bool disabled=false;

    HidTranslator();

    int map_linear(float value, float min_input, float max_input);
    int get_current_bool(float valu, float min_input, float max_input);
    int get_current_int(float value, float min_input, float max_input);

    void to_json(nlohmann::json& j, const HidTranslator& t);
    void from_json(const nlohmann::json& j, HidTranslator& t);

    nlohmann::json get_json() const;
    void set_from_json(const nlohmann::json& j);
    
    string serialize() const;
    void deserialize(const string& data);

    template <typename T>
    void set_param(const string& param_name, const T& value){
        if (param_name == "out_type") {
            out_type = value;
        }
        else if (param_name == "mapto") {
            mapto = value;
        }
        else if (param_name == "quantize") {
            quantize = value;
        }
        else if (param_name == "quantize_steps") {
            quantize_steps = value;
        }
        else if (param_name == "use_threshold") {
            use_threshold = value;
        }
        else if (param_name == "threshold") {
            threshold = value;
        }
        else if (param_name == "output_max") {
            output_max = value;
        }
        else if (param_name == "output_min") {
            output_min = value;
        }
        else {
            Serial.println("Invalid parameter name.") ;
        }
    }


};


#endif //HID_TRANSLATOR_H