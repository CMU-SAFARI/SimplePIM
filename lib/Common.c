#include "Common.h"
void zero_init(uint32_t value_size, void* value_ptr){
    char* casted_value_ptr = (char*) value_ptr;
    for(int i=0; i<value_size; i++){
        casted_value_ptr[i] = 0;
    }
}

void print_int(void* value){
    int *value_casted = (int*)value;
    printf("%d\n", *value_casted);
}


void add_int(void* i1, void* i2){
    int* i1_casted = (int*)i1;
    int* i2_casted = (int*)i2;
    *i1_casted = *i1_casted + *i2_casted;
}


