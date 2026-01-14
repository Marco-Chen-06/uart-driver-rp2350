#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdio.h>
#include <string.h>
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

// transmit byte
void UART_write_byte(UART_t *uart, uint8_t byte);

// transmit bytes
void UART_write_bytes(UART_t *uart, uint8_t *bytes, size_t len);

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
void UART_set_enable_irqs(UART_t *uart, bool enable_rx, bool enable_tx);

void UART_set_hw_flow(UART_t *uart, bool enable_cts, bool enable_rts);
// uartcr register offsets and masks
#define UART_UARTCR_UARTEN_BITS 0x00000001u // masks bit 0 of uartcr
#define UART_UARTCR_TXE_BITS    0x00000100u // masks bit 8 of uartcr
#define UART_UARTCR_RXE_BITS    0x00000200u // masks bit 9 of uartcr
#define UART_UARTCR_RTSEN_BITS  0x00004000u // masks bit 14 of uartcr
#define UART_UARTCR_CTSEN_BITS  0x00008000u // masks bit 15 of uartcr
#define UART_UARTCR_LBE_LSB     7u 
#define UART_UARTCR_RTSEN_LSB   14u
#define UART_UARTCR_CTSEN_LSB   15u

// uart lcr_h register offsets and masks
#define UART_UARTLCR_H_WLEN_BITS  0x00000060u // masks bits 5-6 of uartlcr_h
#define UART_UARTLCR_H_FEN_BITS   0x00000010u // masks bit 4 of uartlcr_h
#define UART_UARTLCR_H_WLEN_LSB   5u  
#define UART_UARTLCR_H_FEN_LSB    4u


// uart imsc register offsets and masks
#define UART_UARTIMSC_TXIM_LSB 5u
#define UART_UARTIMSC_RXIM_LSB 4u
#define UART_UARTIMSC_RTIM_LSB 6u

// uart ifls register offsets and masks
#define UART_UARTIFLS_TXIFLSEL_BITS 0x00000007u // masks bits 0-3 of uartifls
#define UART_UARTIFLS_RXIFSEL_BITS 0x00000038u // masks bits 3-5 of uartifls

// uart fr register offsets and masks
#define UART_UARTFR_TXFF_BITS 0x00000020u // masks bit 5 of uartfr
#define UART_UARTFR_RXFE_BITS 0x00000010u // masks bit 4 of uartfr

#define UART0_IRQ 33 // UART0 IRQ number from vector table
#define UART1_IRQ 34 // UART1 IRQ number from vector table
#endif
