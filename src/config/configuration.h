#ifndef CNCSMARTINTERFACE_CONFIGURATION_H
#define CNCSMARTINTERFACE_CONFIGURATION_H

#endif //CNCSMARTINTERFACE_CONFIGURATION_H

struct Configuration {
    char firmware_version[5];
    char secret[60];

    char uuid[36];
    char name[16];

    float internal_r1, r1;
    float internal_r2, r2;

    float adc_resistance;
    float adc_current;

    bool should_sleep;
    int sleep_time;

    bool sleep_when_24V_present;
    bool status_readings;
    bool read_temperature;
} configuration;
