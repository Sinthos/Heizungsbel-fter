# ESP-IDF Project CMakeLists.txt
# Minimal ESP32-C6 Zigbee On/Off Fan Controller
# For use with ESP-IDF v5.1+ and esp-zigbee-sdk

cmake_minimum_required(VERSION 3.16)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32c6_fan_switch)
