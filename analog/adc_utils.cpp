#include "adc_utils.h"

#define N_SAMPLES 1000
const double ADC_RESISTANCE = 500000.0;

uint16_t sample_buf[N_SAMPLES];

void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count) {
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    for (int i = 0; i < count; i = i + 1) {
        buf[i] = adc_fifo_get_blocking();
    }


    adc_run(false);
    adc_fifo_drain();
}

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
    busy_wait_ms(200);

    adc_capture(sample_buf, N_SAMPLES);

    double voltage = convert_adc_to_voltage(sample_buf[500]);

    // Turn off measurement channel
    gpio_put(batteryMosfet, false);

    //return (((((47000.0 * ADC_RESISTANCE) / (47000.0 + ADC_RESISTANCE)) / (47000.0 + ADC_RESISTANCE)) + 100000.0) * voltage) / ((47000.0 * ADC_RESISTANCE) / (47000.0 + ADC_RESISTANCE));
    return ((47000.0 + 99060.0) / 47000.0) * voltage;
}

double readInternalBatteryVoltage() {
    adc_select_input(2);
    busy_wait_ms(200);
    adc_capture(sample_buf, N_SAMPLES);

    double voltage = convert_adc_to_voltage(sample_buf[500]);
//    for (uint16_t sample : sample_buf) {
//        voltage += convert_adc_to_voltage(sample);
//    }
//    voltage /= N_SAMPLES;

    return ((47000.0 + 99060.0) / 47000.0) * voltage;
}
