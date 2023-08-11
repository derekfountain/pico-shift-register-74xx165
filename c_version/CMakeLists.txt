cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(shift_reg)

set(CMAKE_C_STANDARD 11)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(shift_reg
	shift_reg.c
)

target_link_libraries(shift_reg pico_stdlib)

pico_enable_stdio_usb(shift_reg 1) 
pico_enable_stdio_uart(shift_reg 0) 

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(shift_reg)

