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
            disp.draw_string(40, 0, 1, ssid.c_str());

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

    ssid = ssid_;
    password = password_;
    hostname = hostname_;
    port = port_;

    disp.clear();
    disp.draw_string(10, 0, 1, "SAVING DATA...");
    disp.show();

    uint trys = 0;
    while (true) {
        info("Saving data to eeprom...");

        memory.writeBoolean(0, true);
        memory.writeString(1, ssid);
        memory.writeString(18, password);
        memory.writeString(69, hostname);
        memory.writeString(89, to_string(port));

        auto readSsid = memory.readString(1);
        auto readPassword = memory.readString(18);
        auto readHostname = memory.readString(69);
        auto readPort = atoi(memory.readString(89).c_str());

        if (trys >= 10) {
            watchdog_enable(1, false);
            while (true) {}
        }

        if (readSsid == ssid || readPassword == password || readHostname == hostname || readPort == port) {
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

void initInfineon() {

    info("WiFi infineon 43439 initializing...");
    if (cyw43_arch_init()) {
        info("WiFi init failed, killing process...");
        return;
    }
    info("Done.");
}

int main() {
    //Init pico usb
    stdio_init_all();
    stdio_usb_init();
    info("USB interface initialization done.");
    busy_wait_ms(2000);

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
    watchdog_update();

    if (gpio_get(BUTTON)) {
        uint64_t startTime = get_absolute_time()._private_us_since_boot;
        while (gpio_get(BUTTON)) {
            uint64_t now = get_absolute_time()._private_us_since_boot;

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
                        memory.write(i, 0x00);
                    }
                }

                break;
            }
            watchdog_update();
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
        ssid = memory.readString(1);
        password = memory.readString(18);
        hostname = memory.readString(69);
        port = atoi(memory.readString(89).c_str());
        info("Loaded!");

        printf("Loaded data: \n ssid: %s \n password: %s \n hostname: %s \n port: %d \n", ssid.c_str(), password.c_str(), hostname.c_str(), port);

        bool v24detect = gpio_get(V_24_SENSE);
        if (v24detect) {
            gpio_put(RELAY_POWER_24V, true);
            state = RUNNING_V24;
        }

        if (!v24detect) {
            state = RUNNING_BATTERY;
        }
    }

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
    wifi_manager.set_network(ssid, password);
    auto wifiSuccess = wifi_manager.connect();

    // Reset
    if (!wifiSuccess) {
        watchdog_enable(9 * 1000, false);
        while(true) {}
    }

    if (wifi_manager.connected() == 0) {
        disp.clear();
        disp.bmp_show_image_with_offset(__wifi_bmp_data, 226, 5, 5);
        disp.draw_string(50, 13, 1, "Connected!");
        disp.show();

        busy_wait_ms(2000);
    }

    if (state == CONFIGURATION_HARDWARE) {

    }

    loop();
}

void loop()
{
    info("Entering main loop.");
    while (true)
    {
        watchdog_update();

        gpio_put(RELAY_POWER_24V, gpio_get(V_24_SENSE));

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

    // Setup Wi-Fi
    //wifi_manager.set_network(ssid, password);
    auto wifiSuccess = wifi_manager.connect();

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
