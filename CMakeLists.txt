cmake_minimum_required(VERSION 3.24)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(interface_pico C CXX ASM)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

#add_executable(interface_pico main.cpp)
add_executable(interface_pico ${SRC_LIST} lwipopts.h
        analog/adc_utils.cpp
        analog/adc_utils.h utils/utils.cpp
        main.h main.cpp
        oled/oled_display.cpp oled/oled_display.h oled/font.h
        sleep/sleep.h sleep/sleep.c hardware/rosc.c hardware/rosc.h mbedtls_config.h wifi/HTTPRequestBuilder.cpp wifi/HTTPRequestBuilder.h wifi/WiFiManager.cpp wifi/WiFiManager.h wifi/tls2_client.cpp wifi/tls2_client.h utils/oledImages.h utils/pin_mapping.h domain/InterfaceMeasurement.cpp domain/InterfaceMeasurement.h domain/CurrentMeasurement.cpp domain/CurrentMeasurement.h domain/SmartInterface.cpp domain/SmartInterface.h dns/dnsserver.cpp dns/dnsserver.h dhcp/dhcpserver.cpp dhcp/dhcpserver.h http_server/server.cpp http_server/server.h eeprom/eeprom.cpp eeprom/eeprom.h)

target_include_directories(interface_pico PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

target_link_libraries(interface_pico
        pico_stdlib
        pico_stdio_usb
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        pico_lwip_mbedtls
        pico_mbedtls
        hardware_flash
        pico_multicore
        hardware_pwm
        hardware_adc
        hardware_spi
        hardware_i2c
        hardware_flash
        hardware_clocks
        hardware_rtc
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(interface_pico 1)
pico_enable_stdio_uart(interface_pico 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(interface_pico)
