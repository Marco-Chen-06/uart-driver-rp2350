#ifndef REGISTER_UTILS_H
#define REGISTER_UTILS_H

#include <stdint.h>

void reg_set_bits(volatile uint32_t *address, uint32_t mask);
void reg_clear_bits(volatile uint32_t *address, uint32_t mask);
void reg_write_masked(volatile uint32_t *address, uint32_t value, uint32_t mask);

#endif