# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/pico/pico-sdk/tools/pioasm"
  "F:/CLionProjects/interface-pico/cmake-build-release/pioasm"
  "F:/CLionProjects/interface-pico/cmake-build-release/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "F:/CLionProjects/interface-pico/cmake-build-release/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "F:/CLionProjects/interface-pico/cmake-build-release/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "F:/CLionProjects/interface-pico/cmake-build-release/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "F:/CLionProjects/interface-pico/cmake-build-release/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/CLionProjects/interface-pico/cmake-build-release/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/CLionProjects/interface-pico/cmake-build-release/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
