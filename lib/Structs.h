#ifndef STRUCTS_H
#define STRUCTS_H
#include <stdint.h>
#include <stdio.h>

typedef struct {
   uint32_t  input_start_offset;
   uint32_t  input_len;
   uint32_t  input_type_size;
   uint32_t  data_start_offset;
   uint32_t  data_len;
   uint32_t  data_type_size;
   uint32_t  end_offset;
   uint32_t  table_type_size;
   uint32_t  table_len;
   uint32_t mode; //mode 0, all on wram; mode 1, data on mram;
} dpu_arguments_t;


#endif 