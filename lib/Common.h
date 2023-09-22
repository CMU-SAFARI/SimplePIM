#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void print_int(void* value);
void add_int(void* i1, void* i2);
void zero_init(uint32_t value_size, void* value_ptr);
#endif 