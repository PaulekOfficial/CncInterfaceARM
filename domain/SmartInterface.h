//
// Created by pawel on 01/06/2023.
//

#ifndef INTERFACE_PICO_SMARTINTERFACE_H
#define INTERFACE_PICO_SMARTINTERFACE_H

enum SmartInterface_Status {
    RUNNING,
    IDLE,
    BATTERY_MODE,
    SLEEPING,
    OFFLINE
};
static const char* SmartInterface_Status_String[] = {
        "RUNNING",
        "IDLE",
        "BATTERY_MODE",
        "SLEEPING",
        "OFFLINE"
};

class SmartInterface {

public:

};


#endif //INTERFACE_PICO_SMARTINTERFACE_H
