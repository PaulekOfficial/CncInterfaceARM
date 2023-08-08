#include "main.h"
#include "hardware/rosc.h"

//TODO DEBUG GPIO 5-2

[[noreturn]] void core1_entry() {
    while (true) {
        if (wifi_manager.connecting()) {
            disp.clear();
            disp.bmp_show_image_with_offset(__wifi_bmp_data, 226, 5, 5);
            disp.draw_string(40, 0, 1, simpleWifiCredentials.ssid.c_str());

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
        }
        if(!watchdogInterrupt) {
            watchdog_update();
        }
        busy_wait_ms(1000);
    }
}

static void send_callback() {
    watchdogInterrupt = true;
}

static void alarm_callback() {
    rtcInterrupt = true;
}

[[noreturn]] void awakeWifiSetup() {
    // Setup Wi-Fi
    wifi_manager.init();
    wifi_manager.set_network(simpleWifiCredentials.ssid, simpleWifiCredentials.password);
    bool wifiSuccess = wifi_manager.connect();

    // Reset
    if (!wifiSuccess) {
        watchdog_enable(1000, false);
        while(true) {}
    }

    if (wifi_manager.connected() == 0) {
        disp.clear();
        disp.bmp_show_image_with_offset(__wifi_bmp_data, 226, 5, 5);
        disp.draw_string(50, 13, 1, "Connected!");
        disp.show();

        busy_wait_ms(2000);
    }
}

void initInfineon() {

    info("WiFi infineon 43439 initializing...");
    if (cyw43_arch_init()) {
        info("WiFi init failed, killing process...");
        return;
    }
    cyw43_wifi_pm(&cyw43_state, CYW43_AGGRESSIVE_PM);
    info("Done.");
}

void initConfigServer() {
    std::string ap_ssid = "SmartInterface_" + gen_random_string(5);

    disp.clear();
    disp.draw_string(0, 0, 1, "KONFIGURACJA");
    disp.draw_string(0, 10, 1, "Polacz sie z siecia");
    disp.draw_string(0, 20, 1, ap_ssid.c_str());
    disp.show();

    cyw43_arch_enable_ap_mode(ap_ssid.c_str(), "", CYW43_AUTH_OPEN);

    TCP_SERVER_T *state = static_cast<TCP_SERVER_T *>(calloc(1, sizeof(TCP_SERVER_T)));

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    if (!tcpServerOpen(state)) {
        DEBUG_printf("failed to open server\n");
        return;
    }

    while(!state->complete) {
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer interrupt) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        // if you are not using pico_cyw43_arch_poll, then Wi-FI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }

    simpleWifiCredentials.ssid = ssid_;
    simpleWifiCredentials.password = password_;
    simpleWifiCredentials.hostname = hostname_;
    simpleWifiCredentials.port = port_;

    disp.clear();
    disp.draw_string(10, 0, 1, "SAVING DATA...");
    disp.show();

    uint trys = 0;
    while (true) {
        info("Saving data to eeprom...");

        memory.writeBoolean(0, true);
        memory.writeString(1, simpleWifiCredentials.ssid);
        memory.writeString(18, simpleWifiCredentials.password);
        memory.writeString(69, simpleWifiCredentials.hostname);
        memory.writeString(89, to_string(simpleWifiCredentials.port));

        auto readSsid = memory.readString(1);
        auto readPassword = memory.readString(18);
        auto readHostname = memory.readString(69);
        auto readPort = atoi(memory.readString(89).c_str());

        if (trys >= 10) {
            watchdog_enable(1, false);
            while (true) {}
        }

        if (readSsid == simpleWifiCredentials.ssid || readPassword == simpleWifiCredentials.password
        || readHostname == simpleWifiCredentials.hostname || readPort == simpleWifiCredentials.port) {
            break;
        }

        trys++;
    }
    info("Saved!");

    tcpServerClose(state);
    dns_server_deinit(&dns_server);
    dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
}

void setupWifiSettings() {
    if (state == CONFIGURATION_WIFI) {
        initConfigServer();

        busy_wait_ms(5000);
        //Init pico wifi module
        initInfineon();

        // Set microcontroller indicator on
        info("Mpu set state to on");
        cyw43_arch_gpio_put(MPU_LED, true);
    }

    // Setup wifi
    wifi_manager.init();
    wifi_manager.set_network(simpleWifiCredentials.ssid, simpleWifiCredentials.password);
    auto wifiSuccess = wifi_manager.connect();

    // Reset
    if (!wifiSuccess) {
        while(true) {}
    }
    watchdog_update();

    if (wifi_manager.connected() == 0) {
        disp.clear();
        disp.bmp_show_image_with_offset(__wifi_bmp_data, 226, 5, 5);
        disp.draw_string(50, 13, 1, "Connected!");
        disp.show();

        busy_wait_ms(2000);
        watchdog_update();
    }
}

void loopShutdown(bool v24Present) {
    if (!v24Present)
    {
        shutdown();

        sleep_run_from_rosc();
        //sleep_goto_dormant_until_pin(BUTTON, true, true);

        setupAlarm();

        sleep = true;
        do {
            watchdog_update();
            bool buttonPressed = gpio_get(BUTTON);
            if (buttonPressed || rtcInterrupt) {
                awake();

                rtcInterrupt = false;
                sleep = false;
            }
        } while (sleep);

        rtc_disable_alarm();
    }
}

void awake() {
    disp.init(128, 32, 0x3C, I2C_ID);
    disp.poweron();

    disp.clear();
    disp.draw_string(10, 0, 1, "Waking up...");
    disp.show();

    //watchdog_enable(1, false);

    recover_from_sleep(simpleClocks.scb_orig, simpleClocks.clock0_orig, simpleClocks.clock1_orig);

    info("WiFi infineon 43439 reinitializing...");
    if (cyw43_arch_init()) {
        info("WiFi init failed, killing process...");

        disp.clear();
        disp.draw_string(10, 10, 1, "Infineon 43439");
        disp.draw_string(10, 20, 1, "ERROR");
        disp.show();
        return;
    }
    info("Done.");

    for(int i = 0; i <= 6; i++) {
        cyw43_arch_gpio_put(MPU_LED, !cyw43_arch_gpio_get(MPU_LED));
        busy_wait_ms(500);
    }
    busy_wait_ms(2000);

    multicore_launch_core1(core1_entry);

    awakeWifiSetup();
}

void shutdown() {
    info("Shutting down...");
    gpio_put(RELAY_BAT_0, false);
    gpio_put(RELAY_BAT_1, false);
    gpio_put(RELAY_POWER_24V, false);
    gpio_put(MOSFET_BUZZER, false);
    disp.poweroff();
    disp.deinit();

    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    cyw43_arch_gpio_put(MPU_LED, false);
    cyw43_arch_deinit();
    gpio_put(MOSFET_LCD, false);
}

void setupWatchdogAlarm() {
    printf("RTC Alarm Watchdog Setting!\n");
    watchdogInterrupt = false;
    datetime_t t = {
            .year  = 2023,
            .month = 01,
            .day   = 13,
            .dotw  = 3,
            .hour  = 11,
            .min   = 00,
            .sec   = 00
    };
    rtc_set_datetime(&t);

    // Alarm time
    datetime_t alarm = {
            .year  = 2023,
            .month = 01,
            .day   = 13,
            .dotw  = 3,
            .hour  = 11,
            .min   = 15,
            .sec   = 00
    };

    rtc_set_alarm(&alarm, &send_callback);
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
            .min   = 00,
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
            .min   = 02,
            .sec   = 00
    };

    //rtc_set_alarm(&alarm, &alarm_callback);
    sleep_goto_sleep_until(&alarm, &alarm_callback);
}

void setupEEpromSettings() {
    setupWatchdogAlarm();

    if (gpio_get(BUTTON)) {
        uint64_t startTime = get_absolute_time()._private_us_since_boot;
        while (gpio_get(BUTTON)) {
            uint64_t now = get_absolute_time()._private_us_since_boot;
            watchdog_update();

            std::stringstream in;
            in << "in ";
            in << to_string((now - startTime) / 1000 - 10);

            disp.clear();
            disp.draw_string(10, 0, 1, "ERASE MEMORY");
            disp.draw_string(10, 10, 1, in.str().c_str());
            disp.show();

            if ((startTime + 10000) <= now) {
                disp.clear();
                disp.draw_string(10, 0, 1, "ERASING...");
                disp.show();

                for (int t = 0; t < 3; t++) {
                    for(int i = 0; i < 128; i++) {
                        watchdog_update();
                        memory.write(i, 0x00);
                    }
                }

                break;
            }

        }
    }


    // Check if data stored in eeprom
    if (!memory.readBoolean(0)) {
        state = CONFIGURATION_WIFI;
    } else {
        // ssid 16 chars
        // password 30 chars
        // hostname 20 chars
        // port 4 chars
        // add to offset 1 because of 0x00 end of string

        //load from memory
        info("Loading data from memory...");
        simpleWifiCredentials.ssid = memory.readString(1);
        simpleWifiCredentials.password = memory.readString(18);
        simpleWifiCredentials.hostname = memory.readString(69);
        simpleWifiCredentials.port = atoi(memory.readString(89).c_str());
        info("Loaded!");

        printf("Loaded data: \n ssid: %s \n password: %s \n hostname: %s \n port: %d \n", simpleWifiCredentials.ssid.c_str(),
               simpleWifiCredentials.password.c_str(), simpleWifiCredentials.hostname.c_str(), simpleWifiCredentials.port);

        bool v24detect = gpio_get(V_24_SENSE);
        if (v24detect) {
            gpio_put(RELAY_POWER_24V, true);
            state = RUNNING_V24;
        }

        if (!v24detect) {
            state = RUNNING_BATTERY;
        }
    }

    rtc_disable_alarm();
}


int main() {
    //Init pico usb
    stdio_init_all();
    //stdio_usb_init();
    info("USB interface initialization done.");
    busy_wait_ms(5000);

    //Init pico wifi module
    initInfineon();

    // Set microcontroller indicator on
    info("Mpu set state to on");
    for(int i = 0; i <= 24; i++) {
        cyw43_arch_gpio_put(MPU_LED, !cyw43_arch_gpio_get(MPU_LED));
        busy_wait_ms(100);
    }

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
    internalBattery = readInternalBatteryVoltage();

    disp.init(128, 32, 0x3C, I2C_ID);
    disp.poweron();

    //TODO should be set by web interface
    if (internalBattery <= 3.80) {
        disp.disableDisplay();
    }

    disp.clear();
    disp.bmp_show_image(__paulek_bmp_data, 1086);
    disp.show();

    busy_wait_ms(2000);

    // Setup rtc
    info("RTC clock initialization...");
    rtc_init();

    // Save clock speed
    simpleClocks.scb_orig = scb_hw->scr;
    simpleClocks.clock0_orig = clocks_hw->sleep_en0;
    simpleClocks.clock1_orig = clocks_hw->sleep_en1;

    // Setup watchdog
    info("Watchdog system initialization...");
    //watchdog_enable(9 * 1000, true);

    multicore_launch_core1(core1_entry);
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
    disp.draw_string(10, 10, 1, "Trwa pomiar");
    disp.draw_string(20, 20, 1, "Prosze czekac");
    disp.show();

    internalBattery = readInternalBatteryVoltage();
    info("Internal voltage: " + to_string(internalBattery));
    watchdog_update();


    double batteryVoltage0 = readBatteryVoltage(RELAY_BAT_0);
    info("Battery0 voltage: " + to_string(batteryVoltage0));


    double batteryVoltage1 = readBatteryVoltage(RELAY_BAT_1);
    info("Battery1 voltage: " + to_string(batteryVoltage1));


    //TODO should be set by web interface
    if (internalBattery <= 3.80) {
        disp.disableDisplay();
    } else {
        disp.enableDisplay();
    }

    float temperature = readInternalTemperature();
    bool externalVoltagePresent = gpio_get(POWER_24V_READY);
    watchdog_update();

    std::list<CurrentMeasurement> measurements;
    CurrentMeasurement internalBatteryMeasurement("0", INTERNAL_BATTERY_VOLTAGE, internalBattery);
    measurements.push_back(internalBatteryMeasurement);
    watchdog_update();

    CurrentMeasurement temp("0", TEMPERATURE, temperature);
    measurements.push_back(temp);
    watchdog_update();

    CurrentMeasurement battery0("0", BATTERY_VOLTAGE, batteryVoltage0);
    measurements.push_back(battery0);
    watchdog_update();

    CurrentMeasurement battery1("1", BATTERY_VOLTAGE, batteryVoltage1);
    measurements.push_back(battery1);
    watchdog_update();

    InterfaceMeasurement interfaceMeasurement("352da5cf-7e92-45ca-88a5-639e5dc2f592", BATTERY_MODE, measurements);

    HTTPRequestBuilder requestBuilder(simpleWifiCredentials.hostname.data(), simpleWifiCredentials.port, "keep-alive", POST, "/smart-interface/measurement", JSON);
    requestBuilder.setPayload(interfaceMeasurement.serialize());
    info(requestBuilder.build_request());

    disp.clear();
    disp.draw_string(10, 10, 1, "Wysylanie Danych");
    disp.draw_string(20, 20, 1, "do serwera");
    disp.show();

    setupWatchdogAlarm();
    WiFiManager::http_request(requestBuilder);
    rtc_disable_alarm();

    if (internalBattery >= 3.80) {
        writeInfo(temperature, batteryVoltage0, batteryVoltage1, internalBattery, externalVoltagePresent);
    }

    loopShutdown(externalVoltagePresent);

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
    watchdog_update();
    sprintf(subtitle2, "Bateria 0: %2.2fV", batteryVoltage0);

    busy_wait_ms(2000);
    watchdog_update();
    sprintf(subtitle3, "Bateria 1: %2.2fV", batteryVoltage1);

    disp.clear();
    disp.draw_string(90, 0, 1, title);
    disp.draw_string(0, 0, 1, subtitle);
    disp.draw_string(0, 10, 1, subtitle2);
    disp.draw_string(0, 20, 1, subtitle3);
    disp.show();

    
    busy_wait_ms(3000);
    watchdog_update();
    

    if (highVoltagePresent) {
        sprintf(subtitle, "Zasilanie 24V");
    } else {
        sprintf(title, "Praca na baterii");
        sprintf(subtitle, "Przechodze w tryb uspienia");
    }

    disp.clear();
    disp.draw_string(0, 0, 1, title);
    disp.draw_string(0, 10, 1, subtitle);
    disp.show();

    
    busy_wait_ms(2000);
    watchdog_update();
}
