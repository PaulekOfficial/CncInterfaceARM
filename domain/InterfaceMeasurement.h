//
// Created by pawel on 01/06/2023.
//

#ifndef INTERFACE_PICO_INTERFACEMEASUREMENT_H
#define INTERFACE_PICO_INTERFACEMEASUREMENT_H


#include <list>
#include "SmartInterface.h"
#include <string>
#include <map>
#include <sstream>
#include "CurrentMeasurement.h"

class InterfaceMeasurement {
private:
    char* interfaceUUID;
    SmartInterface_Status status;
    std::list<CurrentMeasurement> measurementList;
    long timestamp{};

public:
    virtual ~InterfaceMeasurement();

    InterfaceMeasurement(char *interfaceUuid, SmartInterface_Status status,
                         const std::list<CurrentMeasurement> &measurementList, long timestamp);

    InterfaceMeasurement(char *interfaceUuid, SmartInterface_Status status,
                         const std::list<CurrentMeasurement> &measurementList);

    char *getInterfaceUuid() const;

    void setInterfaceUuid(char *interfaceUuid);

    SmartInterface_Status getStatus() const;

    void setStatus(SmartInterface_Status status);

    const std::list<CurrentMeasurement> &getMeasurementList() const;

    void setMeasurementList(const std::list<CurrentMeasurement> &measurementList);

    long getTimestamp() const;

    void setTimestamp(long timestamp);

    std::string serialize();

    bool operator==(const InterfaceMeasurement &rhs) const;

    bool operator!=(const InterfaceMeasurement &rhs) const;
};

#endif //INTERFACE_PICO_INTERFACEMEASUREMENT_H
