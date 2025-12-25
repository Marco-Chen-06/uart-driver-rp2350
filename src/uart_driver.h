#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/structs/resets.h"
#include "hardware/timer.h"

// don't need these because pico already defines them
// #define UART0_BASE 0x40070000UL
// #define UART1_BASE 0x40078000UL

typedef struct {
    volatile uint32_t dr; // 0x000
    volatile uint32_t rsr; //0x004
    volatile uint32_t pad0[4];  // occupy 4 bytes because there is a gap in memory in datasheet
    volatile uint32_t fr; // 0x018
    volatile uint32_t pad1; // occupy 4 bytes because there is a gap in memory in datasheet
    volatile uint32_t ilpr; // 0x020
    volatile uint32_t ibrd;
    volatile uint32_t fbrd;
    volatile uint32_t lcr_h;
    volatile uint32_t cr;
    volatile uint32_t ifls;
    volatile uint32_t imsc;
    volatile uint32_t ris;
    volatile uint32_t mis;
    volatile uint32_t icr;
    volatile uint32_t dmacr;
    // there are a few more peripheral and cell registers but idk what they do and im too lazy to type
} UART_t;

#define UART0 ((UART_t *)UART0_BASE)
#define UART1 ((UART_t *)UART1_BASE)

#define UART_CLOCK_NUM(uart) clk_peri

void UART_init(UART_t *uart, uint32_t baud_rate);

uint32_t UART_clock_get_hz(UART_t *uart) {
    return clock_get_hz(UART_CLOCK_NUM(uart));
}



// int custom_uart_set_baud_rate(uart_inst_t *art, uint32_t baud_rate);
