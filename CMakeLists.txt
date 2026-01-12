cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(uart_driver_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_executable(uart_driver
    src/uart_driver.c
    src/register_utils.c
)

target_link_libraries(
    uart_driver
    pico_stdlib      
    hardware_gpio
    hardware_timer
    hardware_resets
    hardware_clocks
)

pico_enable_stdio_usb(uart_driver 0)
pico_enable_stdio_uart(uart_driver 0)

pico_add_extra_outputs(uart_driver)

target_compile_definitions(uart_driver PRIVATE 
    PICO_RP2350=1
    PICO_DEBUG=1
)

# be explicit about using rp2350
target_include_directories(uart_driver PRIVATE
    ${PICO_SDK_PATH}/src/rp2350/hardware_regs/include
    ${PICO_SDK_PATH}/src/rp2350/hardware_structs/include
)

target_compile_options(uart_driver PRIVATE -g3)


