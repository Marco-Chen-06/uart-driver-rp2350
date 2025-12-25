#include "uart_driver.h"

int main() { 
    UART_init(UART0, 115200);
    // UART_init((UART_t * )0x40070000UL, 115200) // equivalent of above line for learning purposes
    uint8_t count = 0;
    // just there because i want something to do while gdbing
    while (count < 100) {
        count++;
    }
 
}


void UART_init(UART_t *uart, uint32_t baud_rate) {
    // consider implementing resets.h and gpio.h myself but it sounds like a lot of work
    // also this only applies to UART0 so look into that too
    hw_clear_bits(&resets_hw->reset, RESETS_RESET_UART0_BITS);
    // i think this busy wait is ok, but I should consider improving it
    while (!(resets_hw->reset_done & RESETS_RESET_UART0_BITS)) {
        // do nothing
    }

    // baud rate calculations
    uint32_t baud = 0;
    // TODO: IMPLEMENT UART_CLOCK_GET_HZ, then baud rate section is finished
    uint32_t baud_rate_div = (8 * uart_clock_get_hz(uart) / baud_rate) + 1;
    
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

    // TODO: haven't tested if this is the correct way to access memory yet
    uart->ibrd = baud_ibrd;
    uart->fbrd = baud_fbrd;

    uart->lcr_h = 0;

    // baud = (4 * uart_clock_get_hz(uart)) / (64 * baud_ibrd + baud_fbrd);

    // // TODO: MAKE GOOD CONSTANTS FOR BITS
    // // allow 8 data bits per frame, and enable TX & RX FIFOs
    // uart->lcr_h = UART_UARTLCR_H_WLEN_BITS, UART_UARTLCR_H_FEN_BITS;

    // // enable uart peripheral, and TX & RX bits
    // uart->cr = UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS | UART_UARTCR_RXE_BITS;
}

//  ---- EVERYTHING BELOW IS OLD CODE THAT IM ONLY LEAVING FOR REFERENCE!!

// come up with better naming convention too

// void custom_uart_init(uart_inst_t *uart, uint32_t baud_rate) {
//     // don't let code compile if uart doens't point to an actual uart peripheral
//     invalid_params_if(HARDWARE_UART, uart != uart0 && uart != uart1);

//     // gotta find a better way to do this, than a busy wait
//     hw_clear_bits(&resets_hw->reset, RESETS_RESET_UART0_BITS);
//     while (!(resets_hw->reset_done & RESETS_RESET_UART0_BITS)) {
//         // do nothing
//     }

//     uint32_t baud = custom_uart_set_baud_rate(uart, baud_rate);

//     // allow 8 data bits per frame, and enable TX & RX FIFOs
//     uart_get_hw(uart)->lcr_h = UART_UARTLCR_H_WLEN_BITS, UART_UARTLCR_H_FEN_BITS;

//     // enable uart peripheral, and TX & RX bits
//     uart_get_hw(uart)->cr = UART_UARTCR_UARTEN_BITS | UART_UARTCR_TXE_BITS | UART_UARTCR_RXE_BITS;
// }

// int custom_uart_set_baud_rate(uart_inst_t *uart, uint32_t baud_rate) {
//     // don't let code compile if baudrate is 0
//     invalid_params_if(HARDWARE_UART, baud_rate == 0);
//     uint32_t baud_rate_div = (8 * uart_clock_get_hz(uart) / baud_rate) + 1;
    

//     // integer part of baud rate divisor
//     uint32_t baud_ibrd = baud_rate_div >> 7; 

//     // float part of baud rate divisor
//     uint32_t baud_fbrd;

//     if (baud_ibrd == 0) {
//         baud_ibrd = 1;
//         baud_fbrd = 0;
//     } else if (baud_ibrd >= 65535) {
//         baud_ibrd = 65535;
//         baud_fbrd = 0;
//     } else {
//         baud_fbrd = (baud_rate_div & 0x7f) >> 1;
//     }
    
//     uart_get_hw(uart)->ibrd = baud_ibrd;
//     uart_get_hw(uart)->fbrd = baud_fbrd;


//     uart_get_hw(uart)->lcr_h = 0;

//     return (4 * uart_clock_get_hz(uart)) / (64 * baud_ibrd + baud_fbrd);
// }