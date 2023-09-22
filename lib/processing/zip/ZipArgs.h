#ifndef ZIPARGS_H
#define ZIPARGS_H
#include <stdint.h>
#include <stdio.h>

typedef struct {
   uint32_t  input_start_offset1;
   uint32_t  input_start_offset2;
   uint32_t  input_type_size1;
   uint32_t  input_type_size2;
   uint32_t  outputs;
   uint32_t  len;
} zip_arguments_t;


#endif 