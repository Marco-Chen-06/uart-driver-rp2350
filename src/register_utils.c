/*
    register_utils.c
    Helper functions for modifying registers. Meant to improve readability, so 
    that we can avoid haivng to do bitwise operation math in our heads while
    reading the code.
*/

#include "register_utils.h"

void reg_set_bits(volatile uint32_t *address, uint32_t mask) {
    *address |= mask;
}

void reg_clear_bits(volatile uint32_t *address, uint32_t mask) {
    *address &= ~mask;
}

void reg_write_masked(volatile uint32_t *address, uint32_t value, uint32_t mask) {
    // clear all masked bits and then write them
    *address = (*address & ~mask) | (value & mask);
}