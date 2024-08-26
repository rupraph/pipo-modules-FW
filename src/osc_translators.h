#ifndef OSC_TRANSLATORS_H
#define OSC_TRANSLATORS_H

#include <Arduino.h>
#include "config.h"
#include "utils/json.hpp"


using namespace std;

class OscTranslator
{
    public:
        //OscTranslator();// = default;
        // OscTranslator(float input_min, float input_max): input_min(input_min), input_max(input_max) {}

        bool enabled=false;
        bool mode_raw=true; // sends raw sensor data

        float output_max=1;
        float output_min=0;

        float get_value(float value, float min_input, float max_input);
        int get_current_bool(float value, float min_input, float max_input);

        void to_json(nlohmann::json& j, const OscTranslator& t);
        void from_json(const nlohmann::json& j, OscTranslator& t);

        nlohmann::json get_json() const;
        void set_from_json(const nlohmann::json& j);

        string serialize() const;
        void deserialize(const string& data);

    
    //private:
        // float input_min;
        // float input_max;

};

#endif // OSC_TRANSLATORS_H