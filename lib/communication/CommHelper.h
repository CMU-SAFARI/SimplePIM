#ifndef COMMHELPER_H
#define COMMHELPER_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>
uint32_t calculate_pad_len(uint32_t len, uint32_t type_size, uint32_t num_dpus);
#endif 