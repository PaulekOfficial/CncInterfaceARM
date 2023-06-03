//
// Created by pawel on 01/06/2023.
//

#include "CurrentMeasurement.h"

std::string CurrentMeasurement::serialize() {
    //{"name":"0", "type":"BATTERY_VOLTAGE", "value":"6.14"}

    std::map<std::string,std::string> jsonMap;
    auto iterator = jsonMap.begin();

    jsonMap.insert(iterator, std::pair<std::string,std::string>("name", name));
    jsonMap.insert(iterator, std::pair<std::string,std::string>("type", CurrentType_String[type]));
    jsonMap.insert(iterator, std::pair<std::string,std::string>("value", std::to_string(value)));

    std::stringstream stream;

    stream << "{";
    int count = 0;
    for (auto & i : jsonMap) {
        stream << "\"";
        stream << i.first;
        stream << "\"";
        stream << ":";
        stream << "\"";
        stream << i.second;
        stream << "\"";

        count++;
        if (count < jsonMap.size()) {
            stream << ",";
        }
    }
    stream << "}";

    return stream.str();
}

char *CurrentMeasurement::getName() const {
    return name;
}

void CurrentMeasurement::setName(char *name) {
    CurrentMeasurement::name = name;
}

CurrentType CurrentMeasurement::getType() const {
    return type;
}

void CurrentMeasurement::setType(CurrentType type) {
    CurrentMeasurement::type = type;
}

double CurrentMeasurement::getValue() const {
    return value;
}

void CurrentMeasurement::setValue(double value) {
    CurrentMeasurement::value = value;
}

CurrentMeasurement::CurrentMeasurement(char *name, CurrentType type, double value) : name(name), type(type), value(value)
{

}

bool CurrentMeasurement::operator==(const CurrentMeasurement &rhs) const {
    return static_cast<const CurrentMeasurement &>(*this) == static_cast<const CurrentMeasurement &>(rhs) &&
           name == rhs.name &&
           type == rhs.type &&
           value == rhs.value;
}

bool CurrentMeasurement::operator!=(const CurrentMeasurement &rhs) const {
    return !(rhs == *this);
}

CurrentMeasurement::~CurrentMeasurement() {
    delete name;
}
