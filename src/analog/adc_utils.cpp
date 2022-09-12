#include <analog/adc_utils.h>

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

float readBatteryVoltage(int batteryMosfet, float r1, float r2) {
    // Turn on measurement channel
    gpio_put(batteryMosfet, true);

    adc_select_input(3);

    float v = convert_adc_to_voltage(adc_read());

    // Turn off measurement channel
    gpio_put(batteryMosfet, false);

    float voltage = (v * (r1 + r2)) / r2;

    return voltage;
}

float readInternalBatteryVoltage(float r1, float r2) {
    // Turn on measurement channel
    //gpio_put(INTERNAL_BATTERY_MOSFET, true);

    adc_select_input(2);

    float v = convert_adc_to_voltage(adc_read());

    // Turn off measurement channel
    //gpio_put(INTERNAL_BATTERY_MOSFET, false);

    float voltage = (v * (r1 + r2)) / r2;

    return voltage;
}

