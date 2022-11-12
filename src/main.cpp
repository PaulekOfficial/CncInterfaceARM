#include <main.h>
#include <condition_variable>
#include <hardware/i2c.h>
#include "lcd/rgb_lcd.h"

ESP8266 wifi(UART_ID);
rgb_lcd lcd;

void initConfig();
void initADC();
void initI2C();
void initGPIO();
void initPullUps();
void testGPIO();
void initLcd();

void setupWiFiModule(string ssid, string password);

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main() {
    //Init pico usb
    stdio_init_all();
    stdio_usb_init();

    info("USB interface initialization done.");

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

    // Self test
    testGPIO();

    //Load config from memory
    initConfig();

    // Set microcontroller indicator on
    info("Mpu set state to on");
    gpio_put(MPU_LED, true);
    gpio_put(MOSFET_LCD, true);
    gpio_put(MOSFET_WIFI, true);
    sleep_ms(1000);

    // Setup rtc
    info("RTC clock initialization...");
    rtc_init();

    // Setup lcd
    initLcd();
    lcd.sendMessage("PaulekLab Inc.");
    lcd.setCursor(1, 1);
    lcd.sendMessage("Kamex Interface");
    sleep_ms(3000);

    // Setup watchdog
    info("Watchdog system initialization...");
    watchdog_enable(9 * 1000, true);
    if(watchdog_caused_reboot()) {
        info("ALERT!!! System rebooted by watchdog, investigation required.");
    }

    watchdog_update();
    //gpio_put(POWER_24V, true);

    // Setup wifi
    setupWiFiModule("Xiaomi_53E3", "102101281026");

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

void initLcd() {
    info("Initializing LCD module...");
    lcd.begin(16, 2, LCD_5x8DOTS, I2C_ID);
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
    i2c_init(I2C_ID, 400000);
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

void initPullUps() {
    info("Pull ups initialization...");
    gpio_pull_up(BUZZER);
}

void setupWiFiModule(string ssid, string password) {
    info("Turining on wifi module");
    lcd.sendMessages("WIFI", "INITIALIZATION", 1);

    gpio_put(MOSFET_WIFI, true);
    sleep_ms(5000);
    int wifiConnectionTrys = 0;
    int hardResetAttempts = 0;
    wifi_init:
    // Setup WIFI module
    info("WIFI interface initialization...");
    wifi.detectModule();
    wifi.setMode(Station);

    lcd.sendMessages("CONNETING WIFI", ssid.data(), 1);
    sleep_ms(1000);

    bool connected = wifi.connect(ssid, password);
    if (!connected) {
        wifiConnectionTrys++;
        info("Connection failed.");

        // Display status
        lcd.sendMessages("CONNETING WIFI", "Connection failed.", 0);

        // Reset wifi module
        auto success = wifi.resetModule();
        watchdog_update();
        sleep_ms(4000);
        watchdog_update();

        if (wifiConnectionTrys >= 5) {
            success = false;
        }

        if (!success && hardResetAttempts >= 5) {
            info("System total failure, waiting for watchdog to reset.");

            // Display status
            lcd.sendMessages("CONNETING WIFI", "Module failure", 0);

            sleep_ms(100000);
        }

        if (!success) {
            info("ESP8266 soft reset fail, performing hard reset.");
            lcd.sendMessages("CONNETING WIFI", "Restart wifi mod", 0);

            gpio_put(MOSFET_WIFI, false);
            sleep_ms(4000);
            gpio_put(MOSFET_WIFI, true);
            sleep_ms(1500);


            watchdog_update();
            sleep_ms(8000);
            watchdog_update();
            sleep_ms(8000);

            hardResetAttempts++;
            goto wifi_init;
        }

        if (wifiConnectionTrys >= 25 || hardResetAttempts >= 5) {
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
    lcd.sendMessages("CONNETING WIFI", "SUCCESS", 0);

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
