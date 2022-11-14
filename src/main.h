#ifndef CNCSMARTINTERFACE_MAIN_H
#define CNCSMARTINTERFACE_MAIN_H

#endif //CNCSMARTINTERFACE_MAIN_H

#include <stdlib.h>
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
#include <config/configuration.h>
#include <analog/adc_utils.h>
#include <lcd/rgb_lcd.h>
#include <condition_variable>
#include <hardware/i2c.h>
#include <wifi/ESP8266.h>
#include <utils/utils.h>
#include "sleep/sleep.h"

using namespace std;

#define FIRMWARE_VERSION     "v0.1"

#define MPU_LED                25
#define I2C_ID                 i2c1
#define I2C_SDA                14
#define I2C_SCL                15

#define ADC_SYSTEM_BATTERY     27
#define ADC_EXTERNAL_BATTERY   26

#define RELAY_POWER_24V        11
#define RELAY_BAT_0            17
#define RELAY_BAT_1            16

#define MOSFET_LCD             13
#define MOSFET_BUZZER          12
#define MOSFET_WIFI            2

#define BUZZER                 10
#define BUTTON                 19

#define POWER_24V_READY         7

uint scb_orig;
uint clock0_orig;
uint clock1_orig;

ESP8266 wifi(UART_ID);
rgb_lcd lcd;

bool alarm = false;

void loop();
void initConfig();
void initADC();
void initI2C();
void initGPIO();
void initPullUps();
void initLcd();
void shutdown();
void awake();
void setupAlarm();

void writeInfo(double temperature, double batteryVoltage0, double batteryVoltage1, double internalBattery, bool highVoltagePresent);
void setupWiFiModule(string ssid, string password);
