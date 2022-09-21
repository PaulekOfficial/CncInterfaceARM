#include <main.h>
#include <condition_variable>
#include <oled/ssd1306.h>
#include <oled/textRenderer/TextRenderer.h>
#include <hardware/i2c.h>

ESP8266 wifi(UART_ID);

void initConfig();
void initADC();
void initI2C();
void initGPIO();
void initPullups();
void testGPIO();

void setupWiFiModule();
void setupBuzzerModule();
void setupOLED();

int main() {
    //Init pico usb
    stdio_init_all();
    stdio_usb_init();

    // For debug purposes, wait until serial connected
    while(!stdio_usb_connected()) {}

    info("USB interface initialization done.");

    //Load config from memory
    initConfig();

    // Setup spi
    setupUart1();

    // Setup adc
    initADC();

    // Setup i2c register 1
    initI2C();

    //Setup used pins
    initGPIO();

    // Setup pull ups
    initPullups();

    // Set microcontroller indicator on
    info("Mpu set state to on");
    gpio_put(MPU_LED, true);

    // Setup oled display
    setupOLED();

    // Self test
    testGPIO();

    info("Wait 15 sec for system startup");
    sleep_ms(15000);

    // Setup buzzer
    setupBuzzerModule();

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
    info("I2C register 0 interface initialization...");
    i2c_init(i2c0, 4000000);
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
    gpio_put(MOSFET_LCD, true);
    sleep_ms(1000);
    gpio_put(MOSFET_LCD, false);

    sleep_ms(1000);
    gpio_put(MOSFET_BUZZER, true);
    sleep_ms(1000);
    gpio_put(MOSFET_BUZZER, false);

    sleep_ms(1000);
    gpio_put(MOSFET_WIFI, true);
    sleep_ms(1000);
    gpio_put(MOSFET_WIFI, false);

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
    sleep_ms(1000);
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
                gpio_put(MOSFET_BUZZER, true);
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
    gpio_put(MOSFET_WIFI, false);
}

void setupBuzzerModule() {
    info("Testing buzzer...");
    gpio_put(MOSFET_BUZZER, true);
    sleep_ms(1500);
    for (int i = 0; i <= 6; i++) {
        gpio_put(BUZZER, !gpio_get(MOSFET_BUZZER));
        sleep_ms(80);
    }
    gpio_put(BUZZER, false);
    gpio_put(MOSFET_BUZZER, false);
    info("Done.");
}

void setupOLED() {
    info("OLED module initialization...");
    gpio_put(MOSFET_LCD, true);
    sleep_ms(1000);
    auto display = pico_ssd1306::SSD1306(i2c0, 0x3C, pico_ssd1306::Size::W128xH32);
    sleep_ms(250);

    display.setOrientation(0);

    drawText(&display, font_12x16, " PaulekLab", 0 ,0);
    drawText(&display, font_8x8, "CNCINTERFACE", 0 ,24);

    // Send buffer to the display
    display.sendBuffer();
}
