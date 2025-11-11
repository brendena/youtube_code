cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
project(i2c_example C CXX ASM)
pico_sdk_init()

add_executable(i2c_example i2c_hid.c)


# Add the standard library to the build
target_link_libraries(i2c_example pico_stdlib)

# Add any user requested libraries
target_link_libraries(i2c_example
    pico_i2c_slave
    hardware_i2c
    pico_stdlib
        )

pico_add_extra_outputs(i2c_example)
