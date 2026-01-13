#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/structs/resets.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

/* 
 * rp2350 uart peripheral memory layout
 */
typedef struct {
    volatile uint32_t dr; // 0x000
    volatile uint32_t rsr; //0x004
    volatile uint32_t RESERVED0[4];  // occupy 16 bytes because there is a gap in memory in datasheet
    volatile uint32_t fr; // 0x018
    volatile uint32_t RESERVED1; // occupy 4 bytes because there is a gap in memory in datasheet
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
    // there are a few more peripheral and cell registers but ill add them later when I need to (lazy)
} UART_t;

/*
 * uart driver functions
 */

uint32_t UART_init(UART_t *uart, uint32_t baud_rate);

// set baud rate on UART peripheral, then return the calculated baud rate
uint32_t UART_clock_get_hz(UART_t *uart);

// rx interrupt handler
void UART_rx_irq_handler();

// not sure why we would ever transmit from an interrupt but maybe :)
void UART_tx_irq_handler();

/*
 * uart peripheral register memory addresses, offsets, and masks
 */

#define UART0 ((UART_t *)UART0_BASE)
#define UART1 ((UART_t *)UART1_BASE)

#define UART_CLOCK_NUM(uart) clk_peri

/*
 * Enables or disables interrupt handlers for UART rx and tx.
 * If enable_rx is true, an interrupt will be fired when RX fifo contains data
 * If enable_tx is true, an interrupt will be fired when the TX fifo needs data
 */
void UART_enable_irqs(UART_t *uart, bool enable_rx, bool enable_tx);

// uartcr register offsets and masks
#define UART_UARTCR_UARTEN_BITS 0x00000001u // uartcr bit 0 uart enable
#define UART_UARTCR_TXE_BITS    0x00000100u // uartcr bit 8 tx enable
#define UART_UARTCR_RXE_BITS    0x00000200u // uartcr bit 9 rx enable
#define UART_UARTCR_LBE_LSB     7u 

// uart lcr_h register offsets and mass=js
#define UART_UARTLCR_H_WLEN_BITS  0x00000060u // uartlcr_h wordlength bits
#define UART_UARTLCR_H_FEN_BITS   0x00000010u // uartlcr_h fifo enable bits
#define UART_UARTLCR_H_WLEN_LSB   5u // 

// uart imsc register offsets and masks
#define UART_UARTIMSC_TXIM_LSB 5u
#define UART_UARTIMSC_RXIM_LSB 4u
#define UART_UARTIMSC_RTIM_LSB 6u

// uart ifls register offsets and masks
#define UART_UARTIFLS_TXIFLSEL_BITS 0x00000007u
#define UART_UARTIFLS_RXIFSEL_BITS 0x00000038u

#define UART0_IRQ 33
#define UART1_IRQ 34
#endif
