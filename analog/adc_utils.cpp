#include "adc_utils.h"

const double ADC_RESISTANCE = 800000.0;

float convert_adc_to_voltage(uint16_t adc_value) {
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    return (float) adc_value * conversionFactor;
}

float readInternalTemperature() {
    adc_select_input(4);
    float voltage = convert_adc_to_voltage(adc_read());
    float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;

    return tempC;
}

double readBatteryVoltage(int batteryMosfet) {
    // Turn on measurement channel
    gpio_put(batteryMosfet, true);
    adc_select_input(0);
    busy_wait_ms(700);
    float v = convert_adc_to_voltage(adc_read());
    // Turn off measurement channel
    gpio_put(batteryMosfet, false);

    return (((((47000.0 * ADC_RESISTANCE) / (470000.0 + ADC_RESISTANCE)) / (470000.0 + ADC_RESISTANCE)) + 100000.0) * v) / ((47000.0 * ADC_RESISTANCE) / (470000.0 + ADC_RESISTANCE));
}

double readInternalBatteryVoltage() {
    adc_select_input(2);
    busy_wait_ms(700);
    float v = convert_adc_to_voltage(adc_read());

    return (((((47000.0 * ADC_RESISTANCE) / (470000.0 + ADC_RESISTANCE)) / (470000.0 + ADC_RESISTANCE)) + 100000.0) * v) / ((47000.0 * ADC_RESISTANCE) / (470000.0 + ADC_RESISTANCE));
}

