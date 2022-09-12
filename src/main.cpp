#include <main.h>

ESP8266 wifi(UART_ID);

int main() {
    //Init pico usb
    stdio_init_all();
    stdio_usb_init();

    //while(!stdio_usb_connected()) {}
    // Wait 20 secounds befoore start
    sleep_ms(20 * 1000);

    info("USB interface initialization done.");

    //Load config from memory
    info("Configuration read from memory...");
    strcpy(configuration.firmware_version, FIRMWARE_VERSION);

    strcpy(configuration.uuid, "");
    strcpy(configuration.name, "");
    strcpy(configuration.secret, "");

    //TODO remove for production
    configuration.internal_r1 = 220000;
    configuration.internal_r2 = 193400;
    configuration.adc_resistance = 0.0f;
    configuration.adc_current = 0.0f;
    configuration.should_sleep = true;
    configuration.sleep_time = 1 * 60 * 60 * 1000;
    configuration.sleep_when_24V_present = true;
    configuration.status_readings = false;
    configuration.read_temperature = true;
    info("Done.");

    //Setup spi
    info("UART interface initialization...");
    setupUart1();

    //Setup adc
    info("ADC interface initialization...");
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_gpio_init(ADC_INTERNAL_BATTERY);
    adc_gpio_init(ADC_BATTERY);
    info("Done.");

    //Setup used pins
    info("GPIO interface initialization...");
    gpio_init(POWER_24V);
    gpio_init(LED);
    gpio_init(BUZZER);
    gpio_init(INTERNAL_BATTERY_MOSFET);

    gpio_set_dir(POWER_24V, GPIO_OUT);
    gpio_set_dir(INTERNAL_BATTERY_MOSFET, GPIO_OUT);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_set_dir(BUZZER, GPIO_OUT);

    gpio_pull_down(POWER_24V);
    gpio_pull_up(BUZZER);

    for (int i = 0; i <= 4; i++) {
        gpio_put(BUZZER, !gpio_get(BUZZER));
        sleep_ms(60);
    }
    gpio_put(BUZZER, true);
    info("Done.");

    // Setup rtc
    rtc_init();

    //Setup watchdog
    watchdog_enable(9 * 1000, true);
    if(watchdog_caused_reboot()) {
        info("ALERT!!! System rebooted by watchdog, investigation required.");
    }

    int wifiConnectionTrys = 0;
wifi_init:
    // Setup WIFI module
    info("WIFI interface initialization...");
    wifi.detectModule();
    wifi.setMode(Station);
    bool connected = wifi.connect("kamex", "K229138621");
    bool pingSuccessful = wifi.pingIp("8.8.8.8");
    if (!pingSuccessful || !connected) {
        wifiConnectionTrys++;
        wifi.resetModule();

        if (wifiConnectionTrys > 5) {
            while (true) {
                int64_t startTime = get_absolute_time();
                while (true) {
                    gpio_put(BUZZER, !gpio_get(BUZZER));
                    sleep_ms(550);

                    uint64_t now = get_absolute_time();

                    if ((startTime + (100 * 1000000)) <= now) {
                        return {};
                    }
                    watchdog_update();
                }
            }
        }
        goto wifi_init;
    }

    watchdog_update();
    gpio_put(LED, true);
    gpio_put(POWER_24V, true);

    info("Entering main loop.");
    while (true) {
        sleep_ms(3000);

        info(to_string((int)wifi.getConnectionStatus()));

        float internalVoltage = readInternalBatteryVoltage(configuration.internal_r1, configuration.internal_r2);
        float voltage = readBatteryVoltage(BATTERY_MOSFET, configuration.r1, configuration.r2);
        float temperature = readInternalTemperature();


        watchdog_update();
    }
}