#include "osc_translators.h"


using namespace std;
using json = nlohmann::json;

float OscTranslator::get_value(float value, float input_min, float input_max)
{
    if (mode_raw)
    {
        return value;
    }
    else
    {
        return (value - input_min) / (input_max - input_min) * (output_max - output_min) + output_min;
    }
}


void to_json(json& j, const OscTranslator& t) {
    j = json{
        {"enabled", t.enabled},
        {"mode_raw", t.mode_raw},
        {"output_max", t.output_max},
        {"output_min", t.output_min},
    };
}

void from_json(const json& j, OscTranslator& t) {
    j.at("enabled").get_to(t.enabled);
    j.at("mode_raw").get_to(t.mode_raw);
    j.at("output_max").get_to(t.output_max);
    j.at("output_min").get_to(t.output_min);
}

string OscTranslator::serialize() const {
    json j = *this;
    return j.dump();
}
void OscTranslator::deserialize(const string& data) {
    json j = json::parse(data);
    *this = j.get<OscTranslator>();
}

json OscTranslator::get_json() const {
    return json(*this);
}

void OscTranslator::set_from_json(const json& j) {
    *this = j.get<OscTranslator>();
}





