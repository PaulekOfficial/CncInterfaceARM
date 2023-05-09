#include "main.h"

static void alarm_callback() {
    wakeUp = true;
}

void core1_entry() {
    while (true) {
        if (wifiConnecting) {
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
    watchdog_enable(9 * 1000, false);
    if(watchdog_caused_reboot()) info("ALERT!!! System rebooted by watchdog, investigation required.");
    watchdog_update();

    // Setup wifi
    setupWiFiModule("Xiaomi_52E3", "102101281026");

    loop();
}

void loop()
{
    info("Entering main loop.");
    while (true)
    {
        watchdog_update();

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

        if (batteryVoltage >= 2.5) writeInfo(temperature, batteryVoltage0, batteryVoltage1, batteryVoltage, highVoltagePresent);
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
//    char title[32];
//    sprintf(title, "Temp: %2.2f C", temperature);
//
//    char subtitle[32];
//    sprintf(subtitle, "VBAT:%2.2fV", internalBattery);
//    watchdog_update();
//
//    lcd.sendMessages(title, subtitle, 0);
//    busy_wait_ms(2000);
//    sprintf(subtitle, "Bateria 0: %2.2fV", batteryVoltage0);
//    lcd.sendMessages(title, subtitle, 0);
//    watchdog_update();
//
//    busy_wait_ms(2000);
//    sprintf(subtitle, "Bateria 1: %2.2fV", batteryVoltage1);
//    lcd.sendMessages(title, subtitle, 0);
//    watchdog_update();
//    busy_wait_ms(2000);
//    watchdog_update();
//
//    if (highVoltagePresent) {
//        sprintf(subtitle, "Zasilanie 24V");
//    } else {
//        sprintf(subtitle, "Brak 24V");
//    }
//
//    lcd.sendMessages(title, subtitle, 0);
//    watchdog_update();
//    busy_wait_ms(2000);
//    watchdog_update();
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
    setupWiFiModule("Xiaomi_52E3", "102101281026");
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

void initADC() {
    info("ADC interface initialization...");
    adc_init();

    adc_set_temp_sensor_enabled(true);
    adc_gpio_init(ADC_SYSTEM_BATTERY);
    adc_gpio_init(ADC_EXTERNAL_BATTERY);
    info("Done.");
}
void initI2C() {
    info("I2C register interface initialization...");
    i2c_init(I2C_ID, 400 * 1000);
    info("setting gpio i2c functions");
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    info("activating internal pullup's");
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}
void initGPIO() {
    info("GPIO interface initialization...");
    gpio_init(RELAY_POWER_24V);
    gpio_init(RELAY_BAT_0);
    gpio_init(RELAY_BAT_1);
    gpio_init(MOSFET_BUZZER);
    gpio_init(MOSFET_LCD);
    //gpio_set_function(MPU_LED, GPIO_FUNC_PWM);

    // Setup pwm
    uint slice_num = pwm_gpio_to_slice_num(MPU_LED);
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);

    gpio_init(BUZZER);
    gpio_set_dir(RELAY_POWER_24V, GPIO_OUT);
    gpio_set_dir(RELAY_BAT_0, GPIO_OUT);
    gpio_set_dir(RELAY_BAT_1, GPIO_OUT);
    gpio_set_dir(MOSFET_BUZZER, GPIO_OUT);
    gpio_set_dir(MOSFET_LCD, GPIO_OUT);
    //gpio_set_dir(MPU_LED, GPIO_OUT);
    gpio_set_dir(BUZZER, GPIO_OUT);
}

void initPullUps() {
    info("Pull ups initialization...");
    gpio_pull_up(BUZZER);
}

void setupWiFiModule(const char *ssid, const char *password) {
    info("Turining on wifi module");
    cyw43_arch_enable_sta_mode();

    int wifiConnectionTrys = 0;
    int hardResetAttempts = 0;
    int connected = -1;

    wifiConnecting = true;

    while (connected)
    {
        busy_wait_ms(2500);
        watchdog_update();
        connected = cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_MIXED_PSK, 5000);
        watchdog_update();

        for (int i = 0; i < 15; i++)
        {
            busy_wait_ms(1000);
            watchdog_update();
        }

        bool hardReset = false;
        wifiConnectionTrys++;

        if (wifiConnectionTrys <= 5 && connected)
        {
            // Display status
            char title[32];
            sprintf(title, "WiFi E:%d", wifiConnectionTrys);

            for (int i = 0; i < 30; i++)
            {
                busy_wait_ms(1000);
                watchdog_update();
            }
        }

        if (wifiConnectionTrys >= 5 && connected)
        {
            hardReset = true;
            wifiConnectionTrys = 0;
            hardResetAttempts++;
        }

        if (hardReset)
        {
            info("ESP8266 init reset fail, performing hard reset.");

            for (int i = 0; i < 60; i++)
            {
                busy_wait_ms(1000);
                watchdog_update();
            }

            for (int i = 0; i < 60; i++)
            {
                busy_wait_ms(1000);
                watchdog_update();
            }
        }

        if (hardResetAttempts >= 5 && connected)
        {
            int64_t startTime = get_absolute_time()._private_us_since_boot;
            gpio_put(MOSFET_BUZZER, true);
            busy_wait_ms(500);
            while (true)
            {
                gpio_put(BUZZER, !gpio_get(BUZZER));
                busy_wait_ms(550);

                uint64_t now = get_absolute_time()._private_us_since_boot;

                if ((startTime + (100 * 10000)) <= now) {
                    watchdog_reboot(0, 0, 0x7fffff);
                    disp.deinit();
                    while (true) {};
                }
                watchdog_update();
            }
        }
    }
    // Signal that Wi-Fi works fine
    watchdog_update();
    printf("Connected!\n");
    wifiConnecting = false;

    disp.clear();
    disp.bmp_show_image_with_offset(__wifi_bmp_data, 226, 5, 5);
    disp.draw_string(50, 13, 1, "Connected!");
    disp.show();

    run_tls_client_test();
    
    busy_wait_ms(5000);
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
