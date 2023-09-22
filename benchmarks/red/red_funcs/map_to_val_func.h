#ifndef MAP_TO_VAL_FUNC_H
#define MAP_TO_VAL_FUNC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <mram.h>
#include <alloc.h>
#include <defs.h>
#include <barrier.h>

#include "../Param.h"
#include "../../../lib/processing/gen_red/GenRedArgs.h"


void start_func(gen_red_arguments_t* args){
    
}

void map_to_val_func(void* input, void* output, uint32_t* key){
    *key = 0;
    *(T*)output = *(T*) input;
}

#endif