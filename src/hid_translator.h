#ifndef HID_TRANSLATOR_H
#define HID_TRANSLATOR_H

#include <Arduino.h>
#include "json.hpp"

class HidTranslator
{
private:
    /* data */
public:
    int out_type=0; //0=axis, 1=button
    std::string mapto="x"; //Gamepad: x,y,z,rz,rx,ry,buttons, hat //Mouse: x,y,wheel,buttons,pan //Keyboard: keycodes

    // axis are int8_t

    bool quantize=0;
    int quantize_steps=5;
    int input_max=180;
    int input_min=-180;

    int use_threshold=1;
    int threshold=0;

    int output_max=255;
    int output_min=0;

    HidTranslator();

    int map_linear(float value);
    int get_current_bool(float value);
    int get_current_int(float value);

    void to_json(nlohmann::json& j, const HidTranslator& t);
    void from_json(const nlohmann::json& j, HidTranslator& t);

    nlohmann::json get_json() const;
    void set_from_json(const nlohmann::json& j);
    
    std::string serialize() const;
    void deserialize(const std::string& data);


};


#endif //HID_TRANSLATOR_H