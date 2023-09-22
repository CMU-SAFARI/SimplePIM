#ifndef PARAM_H
#define PARAM_H

#include <stdlib.h>
#include <stdint.h>
uint32_t print_info = 0;
typedef int T; 
const uint32_t dpu_number = 5; // 2432
const uint32_t dim = 10;
const uint64_t num_elements = 1000*dpu_number;
const uint32_t iter = 1;
const float lr = 1e-4;
const uint32_t prevent_overflow_shift_amount = 3;
const uint32_t shift_amount = 5;
#endif
