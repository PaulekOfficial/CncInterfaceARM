//
// Created by pawel on 01/06/2023.
//
#include "InterfaceMeasurement.h"
#include "utils/utils.h"

char *InterfaceMeasurement::getInterfaceUuid() const {
    return interfaceUUID;
}

void InterfaceMeasurement::setInterfaceUuid(char *interfaceUuid) {
    interfaceUUID = interfaceUuid;
}

SmartInterface_Status InterfaceMeasurement::getStatus() const {
    return status;
}

void InterfaceMeasurement::setStatus(SmartInterface_Status status) {
    InterfaceMeasurement::status = status;
}

const std::list<CurrentMeasurement> &InterfaceMeasurement::getMeasurementList() const {
    return measurementList;
}

void InterfaceMeasurement::setMeasurementList(const std::list<CurrentMeasurement> &measurementList) {
    InterfaceMeasurement::measurementList = measurementList;
}

long InterfaceMeasurement::getTimestamp() const {
    return timestamp;
}

void InterfaceMeasurement::setTimestamp(long timestamp) {
    InterfaceMeasurement::timestamp = timestamp;
}

InterfaceMeasurement::InterfaceMeasurement(char *interfaceUuid, SmartInterface_Status status,
                                           const std::list<CurrentMeasurement> &measurementList) : interfaceUUID(
        interfaceUuid), status(status), measurementList(measurementList) {}

InterfaceMeasurement::InterfaceMeasurement(char *interfaceUuid, SmartInterface_Status status,
                                           const std::list<CurrentMeasurement> &measurementList, long timestamp)
        : interfaceUUID(interfaceUuid), status(status), measurementList(measurementList), timestamp(timestamp) {}

std::string InterfaceMeasurement::serialize() {
    //{"interfaceUUID":"352da5cf-7e92-45ca-88a5-639e5dc2f592","status":"BATTERY_MODE","measurementList": [{"name":"", "type":"INTERNAL_BATTERY_VOLTAGE", "value":".15"},{"name":"0", "type":"BATTERY_VOLTAGE", "value":"6.14"},{"name":"1", "type":"BATTERY_VOLTAGE", "value":"2.16"}]}

    std::map<std::string,std::string> jsonMap;
    auto iterator = jsonMap.begin();

    jsonMap.insert(iterator, std::pair<std::string,std::string>("interfaceUUID", interfaceUUID));
    jsonMap.insert(iterator, std::pair<std::string,std::string>("status", SmartInterface_Status_String[status]));

    std::stringstream arrayStream;
    arrayStream << "[";
    int count = 0;
    for (auto & i : measurementList) {
        arrayStream << i.serialize();

        count++;
        if (count < measurementList.size()) {
            arrayStream << ",";
        }
    }
    arrayStream << "]";

    jsonMap.insert(iterator, std::pair<std::string,std::string>("measurementList", arrayStream.str()));

    std::stringstream stream;

    stream << "{";
    count = 0;
    for (auto & i : jsonMap) {
        stream << "\"";
        stream << i.first;
        stream << "\"";
        stream << ":";

        if (i.second[0] == '[') {
            stream << i.second;
        } else {
            stream << "\"";
            stream << i.second;
            stream << "\"";
        }

        count++;
        if (count < jsonMap.size()) {
            stream << ",";
        }
    }
    stream << "}";

    return stream.str();
}

bool InterfaceMeasurement::operator==(const InterfaceMeasurement &rhs) const {
    return static_cast<const InterfaceMeasurement &>(*this) == static_cast<const InterfaceMeasurement &>(rhs) &&
           interfaceUUID == rhs.interfaceUUID &&
           status == rhs.status &&
           measurementList == rhs.measurementList &&
           timestamp == rhs.timestamp;
}

bool InterfaceMeasurement::operator!=(const InterfaceMeasurement &rhs) const {
    return !(rhs == *this);
}

InterfaceMeasurement::~InterfaceMeasurement() {
    delete interfaceUUID;
}
