//
// Created by PaulekOfficial on 30/05/2023.
//

#include "WiFiManager.h"
#include "hardware/watchdog.h"
#include "utils/utils.h"
#include "tls2_client.h"


bool WiFiManager::connect() {
    info("Turining on wifi module");
    cyw43_arch_enable_sta_mode();

    uint64_t startTime = get_absolute_time()._private_us_since_boot;
    trying_to_connect = true;

    while (link_up != 0)
    {
        busy_wait_ms(2500);
        watchdog_update();
        link_up = cyw43_arch_wifi_connect_timeout_ms(ssid.c_str(), password.c_str(), CYW43_AUTH_WPA2_MIXED_PSK, 6000);
        watchdog_update();

        for (int i = 0; i < 15; i++)
        {
            busy_wait_ms(1000);
            watchdog_update();
        }

        bool hardReset = false;
        wifiConnectionTrys++;

        if (wifiConnectionTrys <= 5 && !link_up)
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

        if (wifiConnectionTrys >= 5 && !link_up)
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

        if (hardResetAttempts >= 5 && !link_up)
        {
            int64_t startTime = get_absolute_time()._private_us_since_boot;
            while (true)
            {
                uint64_t now = get_absolute_time()._private_us_since_boot;

                if ((startTime + (100 * 10000)) <= now) {
                    watchdog_reboot(0, 0, 0x7fffff);
                    return false;
                }
                watchdog_update();
            }
        }
    }
    // Signal that Wi-Fi works fine
    watchdog_update();
    printf("Connected!\n");
    trying_to_connect = false;

    watchdog_update();

    return true;
}

void WiFiManager::disconnect() {
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
}

[[maybe_unused]] char* WiFiManager::http_request(HTTPRequestBuilder requestBuilder) {
    /* No CA certificate checking */
    tls_config = altcp_tls_create_config_client(NULL, 0);

    TLS_CLIENT_T *state = tls_client_init();
    if (!state) {
        return nullptr;
    }

    http_request_string = requestBuilder.build_request();
    portt = requestBuilder.getPort();

    if (!tls_client_open(requestBuilder.getHost(), requestBuilder.getPort(), state)) {
        return nullptr;
    }
    while(!state->complete) {
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }
    free(state);
    altcp_tls_free_config(tls_config);

    return nullptr;
}
