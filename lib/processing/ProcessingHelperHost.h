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

// handle_t contains information of where the handle's binary is located
typedef struct {
   char* bin_location; 
   char* so_bin_location;
   uint32_t func_type;
} handle_t;


/*
   create_handle creates a handle that can be understood by communcation and processing operators (see the paper for details)
*/
handle_t* create_handle(const char* func_fname, uint32_t func_type);

#endif 