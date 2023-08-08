//
// Created by PaulekOfficial on 08/08/2023.
//

#ifndef INTERFACE_PICO_SLEEPSETTINGS_H
#define INTERFACE_PICO_SLEEPSETTINGS_H

#include <list>
#include "pico/types.h"

enum SleepBehaviour {
    DELAY,
    SCHEDULE
};

enum DaysOfTheWeek {
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

class SleepSettings {
    private:
        SleepBehaviour sleepBehaviour;

        std::list<DaysOfTheWeek> wakeDays;
        datetime_t wakeTime;

        int batterySleepTimeUs;

        bool wakeOnExternalPowerSource;
        bool hibernate;
        bool deepSleep;
};


#endif //INTERFACE_PICO_SLEEPSETTINGS_H
