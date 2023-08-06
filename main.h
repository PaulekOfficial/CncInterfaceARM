#ifndef CNCSMARTINTERFACE_MAIN_H
#define CNCSMARTINTERFACE_MAIN_H

#endif //CNCSMARTINTERFACE_MAIN_H

#include <stdlib.h>
#include "oled/oled_display.h"
#include <pico/stdlib.h>
#include <string>
#include <cstring>
#include "utils/utils.h"
#include <hardware/flash.h>
#include <hardware/adc.h>
#include <hardware/rtc.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>
#include <pico/stdio_usb.h>
#include <hardware/clocks.h>
#include <hardware/regs/intctrl.h>
#include "pico/cyw43_arch.h"
#include <hardware/irq.h>
#include "hardware/pwm.h"
#include "analog/adc_utils.h"
#include <condition_variable>
#include "pico/multicore.h"
#include "utils/utils.h"
#include "sleep/sleep.h"
#include <fcntl.h>
#include <hardware/structs/scb.h>
#include <hardware/structs/clocks.h>
#include "utils/oledImages.h"
#include "wifi/WiFiManager.h"
#include <utils/pin_mapping.h>
#include "domain/CurrentMeasurement.h"
#include "domain/InterfaceMeasurement.h"
#include "dhcp/dhcpserver.h"
#include "dns/dnsserver.h"
#include "lwip/init.h"
#include "http_server/server.h"
#include "eeprom/eeprom.h"
#include "utils/configurationAssist.cpp"
#include "utils/sleepUtils.cpp"

using namespace std;

#define FIRMWARE_VERSION     "v0.1"

enum State {
    CONFIGURATION_WIFI,
    CONFIGURATION_HARDWARE,
    RUNNING_BATTERY,
    RUNNING_V24
};

uint scb_orig;
uint clock0_orig;
uint clock1_orig;

State state;

std::string ssid;
std::string password;

std::string hostname;
uint port;

WiFiManager wifi_manager;
eeprom memory(0x50, I2C_ID);

bool wakeUp = false;
bool sleep = false;
double batteryVoltage = 4.5;

int dots = 0;
bool goingUp = false;

[[noreturn]] void loop();
void shutdown();
void awake();
void setupAlarm();

void writeInfo(double temperature, double batteryVoltage0, double batteryVoltage1, double internalBattery, bool highVoltagePresent);

oled_display disp;
