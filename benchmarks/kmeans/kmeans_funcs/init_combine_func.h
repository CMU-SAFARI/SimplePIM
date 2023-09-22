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

void combine_func(void* p1, void* p2){
    int32_t* times1 = (int32_t*)p1;
    int32_t* times2 = (int32_t*)p2;
    *times1 += *times2;
    p1 += sizeof(int32_t);
    p2 += sizeof(int32_t);
    T* ptr1 = (T*)(p1);
    T* ptr2 = (T*)(p2);

    for(int i=0; i<dim; i++){
        ptr1[i] = ptr1[i]+ptr2[i];
    }
}

#endif

