#ifndef CNCSMARTINTERFACE_ADC_UTILS_H
#define CNCSMARTINTERFACE_ADC_UTILS_H

#endif //CNCSMARTINTERFACE_ADC_UTILS_H

#include <pico/stdlib.h>
#include <hardware/adc.h>

float readBatteryVoltage(int batteryMosfet, float r1, float r2);
float readInternalBatteryVoltage(float r1, float r2);
float readInternalTemperature();
