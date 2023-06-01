//
// Created by pawel on 01/06/2023.
//

#ifndef INTERFACE_PICO_CURRENTMEASUREMENT_H
#define INTERFACE_PICO_CURRENTMEASUREMENT_H


class CurrentMeasurement {

};

//private String name;
//private CurrentType type;
//private double value;
//
//public CurrentMeasurement(String name, CurrentType type, double value) {
//this.name = name;
//this.type = type;
//this.value = value;
//}
//
//public String getName() {
//    return name;
//}
//
//public void setName(String name) {
//this.name = name;
//}
//
//public CurrentType getType() {
//    return type;
//}
//
//public void setType(CurrentType type) {
//    this.type = type;
//}
//
//public double getValue() {
//    return value;
//}
//
//public void setValue(double value) {
//    this.value = value;
//}
//
//public enum CurrentType {
//    BATTERY_VOLTAGE("battery"),
//    INTERNAL_BATTERY_VOLTAGE("internal-battery"),
//    CURRENT("current");
//
//    private final String name;
//
//    CurrentType(String name) {
//        this.name = name;
//    }
//
//    public String getName() {
//        return name;
//    }
//
//    public static CurrentType getByName(String name) {
//        for (CurrentType type : CurrentType.values()) {
//            if (name.equalsIgnoreCase(type.getName())) {
//                return type;
//            }
//        }
//
//        return null;
//    }
//}

#endif //INTERFACE_PICO_CURRENTMEASUREMENT_H
