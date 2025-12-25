cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(uart_driver_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(uart_driver
    src/uart_driver.c
)

target_link_libraries(
    uart_driver
    hardware_gpio
    hardware_timer
    hardware_resets
    hardware_clocks
    pico_platform
)

pico_enable_stdio_usb(uart_driver 0)
pico_enable_stdio_uart(uart_driver 0)

pico_add_extra_outputs(uart_driver)

# add debug flag for gdb
target_compile_definitions(uart_driver PRIVATE PICO_DEBUG=1)
target_compile_options(uart_driver PRIVATE -g3)


