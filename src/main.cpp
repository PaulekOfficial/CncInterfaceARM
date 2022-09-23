#include <main.h>
#include <condition_variable>
#include <hardware/i2c.h>
#include <pthread.h>

ESP8266 wifi(UART_ID);

void initConfig();
void initADC();
void initI2C();
void initGPIO();
void initPullups();
void testGPIO();

void setupWiFiModule();
void scanI2CBus();

int main() {
    //Init pico usb
    stdio_init_all();
    stdio_usb_init();

    // For debug purposes, wait until serial connected
    //while(!stdio_usb_connected()) {}
    sleep_ms(10000);

    info("USB interface initialization done.");

    //Load config from memory
    initConfig();

    // Setup spi
    setupUart();

    // Setup adc
    initADC();

    //Setup used pins
    initGPIO();

    // Setup i2c register 1
    initI2C();

    // Setup pull ups
    initPullups();

    // Self test
    testGPIO();

    // Set microcontroller indicator on
    info("Mpu set state to on");
    gpio_put(MPU_LED, true);
    gpio_put(MOSFET_LCD, true);
    gpio_put(MOSFET_WIFI, true);
    sleep_ms(10000);

    i2c_write_blocking(I2C_ID, 0x7c>>1, reinterpret_cast<const uint8_t *>(0x20 | 0x04), 2, false);
    sleep_ms(1000);
    i2c_write_blocking(I2C_ID, 0x7c>>1, reinterpret_cast<const uint8_t *>(0x20 | 0x04), 2, false);
    sleep_ms(1000);
    i2c_write_blocking(I2C_ID, 0x7c>>1, reinterpret_cast<const uint8_t *>(0x20 | 0x04), 2, false);
    sleep_ms(1000);
    i2c_write_blocking(I2C_ID, 0x7c>>1, reinterpret_cast<const uint8_t *>(0x20 | 0x04), 2, false);
    uint8_t command = 0x04 | 0x00 | 0x00 | 0x04;

    i2c_write_blocking(I2C_ID, 0x7c>>1, reinterpret_cast<const uint8_t *>(0x20 | command), 2, false);

    scanI2CBus();

    info("Wait 15 sec for system startup");
    sleep_ms(15000);

    // Setup rtc
    info("RTC clock initialization...");
    rtc_init();

    // Setup watchdog
    info("Watchdog system initialization...");
    watchdog_enable(9 * 1000, true);
    if(watchdog_caused_reboot()) {
        info("ALERT!!! System rebooted by watchdog, investigation required.");
    }

    watchdog_update();
    //gpio_put(POWER_24V, true);

    // Setup wifi
    setupWiFiModule();

    info("Entering main loop.");
    while (true) {
        sleep_ms(3000);

//        info(to_string((int)wifi.getConnectionStatus()));
//
//        float internalVoltage = readInternalBatteryVoltage(configuration.internal_r1, configuration.internal_r2);
//        float voltage = readBatteryVoltage(BATTERY_MOSFET, configuration.r1, configuration.r2);
//        float temperature = readInternalTemperature();


        watchdog_update();
    }
}

void initConfig() {
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
}

void initADC() {
    info("ADC interface initialization...");
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_gpio_init(ADC_SYSTEM_BATTERY);
    adc_gpio_init(ADC_EXTERNAL_BATTERY_0);
    adc_gpio_init(ADC_EXTERNAL_BATTERY_1);
    info("Done.");
}
void initI2C() {
    info("I2C register interface initialization...");
    i2c_init(I2C_ID, 1000000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}
void initGPIO() {
    info("GPIO interface initialization...");
    gpio_init(RELAY_POWER_24V);
    gpio_init(RELAY_BAT_0);
    gpio_init(RELAY_BAT_1);
    gpio_init(MOSFET_WIFI);
    gpio_init(MOSFET_BUZZER);
    gpio_init(MOSFET_LCD);
    gpio_init(MPU_LED);
    gpio_init(BUZZER);
    gpio_set_dir(RELAY_POWER_24V, GPIO_OUT);
    gpio_set_dir(RELAY_BAT_0, GPIO_OUT);
    gpio_set_dir(RELAY_BAT_1, GPIO_OUT);
    gpio_set_dir(MOSFET_WIFI, GPIO_OUT);
    gpio_set_dir(MOSFET_BUZZER, GPIO_OUT);
    gpio_set_dir(MOSFET_LCD, GPIO_OUT);
    gpio_set_dir(MPU_LED, GPIO_OUT);
    gpio_set_dir(BUZZER, GPIO_OUT);
}
void testGPIO() {
    info("System mosfets/relays self test...");
    sleep_ms(1000);
    gpio_put(RELAY_BAT_0, true);
    sleep_ms(1000);
    gpio_put(RELAY_BAT_0, false);

    sleep_ms(1000);
    gpio_put(RELAY_BAT_1, true);
    sleep_ms(1000);
    gpio_put(RELAY_BAT_1, false);

    sleep_ms(1000);
    gpio_put(RELAY_POWER_24V, true);
    sleep_ms(1000);
    gpio_put(RELAY_POWER_24V, false);
    info("Done.");
}

void initPullups() {
    info("Pull ups initialization...");
    gpio_pull_up(BUZZER);
}

void setupWiFiModule() {
    info("Turining on wifi module");
    gpio_put(MOSFET_WIFI, true);
    sleep_ms(5000);
    int wifiConnectionTrys = 0;
    int hardResetAttempts = 0;
    wifi_init:
    // Setup WIFI module
    info("WIFI interface initialization...");
    wifi.detectModule();
    wifi.setMode(Station);
    bool connected = wifi.connect("MikroTik-Kamex", "K229138621");
    bool pingSuccessful = wifi.pingIp("8.8.8.8");
    if (!pingSuccessful || !connected) {
        wifiConnectionTrys++;
        auto success = wifi.resetModule();

        if (!success && hardResetAttempts >= 5) {
            info("System total failure, waiting for watchdog to reset.");
            sleep_ms(100000);
        }

        if (!success) {
            info("ESP8266 soft reset fail, performing hard reset.");
            gpio_put(MOSFET_WIFI, false);
            sleep_ms(4000);
            gpio_put(MOSFET_WIFI, true);
            sleep_ms(1500);

            watchdog_update();
            hardResetAttempts++;
            goto wifi_init;
        }

        if (wifiConnectionTrys > 5) {
            while (true) {
                int64_t startTime = get_absolute_time();
                gpio_put(MOSFET_BUZZER, true);
                sleep_ms(500);
                while (true) {
                    gpio_put(BUZZER, !gpio_get(BUZZER));
                    sleep_ms(550);

                    uint64_t now = get_absolute_time();

                    if ((startTime + (100 * 1000000)) <= now) {
                        return;
                    }
                    watchdog_update();
                }
            }
        }
        goto wifi_init;
    }

    // Signal that wifi works fine
    gpio_put(MOSFET_BUZZER, true);
    sleep_ms(500);
    gpio_put(BUZZER, true);
    sleep_ms(250);
    gpio_put(BUZZER, false);
    sleep_ms(250);
    gpio_put(BUZZER, true);
    sleep_ms(250);
    gpio_put(BUZZER, false);
    gpio_put(MOSFET_BUZZER, false);
}

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scanI2CBus() {
    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(I2C_ID, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}
