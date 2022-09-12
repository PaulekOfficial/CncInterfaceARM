#ifndef CNCSMARTINTERFACE_MAIN_H
#define CNCSMARTINTERFACE_MAIN_H

#endif //CNCSMARTINTERFACE_MAIN_H

#include <pico/stdlib.h>
#include <string>
#include <cstring>
#include <utils/utils.h>
#include <hardware/flash.h>
#include <hardware/adc.h>
#include <hardware/rtc.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>
#include <pico/stdio_usb.h>
#include <hardware/regs/intctrl.h>
#include <hardware/irq.h>
#include <wifi/ESP8266.h>
#include <utils/utils.h>
#include <config/configuration.h>
#include <analog/adc_utils.h>

using namespace std;

#define FIRMWARE_VERSION     "v0.1"

#define POWER_24V               16
#define BUZZER                  15
#define INTERNAL_BATTERY_MOSFET 14
#define BATTERY_MOSFET          13
#define ADC_BATTERY             27
#define ADC_INTERNAL_BATTERY    28
#define LED                     25
