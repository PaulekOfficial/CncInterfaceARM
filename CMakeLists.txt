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

# Set Debug build compiler arguments
set(CMAKE_CXX_FLAGS_DEBUG "-pipe -g -O0 -Wfatal-errors -Wpedantic -Wall -Wextra -Wconversion -Wshadow=local -Wdouble-promotion -Wformat=2 -Wformat-overflow=2 -Wformat-nonliteral -Wformat-security -Wformat-truncation=2 -Wnull-dereference -Wimplicit-fallthrough=3 -Wshift-overflow=2 -Wswitch-default -Wunused-parameter -Wunused-const-variable=2 -Wstrict-overflow=4 -Wstringop-overflow=3 -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wmissing-noreturn -Wsuggest-attribute=malloc -Wsuggest-attribute=format -Wmissing-format-attribute -Wsuggest-attribute=cold -Walloc-zero -Walloca -Wattribute-alias=2 -Wduplicated-branches -Wcast-qual")
set(CMAKE_C_FLAGS_DEBUG "-pipe -g -O0 -Wfatal-errors -Wpedantic -Wall -Wextra -Wconversion -Wshadow=local -Wdouble-promotion -Wformat=2 -Wformat-overflow=2 -Wformat-nonliteral -Wformat-security -Wformat-truncation=2 -Wnull-dereference -Wimplicit-fallthrough=3 -Wshift-overflow=2 -Wswitch-default -Wunused-parameter -Wunused-const-variable=2 -Wstrict-overflow=4 -Wstringop-overflow=3 -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wmissing-noreturn -Wsuggest-attribute=malloc -Wsuggest-attribute=format -Wmissing-format-attribute -Wsuggest-attribute=cold -Walloc-zero -Walloca -Wattribute-alias=2 -Wduplicated-branches -Wcast-qual")

# Set Release build compiler arguments
set(CMAKE_CXX_FLAGS_RELEASE "-pipe -Os -fno-builtin")
set(CMAKE_C_FLAGS_RELEASE "-pipe -Os -fno-builtin")

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
        src/wifi/HttpClient.cpp src/wifi/HttpClient.h src/oled/frameBuffer/FrameBuffer.cpp src/oled/frameBuffer/FrameBuffer.h
        src/oled/shapeRenderer/ShapeRenderer.cpp src/oled/shapeRenderer/ShapeRenderer.h src/oled/textRenderer/5x8_font.h
        src/oled/textRenderer/8x8_font.h src/oled/textRenderer/12x16_font.h src/oled/textRenderer/16x32_font.h
        src/oled/textRenderer/TextRenderer.cpp src/oled/textRenderer/TextRenderer.h src/oled/ssd1306.h src/oled/ssd1306.cpp)
