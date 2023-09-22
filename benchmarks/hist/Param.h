#ifndef PARAM_H
#define PARAM_H
#include <stdlib.h>
uint32_t print_info = 0;
typedef uint32_t T; 
const uint32_t dpu_number = 3; //2432

#define DEPTH 12 // 2^12 = 4096
#define bins 256

uint64_t nr_elements = dpu_number*128; //64*1536*1024
#endif
