#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/structs/uart.h"
#include "hardware/structs/resets.h"
#include "hardware/regs/uart.h"

void custom_uart_init(uart_inst_t *uart, uint32_t baud_rate);
int custom_uart_set_baud_rate(uart_inst_t *art, uint32_t baud_rate);
