
#include <string>
#include <cyw43_ll.h>
#include "utils.h"
#include "http_server/server.h"
#include "dhcp/dhcpserver.h"
#include "dns/dnsserver.h"
#include "main.h"

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


        }
        busy_wait_ms(1000);
    }
}

void awakeWifiSetup() {
    // Setup Wi-Fi
    wifi_manager.init();
    wifi_manager.set_network(ssid, password);
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

void setupEEpromSettings() {
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
}
