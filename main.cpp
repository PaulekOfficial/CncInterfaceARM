#include "main.h"

//TODO DEBUG GPIO 5-2

static void alarm_callback() {
    wakeUp = true;
}

void core1_entry() {
    while (true) {
        if (wifi_manager.connecting()) {
            disp.clear();
            disp.bmp_show_image_with_offset(__wifi_bmp_data, 226, 5, 5);

            int x = 50;
            for (int i = 0; i < dots; i++) {
                disp.draw_string(x, 6, 2, ".");
                x += 15;
            }
            disp.show();

            dots++;
            if (dots > 4) {
                dots = 0;
            }

            watchdog_update();
        }
        busy_wait_ms(1000);
    }
}

int main() {
    //Init pico usb
    stdio_init_all();
    stdio_usb_init();
    info("USB interface initialization done.");
    busy_wait_ms(2000);

    //Init pico wifi module
    info("WiFi infineon 43439 initializing...");
    if (cyw43_arch_init()) {
        info("WiFi init failed, killing process...");
        return -1;
    }
    info("Done.");

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
    watchdog_update();

    // Setup wifi
    wifi_manager.set_network("Xiaomi_52E3", "102101281026");
    wifi_manager.connect();

    if (wifi_manager.connected() == 0) {
        disp.clear();
        disp.bmp_show_image_with_offset(__wifi_bmp_data, 226, 5, 5);
        disp.draw_string(50, 13, 1, "Connected!");
        disp.show();

        busy_wait_ms(2000);
    }

    loop();
}

void loop()
{
    info("Entering main loop.");
    while (true)
    {
        watchdog_update();

        disp.clear();
        disp.draw_string(10, 10, 2, "WAIT");
        disp.show();

        batteryVoltage = readInternalBatteryVoltage();
        info("Internal voltage: " + to_string(batteryVoltage));
        watchdog_update();

        double batteryVoltage0 = readBatteryVoltage(RELAY_BAT_0);
        info("Battery0 voltage: " + to_string(batteryVoltage0));
        watchdog_update();

        double batteryVoltage1 = readBatteryVoltage(RELAY_BAT_1);
        info("Battery1 voltage: " + to_string(batteryVoltage1));
        watchdog_update();

        float temperature = readInternalTemperature();
        bool highVoltagePresent = gpio_get(POWER_24V_READY);
        watchdog_update();

        disp.clear();
        disp.draw_string(10, 10, 2, "PACKAGE");
        disp.show();

        SimpleRequestBuilder simpleRequest("api.pauleklab.com", batteryVoltage0, batteryVoltage1, "battery");

        disp.clear();
        disp.draw_string(10, 10, 2, "SEND");
        disp.show();

        wifi_manager.http_request(simpleRequest.getHost(), simpleRequest.getUrl());

        writeInfo(temperature, batteryVoltage0, batteryVoltage1, batteryVoltage, highVoltagePresent);
        if (!highVoltagePresent)
        {
            watchdog_update();
            shutdown();

            sleep_run_from_rosc();
            setupAlarm();

            while (true)
            {
                sleep = true;
                bool buttonPressed = gpio_get(BUTTON);
                if (buttonPressed || wakeUp) {
                    awake();

                    wakeUp = false;
                    rtc_disable_alarm();
                    watchdog_update();
                    break;
                }

                watchdog_update();
            }
            sleep = false;
        }

        watchdog_update();
    }
}

void writeInfo(double temperature, double batteryVoltage0, double batteryVoltage1, double internalBattery, bool highVoltagePresent) {
    char title[32];
    sprintf(title, "%2.2fC", temperature);

    char subtitle[32];
    char subtitle2[32];
    char subtitle3[32];
    sprintf(subtitle, "VBAT:%2.2fV", internalBattery);
    watchdog_update();

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

    watchdog_update();
    busy_wait_ms(8000);
    watchdog_update();

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

    watchdog_update();
    busy_wait_ms(2000);
    watchdog_update();
}

void awake() {
    watchdog_update();

    disp.init(128, 32, 0x3C, I2C_ID);
    disp.poweron();

    disp.clear();
    disp.draw_string(10, 10, 1, "Waking up...");
    disp.show();

    watchdog_enable(9 * 1000, false);
    watchdog_update();
    recover_from_sleep(0, 4294967295, 32767);
    watchdog_update();

    info("WiFi infineon 43439 reinitializing...");
    if (cyw43_arch_init()) {
        info("WiFi init failed, killing process...");
        return;
    }
    info("Done.");
    cyw43_arch_gpio_put(MPU_LED, true);

    // Setup wifi
    //setupWiFiModule("Xiaomi_52E3", "102101281026");
}

void shutdown() {
    info("Shutting down...");
    gpio_put(RELAY_BAT_0, false);
    gpio_put(RELAY_BAT_1, false);
    gpio_put(RELAY_POWER_24V, false);
    gpio_put(MOSFET_BUZZER, false);
    disp.poweroff();
    disp.deinit();

    watchdog_update();

    cyw43_arch_deinit();
    gpio_put(MOSFET_LCD, false);

    watchdog_update();
}

void setupAlarm() {
    printf("RTC Alarm Repeat!\n");

    // Start on Wednesday 13th January 2021 11:20:00
    datetime_t t = {
            .year  = 2023,
            .month = 01,
            .day   = 13,
            .dotw  = 3, // 0 is Sunday, so 3 is Wednesday
            .hour  = 11,
            .min   = 20,
            .sec   = 00
    };

    // Setup the RTC
    rtc_set_datetime(&t);

    // Alarm time
    datetime_t alarm = {
            .year  = 2023,
            .month = 01,
            .day   = 13,
            .dotw  = 3, // 0 is Sunday, so 3 is Wednesday
            .hour  = 11,
            .min   = 50,
            .sec   = 00
    };

    rtc_set_alarm(&alarm, &alarm_callback);
}
