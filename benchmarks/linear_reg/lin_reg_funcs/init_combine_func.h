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
    int64_t* ptr1 = (int64_t*)dest;
    int64_t* ptr2 = (int64_t*)src;
    for(int i=0; i<dim; i++){
        ptr1[i] += ptr2[i];
    }
}

#endif