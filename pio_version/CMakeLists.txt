cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(shift_reg_pio)

set(CMAKE_C_STANDARD 11)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(shift_reg_pio
	shift_reg_pio.c
)

pico_generate_pio_header(shift_reg_pio ${CMAKE_CURRENT_LIST_DIR}/shift_reg.pio)

target_link_libraries(shift_reg_pio pico_stdlib hardware_pio hardware_dma)

pico_enable_stdio_usb(shift_reg_pio 1) 
pico_enable_stdio_uart(shift_reg_pio 0) 

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(shift_reg_pio)

