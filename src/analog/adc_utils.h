#ifndef CNCSMARTINTERFACE_ADC_UTILS_H
#define CNCSMARTINTERFACE_ADC_UTILS_H

#endif //CNCSMARTINTERFACE_ADC_UTILS_H

#include <pico/stdlib.h>
#include <hardware/adc.h>

double readBatteryVoltage(int batteryMosfet);
double readInternalBatteryVoltage();
float readInternalTemperature();
