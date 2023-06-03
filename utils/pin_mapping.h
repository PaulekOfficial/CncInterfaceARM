//
// Created by PaulekOfficial on 30/05/2023.
//

#ifndef INTERFACE_PICO_PIN_MAPPING_H
#define INTERFACE_PICO_PIN_MAPPING_H

#define MPU_LED                CYW43_WL_GPIO_LED_PIN
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

#define BUZZER                 10
#define BUTTON                 19
#define V_24_SENSE             6

#define POWER_24V_READY         7

#endif //INTERFACE_PICO_PIN_MAPPING_H
