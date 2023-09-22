#ifndef GENREDARGS_H
#define GENREDARGS_H
#include <stdint.h>
#include <stdio.h>

typedef struct {
   uint32_t  input_start_offset;
   uint32_t  input_type_size;
   uint32_t  output_start_offset;
   uint32_t  output_type_size;
   uint32_t  len;
   uint32_t  table_len;
   uint32_t info;
} gen_red_arguments_t;


#endif 