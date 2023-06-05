//
// Created by pawel on 01/06/2023.
//

#ifndef INTERFACE_PICO_CURRENTMEASUREMENT_H
#define INTERFACE_PICO_CURRENTMEASUREMENT_H

#include <string>
#include <map>
#include <sstream>

enum CurrentType {
    BATTERY_VOLTAGE,
    INTERNAL_BATTERY_VOLTAGE,
    CURRENT,
    TEMPERATURE
};
static const char* CurrentType_String[] = {
        "BATTERY_VOLTAGE",
        "INTERNAL_BATTERY_VOLTAGE",
        "CURRENT",
        "TEMPERATURE"
};

class CurrentMeasurement {
private:
    char* name;
    CurrentType type;
    double value;

public:
    virtual ~CurrentMeasurement();

    CurrentMeasurement(char *name, CurrentType type, double value);

    [[nodiscard]] char *getName() const;

    void setName(char *name);

    [[nodiscard]] CurrentType getType() const;

    void setType(CurrentType type);

    [[nodiscard]] double getValue() const;

    void setValue(double value);

    std::string serialize();

    bool operator==(const CurrentMeasurement &rhs) const;

    bool operator!=(const CurrentMeasurement &rhs) const;
};

#endif //INTERFACE_PICO_CURRENTMEASUREMENT_H
