//
// Created by pawel on 01/06/2023.
//

#ifndef INTERFACE_PICO_INTERFACEMEASUREMENT_H
#define INTERFACE_PICO_INTERFACEMEASUREMENT_H


#include <list>
#include "SmartInterface.h"
#include "CurrentMeasurement.h"

class InterfaceMeasurement {
private:
    char* interfaceUUID;
    SmartInterface_Status status;
    std::list<CurrentMeasurement> measurementList;
    long timestamp;

public:

};

//private UUID interfaceUUID;
//private SmartInterface.InterfaceStatus status;
//public List<CurrentMeasurement> measurementList;
//public ZonedDateTime timestamp;
//
//public InterfaceMeasurement() {
//}
//
//public InterfaceMeasurement(UUID interfaceUUID, SmartInterface.InterfaceStatus status, List<CurrentMeasurement> measurementList) {
//this.interfaceUUID = interfaceUUID;
//this.status = status;
//this.measurementList = measurementList;
//}
//
//public InterfaceMeasurement(UUID interfaceUUID, SmartInterface.InterfaceStatus status, List<CurrentMeasurement> measurementList, ZonedDateTime timestamp) {
//this.interfaceUUID = interfaceUUID;
//this.status = status;
//this.measurementList = measurementList;
//this.timestamp = timestamp;
//}
//
//public UUID getInterfaceUUID() {
//    return interfaceUUID;
//}
//
//public void setInterfaceUUID(UUID interfaceUUID) {
//this.interfaceUUID = interfaceUUID;
//}
//
//public SmartInterface.InterfaceStatus getStatus() {
//    return status;
//}
//
//public void setStatus(SmartInterface.InterfaceStatus status) {
//this.status = status;
//}
//
//public List<CurrentMeasurement> getMeasurementList() {
//    return measurementList;
//}
//
//public void setMeasurementList(List<CurrentMeasurement> measurementList) {
//    this.measurementList = measurementList;
//}
//
//public ZonedDateTime getTimestamp() {
//    return timestamp;
//}
//
//public void setTimestamp(ZonedDateTime timestamp) {
//    this.timestamp = timestamp;
//}
//
//@Override
//public boolean equals(Object o) {
//    if (this == o) return true;
//    if (!(o instanceof InterfaceMeasurement that)) return false;
//    return Objects.equals(interfaceUUID, that.interfaceUUID) && status == that.status && Objects.equals(measurementList, that.measurementList) && Objects.equals(timestamp, that.timestamp);
//}
//
//@Override
//public int hashCode() {
//    return Objects.hash(interfaceUUID, status, measurementList, timestamp);
//}
//
//@Override
//public String toString() {
//    return "InterfaceMeasurement{" +
//           "interfaceUUID=" + interfaceUUID +
//           ", status=" + status +
//           ", measurementList=" + measurementList +
//           ", timestamp=" + timestamp +
//           '}';
//}

#endif //INTERFACE_PICO_INTERFACEMEASUREMENT_H
