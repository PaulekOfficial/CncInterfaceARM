//
// Created by PaulekOfficial on 08/08/2023.
//

#ifndef INTERFACE_PICO_POWERSETTINGS_H
#define INTERFACE_PICO_POWERSETTINGS_H


#include "SleepSettings.h"

class PowerSettings {
    private:
        SleepSettings sleepSettings;

        bool disableLcdOnLowVoltage;
        double minimalLcdVoltage;

        bool wifiPowerSaveMode;
        double wifiPowerSaveModeMinimalVoltage;
        uint32_t wifiPowerManagementMode;
};


#endif //INTERFACE_PICO_POWERSETTINGS_H
