#ifndef PROCESSINGHELPERHOST_H
#define PROCESSINGHELPERHOST_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#define MAP 0
#define REDUCE 1
#define ZIP 2

typedef struct {
   char* bin_location; 
   char* so_bin_location;
   uint32_t func_type;
} handle_t;

//struct timeval start_time;
//struct timeval end_time;

// type 0 is map, type 1 is gen_red, type 2 is zip
handle_t* create_handle(const char* func_fname, uint32_t func_type);

#endif 