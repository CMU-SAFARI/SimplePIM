#ifndef INIT_COMBINE_FUNC_H
#define INIT_COMBINE_FUNC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "../Param.h"


void init_func(uint32_t size, void* ptr){
    char* casted_value_ptr = (char*) ptr;
    for(int i=0; i<size; i++){
        casted_value_ptr[i] = 0;
    }
}

void combine_func(void* dest, void* src){
    *(uint32_t*)dest += *(uint32_t*)src; 
}

#endif
