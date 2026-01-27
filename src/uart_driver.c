/*
 * Bugs/TODO list:
 * Hello appears like so: \376Hello
 * Where does the 376 come from?
 * 
 * GDB changes results of receive_buf based on if I step through the UART_write_bytes function.
 * If I continue straight through, I get: 
 * "B\225\261\261\275!\225\261\261\275\377 \000 <1012 repeats"
 * If I do step through, I get "\376HelloHello \000 <1012 repeats>"
 * I beleve there is some sort of race condition that debugging changes.
 * I will test this with an oscilloscope to confirm or deny these assumptions.
 * 
 * If I want to debug with print statements like an arduino, what is
 * the best workflow?
 */

#include "uart_driver.h"
#include "register_utils.h"

#define UART_TX_PIN 0 // GP0 UART0 TX
#define UART_RX_PIN 1 // GP1 UART0 RX

#define RECEIVE_BUF_SIZE 16384 // size of receive_buf in bytes
volatile uint8_t receive_buf[BUFSIZ]; // holds the received data
volatile uint32_t receive_buf_index = 0; // indexes receive_buf

// // transmit to pin example
// int main() { 
//     // initialize UART0 with 115200 baud rate
//     uint32_t set_baud = UART_init(UART0, 115200);

//     gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
//     gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

//     // write "Hello" to UART0 peripheral
//     uint8_t send_buf[BUFSIZ];
//     strcpy(send_buf, "Hello");
//     // UART_write_bytes(UART0, send_buf, 5);

//     // infinite loop infinitely write Hello
//     for (;;) {
//         UART_write_bytes(UART0, send_buf, 5);
//         tight_loop_contents();
//     }
// }

// loopback example
int main() { 
    // initialize UART0 with 115200 baud rate
    uint32_t set_baud = UART_init(UART0, 115200);

    // temporarily disabled for loopback testing (uncomment below line when not doing loopback testing)
    // UART_set_hw_flow(UART0, true, true);

    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // set loopback enable bit for testing
    reg_set_bits(&UART0->cr, 1 << UART_UARTCR_LBE_LSB);

    // write "Hello" to UART0 peripheral
    uint8_t send_buf[BUFSIZ];
    strcpy(send_buf, "Hello");
    // goal: write HelloHello into send_buf
    UART_write_bytes(UART0, send_buf, 5);
    UART_write_bytes(UART0, send_buf, 5);

    // issue: differnet behavior based on how GDB steps through code
    while (receive_buf_index < 10) {
        tight_loop_contents();
    }

    for (;;) {
        tight_loop_contents();
    }
}

/* 
  Borrowed from uart.h of the pico sdk.
  Disables UART peripheral and wait long enough for any TX or RX to finish.
  Returns the initally saved state of the UART control register.
 */
static uint32_t UART_disable_before_lcr_write(UART_t *uart) {
    // (The long comment below was written by rp2_common from pico sdk. I left it in because it was useful.)
    // Notes from PL011 reference manual:
    //
    // - Before writing the LCR, if the UART is enabled it needs to be
    //   disabled and any current TX + RX activity has to be completed
    //
    // - There is a BUSY flag which waits for the current TX char, but this is
    //   OR'd with TX FIFO !FULL, so not usable when FIFOs are enabled and
    //   potentially nonempty
    //
    // - FIFOs can't be set to disabled whilst a character is in progress
    //   (else "FIFO integrity is not guaranteed")
    //
    // Combination of these means there is no general way to halt and poll for
    // end of TX character, if FIFOs may be enabled. Either way, there is no
    // way to poll for end of RX character.
    //
    // So, insert a 15 Baud period delay before changing the settings.
    // 15 Baud is comfortably higher than start + max data + parity + stop.
    // Anything else would require API changes to permit a non-enabled UART
    // state after init() where settings can be changed safely.
    uint32_t cr_save = uart->cr;

    if (cr_save & UART_UARTCR_UARTEN_BITS) {
        // clear UARTEN, UARTTXE, and UARTRXE bits
        reg_clear_bits(&uart->cr, UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS | UART_UARTCR_RXE_BITS);

        uint32_t current_ibrd = uart->ibrd;
        uint32_t current_fbrd = uart->fbrd;

        // Note: Maximise precision here. Show working, the compiler will mop this up.
        // Create a 16.6 fixed-point fractional division ratio; then scale to 32-bits.
        uint32_t brdiv_ratio = 64u * current_ibrd + current_fbrd;
        brdiv_ratio <<= 10;
        // 3662 is ~(15 * 244.14) where 244.14 is 16e6 / 2^16
        uint32_t scaled_freq = UART_clock_get_hz(uart) / 3662ul;
        uint32_t wait_time_us = brdiv_ratio / scaled_freq;
        busy_wait_us(wait_time_us);
    }
    return cr_save;
}

/* 
    Perform masked write to lcr_h by disabling UART peripheral before the write.
    This is necessary according to the PL011 reference manual.
 */
static void UART_write_lcr_bits_masked(UART_t *uart, uint32_t values, uint32_t write_mask) {
    uint32_t cr_save = UART_disable_before_lcr_write(uart);
    reg_write_masked(&uart->lcr_h, values, write_mask);
    uart->cr = cr_save;
}

// Sets baud rate of UART peripheral to as close as possible to baud_rate parameter
static uint32_t UART_set_baudrate(UART_t *uart, uint32_t baud_rate) {
    uint32_t baud_rate_div = (8 * UART_clock_get_hz(uart) / baud_rate) + 1;
    
    // integer part of baud rate divisor
    uint32_t baud_ibrd = baud_rate_div >> 7; 

    // float part of baud rate divisor
    uint32_t baud_fbrd;

    if (baud_ibrd == 0) {
        baud_ibrd = 1;
        baud_fbrd = 0;
    } else if (baud_ibrd >= 65535) {
        baud_ibrd = 65535;
        baud_fbrd = 0;
    } else {
        baud_fbrd = (baud_rate_div & 0x7f) >> 1;
    }

    uart->ibrd = baud_ibrd;
    uart->fbrd = baud_fbrd;

    /*
     From the PL011 Reference Manual, Section 3, Page 14
     "The UARTLCR_H, UARTIBRD, and UARTFBRD registers form the single 30-bit
     wide UARTLCR Register that is updated on a single write strobe generated by a
     UARTLCR_H write. So, to internally update the contents of UARTIBRD or
     UARTFBRD, a UARTLCR_H write must always be performed at the end.""

     tldr: do a dummy write to lcr_h to update the UARTIBRD and UARTFBRD registers
     because PL011 reference manual says so
     */
    UART_write_lcr_bits_masked(uart, 0, 0);

    return (4 * UART_clock_get_hz(uart)) / (64 * baud_ibrd + baud_fbrd);
}


uint32_t UART_init(UART_t *uart, uint32_t baud_rate) {
    reg_clear_bits(&resets_hw->reset, RESETS_RESET_UART0_BITS);

    while (!(resets_hw->reset_done & RESETS_RESET_UART0_BITS)) {
        tight_loop_contents();
        // do nothing (I'll try to do better than a busy wait once I get the TX and RX working)
    }

    uint32_t baud = UART_set_baudrate(uart, baud_rate);

    /*
     Set up UART with the following settings:
     stick parity disabled, wlen = 8 bits, fifos enabled, one stop bit, parity disabled, normal use
    */
    reg_write_masked(&uart->lcr_h, 3u << UART_UARTLCR_H_WLEN_LSB | 1u << UART_UARTLCR_H_FEN_LSB, UART_UARTLCR_H_WLEN_BITS | UART_UARTLCR_H_FEN_BITS);

    // enable rx interrupts for uart peripheral. Disable tx because we are sending manually for now.
    UART_set_enable_irqs(uart, true, false);

    // // enable uart peripheral, and TX & RX bits
    uart->cr = UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS | UART_UARTCR_RXE_BITS;

    // attach and enable the RX interrupt handler
    irq_set_exclusive_handler(UART0_IRQ, UART_rx_irq_handler);
    irq_set_enabled(UART0_IRQ, true);

    return baud;
}

// set baud rate on UART peripheral, then return the calculated baud rate
uint32_t UART_clock_get_hz(UART_t *uart) {
    return clock_get_hz(UART_CLOCK_NUM(uart));
}

void UART_set_enable_irqs(UART_t *uart, bool enable_rx, bool enable_tx) {
    // converts enable_rx and tx to a 1 or 0 based on the boolean value
    uint32_t enable_rx_as_bit = enable_rx;
    uint32_t enable_tx_as_bit = enable_tx;
    uart->imsc = (enable_rx_as_bit << UART_UARTIMSC_RXIM_LSB) | 
                 (enable_rx_as_bit << UART_UARTIMSC_RTIM_LSB) | 
                 (enable_tx_as_bit << UART_UARTIMSC_TXIM_LSB);
    
    if (enable_rx) {
        // UARTRXINT triggers when receive fifo becomes >= 1/8 full (minimum setting)
        reg_clear_bits(&uart->ifls, UART_UARTIFLS_RXIFSEL_BITS);
    }
    if (enable_tx) {
        // UARTTXINT triggers when transmit fifo becomes <= 1/8 full (minimum setting)
        reg_clear_bits(&uart->ifls, UART_UARTIFLS_TXIFLSEL_BITS);
    }
}

void UART_set_hw_flow(UART_t *uart, bool enable_cts, bool enable_rts) {
    // converts enable_rx and tx to a 1 or 0 based on the boolean value
    uint32_t enable_cts_as_bit = enable_cts;
    uint32_t enable_rts_as_bit = enable_rts;
    reg_write_masked(&uart->cr, enable_cts_as_bit << UART_UARTCR_CTSEN_LSB | enable_rts_as_bit << UART_UARTCR_RTSEN_LSB,
                     UART_UARTCR_CTSEN_BITS | UART_UARTCR_RTSEN_BITS);
}

// interrupt handler which will fire whenever we receive data
void UART_rx_irq_handler() {
    uint32_t status = UART0->mis;

    /*
     * TODO: Make this a switch case so it's easier to error handle in the future
     */

    // handle rx interrupt case
    if (status & (1u << UART_UARTIMSC_RXIM_LSB)) {
        // keep receiving while RX buffer is not empty
        while (!(UART0->fr & UART_UARTFR_RXFE_BITS))
        {
          // process the character in the data register into receive_buf, but mask it to avoid error bits
          receive_buf[receive_buf_index] = (UART0->dr) & 0xFF;       
          receive_buf_index = (receive_buf_index + 1) % RECEIVE_BUF_SIZE;    
        }
    }

    // handle receive timeout interrupt case
    if (status & (1u << UART_UARTIMSC_RTIM_LSB)) {
        /* From the PL011 Reference Manual, section 2, page 24:
        * The receive timeout interrupt is asserted when the receive FIFO is not empty, and no
        * more data is received during a 32-bit period. The receive timeout interrupt is cleared
        * either when the FIFO becomes empty through reading all the data (or by reading the
        * holding register), or when a 1 is written to the corresponding bit of the Interrupt Clear
        * Register, UARTICR
        */
        UART0->icr = 1u << UART_UARTIMSC_RTIM_LSB; 
        // keep receiving while RX buffer is not empty
        while (!(UART0->fr & UART_UARTFR_RXFE_BITS))
        {
          // process the character in the data register into receive_buf, but mask it to avoid error bits
          receive_buf[receive_buf_index] = (UART0->dr) & 0xFF;       
          receive_buf_index = (receive_buf_index + 1) % RECEIVE_BUF_SIZE;  
        }
    }
}

// write 1 byte for transmission
void UART_write_byte(UART_t *uart, uint8_t byte) {
    uart->dr = byte;
}

// write multiple bytes (a string) for transmission
void UART_write_bytes(UART_t *uart, uint8_t *bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        while(uart->fr & UART_UARTFR_TXFF_BITS) {
            tight_loop_contents();
            // wait for tx fifo to not be full
        }
        uart->dr = *bytes++;
    }
}