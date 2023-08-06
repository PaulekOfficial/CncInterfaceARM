#include <hardware/pll.h>
#include "main.h"

//TODO DEBUG GPIO 5-2

int main() {
    //Init pico usb
    stdio_init_all();
    stdio_usb_init();
    info("USB interface initialization done.");
    busy_wait_ms(2000);

    uint32_t new_freq = 68 * MHZ;
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                    0,
                    new_freq,
                    new_freq);

    pll_deinit(pll_sys);

    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                    0,
                    new_freq,
                    new_freq);
    setup_default_uart();

    //Init pico wifi module
    initInfineon();

    /* initialize random seed: */
    srand(time_us_64());

    //Setup used pins
    initGPIO();

    // Setup i2c register 1
    initI2C();

    // Setup spi
    setupUart();

    // Setup adc
    initADC();

    // Setup pull ups
    initPullUps();

    info("Read internal battery voltage to configure module");
    batteryVoltage = readInternalBatteryVoltage();

    // Set microcontroller indicator on
    info("Mpu set state to on");
    cyw43_arch_gpio_put(MPU_LED, true);

    disp.init(128, 32, 0x3C, I2C_ID);
    disp.poweron();

    disp.clear();
    disp.bmp_show_image(__paulek_bmp_data, 1086);
    disp.show();

    busy_wait_ms(2000);

    // Setup rtc
    info("RTC clock initialization...");
    rtc_init();

    multicore_launch_core1(core1_entry);

    // Save clock speed
    scb_orig = scb_hw->scr;
    clock0_orig = clocks_hw->sleep_en0;
    clock1_orig = clocks_hw->sleep_en1;

    printf("scb clock: %u \r\n", scb_orig);
    printf("clock0 clock: %u \r\n", clock0_orig);
    printf("clock1 clock: %u \r\n", clock1_orig);

    // Setup watchdog
    info("Watchdog system initialization...");
    //watchdog_enable(9 * 1000, false);
    if(watchdog_caused_reboot()) info("ALERT!!! System rebooted by watchdog, investigation required.");

    setupEEpromSettings();

    setupWifiSettings();

    if (state == CONFIGURATION_HARDWARE) {

    }

    loop();
}

[[noreturn]] void loop()
{
    info("Entering main loop.");
    gpio_put(RELAY_POWER_24V, gpio_get(V_24_SENSE));

    disp.clear();
    disp.draw_string(10, 10, 2, "WAIT");
    disp.show();

    batteryVoltage = readInternalBatteryVoltage();
    info("Internal voltage: " + to_string(batteryVoltage));


    double batteryVoltage0 = readBatteryVoltage(RELAY_BAT_0);
    info("Battery0 voltage: " + to_string(batteryVoltage0));


    double batteryVoltage1 = readBatteryVoltage(RELAY_BAT_1);
    info("Battery1 voltage: " + to_string(batteryVoltage1));


    float temperature = readInternalTemperature();
    bool highVoltagePresent = gpio_get(POWER_24V_READY);


    std::list<CurrentMeasurement> measurements;
    CurrentMeasurement internalBattery("0", INTERNAL_BATTERY_VOLTAGE, batteryVoltage);
    measurements.push_back(internalBattery);

    CurrentMeasurement temp("0", TEMPERATURE, temperature);
    measurements.push_back(temp);

    CurrentMeasurement battery0("0", BATTERY_VOLTAGE, batteryVoltage0);
    measurements.push_back(battery0);

    CurrentMeasurement battery1("1", BATTERY_VOLTAGE, batteryVoltage1);
    measurements.push_back(battery1);

    InterfaceMeasurement interfaceMeasurement("352da5cf-7e92-45ca-88a5-639e5dc2f592", BATTERY_MODE, measurements);

    disp.clear();
    disp.draw_string(10, 10, 2, "PACKAGE");
    disp.show();

    HTTPRequestBuilder requestBuilder(hostname.data(), port, "keep-alive", POST, "/smart-interface/measurement", JSON);
    requestBuilder.setPayload(interfaceMeasurement.serialize());
    info(requestBuilder.build_request());

    disp.clear();
    disp.draw_string(10, 10, 2, "SEND");
    disp.show();

    WiFiManager::http_request(requestBuilder);

    writeInfo(temperature, batteryVoltage0, batteryVoltage1, batteryVoltage, highVoltagePresent);
    loopShutdown(highVoltagePresent);

    loop();
}

void writeInfo(double temperature, double batteryVoltage0, double batteryVoltage1, double internalBattery, bool highVoltagePresent) {
    char title[32];
    sprintf(title, "%2.2fC", temperature);

    char subtitle[32];
    char subtitle2[32];
    char subtitle3[32];
    sprintf(subtitle, "VBAT:%2.2fV", internalBattery);
    

    busy_wait_ms(2000);
    sprintf(subtitle2, "Bateria 0: %2.2fV", batteryVoltage0);

    busy_wait_ms(2000);
    sprintf(subtitle3, "Bateria 1: %2.2fV", batteryVoltage1);

    disp.clear();
    disp.draw_string(90, 0, 1, title);
    disp.draw_string(0, 0, 1, subtitle);
    disp.draw_string(0, 10, 1, subtitle2);
    disp.draw_string(0, 20, 1, subtitle3);
    disp.show();

    
    busy_wait_ms(8000);
    

    if (highVoltagePresent) {
        sprintf(subtitle, "Zasilanie 24V");
    } else {
        sprintf(subtitle, "Brak 24V");
    }

    disp.clear();
    disp.draw_string(0, 0, 1, title);
    disp.draw_string(0, 10, 1, subtitle);
    disp.draw_string(0, 10, 1, subtitle);
    disp.show();

    
    busy_wait_ms(2000);
    
}
