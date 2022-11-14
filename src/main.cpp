#include <main.h>
#include <hardware/structs/scb.h>
#include <hardware/structs/clocks.h>

static void alarm_callback() {
    alarm = true;
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

    //Load config from memory
    initConfig();

    // Set microcontroller indicator on
    info("Mpu set state to on");
    gpio_put(MPU_LED, true);
    gpio_put(MOSFET_LCD, true);
    gpio_put(MOSFET_WIFI, true);

    // Setup rtc
    info("RTC clock initialization...");
    rtc_init();

    // Setup lcd
    initLcd();
    lcd.sendMessage("PaulekLab Inc.");
    lcd.setCursor(1, 1);
    lcd.sendMessage("Kamex Interface");
    busy_wait_ms(1000);

    // Save clock speed
    scb_orig = scb_hw->scr;
    clock0_orig = clocks_hw->sleep_en0;
    clock1_orig = clocks_hw->sleep_en1;


    // Setup watchdog
    info("Watchdog system initialization...");
    //watchdog_enable(9 * 1000, false);
    if(watchdog_caused_reboot()) {
        info("ALERT!!! System rebooted by watchdog, investigation required.");
    }
    watchdog_update();

    // Setup wifi
    //setupWiFiModule("Xiaomi_53E3", "102101281026");

    loop();
}

void loop()
{
    info("Entering main loop.");
    while (true) {
        watchdog_update();

        lcd.sendMessages("Obliczanie", "prosze czekac...", 1);

        double internalVoltage = readInternalBatteryVoltage();
        info("Internal voltage: " + to_string(internalVoltage));
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

        writeInfo(temperature, batteryVoltage0, batteryVoltage1, internalVoltage, highVoltagePresent);
        if (!highVoltagePresent) {
            lcd.sendMessages("Brak zasilania", "Tryb uspienia...", 0);
            shutdown();

            sleep_run_from_rosc();
            setupAlarm();

            while (true) {
                bool buttonPressed = gpio_get(BUTTON);
                if (buttonPressed || alarm) {
                    awake();

                    alarm = false;
                    rtc_disable_alarm();
                    break;
                }

                watchdog_update();
            }
        }

        watchdog_update();
    }
}

void writeInfo(double temperature, double batteryVoltage0, double batteryVoltage1, double internalBattery, bool highVoltagePresent) {
    char title[32];
    sprintf(title, "Temp: %2.2f C", temperature);

    char subtitle[32];
    sprintf(subtitle, "VBAT:%2.2fV", internalBattery);

    lcd.sendMessages(title, subtitle, 0);
    busy_wait_ms(2000);
    sprintf(subtitle, "Bateria 0: %2.2fV", batteryVoltage0);
    lcd.sendMessages(title, subtitle, 0);

    busy_wait_ms(2000);
    sprintf(subtitle, "Bateria 1: %2.2fV", batteryVoltage1);
    lcd.sendMessages(title, subtitle, 0);
    busy_wait_ms(2000);

    if (highVoltagePresent) {
        sprintf(subtitle, "Zasilanie 24V");
    } else {
        sprintf(subtitle, "Brak 24V");
    }

    lcd.sendMessages(title, subtitle, 0);
    busy_wait_ms(2000);
}

void awake() {
    gpio_put(MPU_LED, true);
    gpio_put(MOSFET_WIFI, true);
    gpio_put(MOSFET_LCD, true);

    // Setup lcd
    initLcd();
    lcd.display();
    lcd.sendMessages("Wybudzanie", "prosze czekac...", 1);

    busy_wait_ms(1000);
    lcd.sendMessages("System clock", "awakeing...", 1);
    recover_from_sleep(scb_orig, clock0_orig, clock1_orig);

    // Setup wifi
    //setupWiFiModule("Xiaomi_53E3", "102101281026");
}

void shutdown() {
    info("Shutting down...");
    gpio_put(RELAY_BAT_0, false);
    gpio_put(RELAY_BAT_1, false);
    gpio_put(RELAY_POWER_24V, false);
    gpio_put(MOSFET_BUZZER, false);
    gpio_put(MOSFET_WIFI, false);
    busy_wait_ms(1500);
    gpio_put(MOSFET_LCD, false);
    gpio_put(MPU_LED, false);
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
    adc_gpio_init(ADC_EXTERNAL_BATTERY);
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
    busy_wait_ms(1000);
    gpio_put(RELAY_BAT_0, true);
    busy_wait_ms(1000);
    gpio_put(RELAY_BAT_0, false);

    busy_wait_ms(1000);
    gpio_put(RELAY_BAT_1, true);
    busy_wait_ms(1000);
    gpio_put(RELAY_BAT_1, false);

    busy_wait_ms(1000);
    gpio_put(RELAY_POWER_24V, true);
    busy_wait_ms(1000);
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
    busy_wait_ms(5000);
    int wifiConnectionTrys = 0;
    int hardResetAttempts = 0;
    wifi_init:
    // Setup WIFI module
    info("WIFI interface initialization...");
    wifi.detectModule();
    wifi.setMode(Station);

    lcd.sendMessages("Laczenie z wifi", ssid.data(), 1);
    busy_wait_ms(1000);

    bool connected = wifi.connect(ssid, password);
    if (!connected) {
        wifiConnectionTrys++;
        info("Connection failed.");

        // Display status
        lcd.sendMessages("Laczenie z wifi", "Blad polaczenia", 0);

        // Reset wifi module
        auto success = wifi.resetModule();
        watchdog_update();
        busy_wait_ms(4000);
        watchdog_update();

        if (wifiConnectionTrys >= 5) {
            success = false;
        }

        if (!success && hardResetAttempts >= 5) {
            info("System total failure, waiting for watchdog to reset.");

            // Display status
            lcd.sendMessages("Laczenie z wifi", "Blad modulu", 0);

            busy_wait_ms(100000);
        }

        if (!success) {
            info("ESP8266 soft reset fail, performing hard reset.");
            lcd.sendMessages("ERROR C:004", "Restart modulu", 0);

            gpio_put(MOSFET_WIFI, false);
            busy_wait_ms(4000);
            gpio_put(MOSFET_WIFI, true);
            busy_wait_ms(1500);


            watchdog_update();
            busy_wait_ms(8000);
            watchdog_update();
            busy_wait_ms(8000);

            hardResetAttempts++;
            goto wifi_init;
        }

        if (wifiConnectionTrys >= 25 || hardResetAttempts >= 5) {
            while (true) {
                int64_t startTime = get_absolute_time();
                gpio_put(MOSFET_BUZZER, true);
                busy_wait_ms(500);
                while (true) {
                    gpio_put(BUZZER, !gpio_get(BUZZER));
                    busy_wait_ms(550);

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
    busy_wait_ms(500);
    gpio_put(BUZZER, true);
    busy_wait_ms(250);
    gpio_put(BUZZER, false);
    busy_wait_ms(250);
    gpio_put(BUZZER, true);
    busy_wait_ms(250);
    gpio_put(BUZZER, false);
    gpio_put(MOSFET_BUZZER, false);
}

void setupAlarm() {
    printf("RTC Alarm Repeat!\n");

    // Start on Wednesday 13th January 2021 11:20:00
    datetime_t t = {
            .year  = 2020,
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
            .year  = 2020,
            .month = 01,
            .day   = 13,
            .dotw  = 3, // 0 is Sunday, so 3 is Wednesday
            .hour  = 11,
            .min   = 50,
            .sec   = 00
    };

    rtc_set_alarm(&alarm, &alarm_callback);
}
