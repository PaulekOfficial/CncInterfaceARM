
#include "main.h"

static void alarm_callback() {
    wakeUp = true;
}

void loopShutdown(bool v24Present) {
    if (!v24Present)
    {
        shutdown();

        sleep_run_from_rosc();
        setupAlarm();

        sleep = true;
        do {
            bool buttonPressed = gpio_get(BUTTON);
            if (buttonPressed || wakeUp) {
                awake();

                wakeUp = false;
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
    disp.draw_string(10, 10, 1, "Waking up...");
    disp.show();

    recover_from_sleep(scb_orig, clock0_orig, clock1_orig);

    info("WiFi infineon 43439 reinitializing...");
    if (cyw43_arch_init()) {
        info("WiFi init failed, killing process...");
        return;
    }
    info("Done.");
    cyw43_arch_gpio_put(MPU_LED, true);

    multicore_reset_core1();
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

    cyw43_arch_deinit();
    gpio_put(MOSFET_LCD, false);
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
            .min   = 01,
            .sec   = 00
    };

    printf("Sleeping for 10 seconds\n");
    uart_default_tx_wait_blocking();

    //rtc_set_alarm(&alarm, &alarm_callback);
    sleep_goto_sleep_until(&alarm, &alarm_callback);
}
