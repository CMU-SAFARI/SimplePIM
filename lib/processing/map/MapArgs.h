#ifndef MAPARGS_H
#define MAPARGS_H
#include <stdint.h>
#include <stdio.h>

typedef struct {
   uint32_t  input_start_offset;
   uint32_t  input_type_size;
   uint32_t  output_start_offset;
   uint32_t  output_type_size;
   uint32_t  len;
   uint32_t  info;

   // handle virtual zip 
   uint32_t is_virtually_zipped;
   uint32_t start1;
   uint32_t start2;
   uint32_t type1;
   uint32_t type2;
} map_arguments_t;


#endif 