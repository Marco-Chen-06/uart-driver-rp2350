# uart-driver-rp2350
Interrupt-driven UART driver implementation for the rp2350. 

Writen in bare-metal, without uart.h and minimal pico-sdk usage.

Receiving data is handled through interrupts. At the moment, transmitting data is not handled through interrupts.

This project is not completely finished nor strongly error tolerant. 
