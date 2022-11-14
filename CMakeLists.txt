cmake_minimum_required(VERSION 3.13)

# Include the Raspberry Pi Pico SDK import script
include("pico_sdk_import.cmake")

project("CncSmartInterface" C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set default build type to Release
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    message("Using by default the Release build")
endif ()

include(CMakeListsPrivate.txt)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeListsUser.txt)
include(CMakeListsUser.txt)
endif()

add_custom_target(
    Production ALL
    COMMAND platformio -c clion run "$<$<NOT:$<CONFIG:All>>:-e${CMAKE_BUILD_TYPE}>"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    Debug ALL
    COMMAND platformio -c clion debug "$<$<NOT:$<CONFIG:All>>:-e${CMAKE_BUILD_TYPE}>"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(Z_DUMMY_TARGET ${SRC_LIST} src/analog/adc_utils.cpp src/config/configuration.cpp src/config/configuration.h
        src/analog/adc_utils.h src/utils/utils.cpp src/utils/utils.h src/wifi/ESP8266.h src/wifi/ESP8266.cpp src/main.h
        src/wifi/HttpClient.cpp src/wifi/HttpClient.h src/lcd/rgb_lcd.cpp
        src/lcd/rgb_lcd.h src/sleep/sleep.h src/sleep/sleep.c src/hardware/rosc.c src/hardware/rosc.h)
