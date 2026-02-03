# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

set(CMAKE_BUILD_TYPE DEBUG)

project(rp2040_ov2640)

pico_sdk_init()

add_executable(firmware
	ethernet.c
	main.c
	ov2640.c
	transfer.c
    network/network.c
    network/drivers/w5500.c
)

pico_generate_pio_header(firmware ${CMAKE_CURRENT_LIST_DIR}/image.pio)

target_link_libraries(firmware
	pico_multicore
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pio
	hardware_pwm
	hardware_spi
)

pico_enable_stdio_usb(firmware 1)
pico_enable_stdio_uart(firmware 1)

target_include_directories(firmware PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(firmware)
